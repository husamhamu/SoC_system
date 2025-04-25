#include "acquisition.h"
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

typedef struct {
    int32_t codePhase;
    int32_t dopplerFrequency;
    // internal state for acquisition goes here
    // for example the current index used to add code and sample entries

    // Example:
    int32_t sampleCount;
    int32_t codeCount;
    float* samples;
    float* codes;
    float* shiftedSamples;
} acquisitionInternal_t;

//#define FS 2000000
//#define N 1000
//#define GAMMA 0.015f

acquisition_t* allocateAcquisition(int32_t nrOfSamples) {
    acquisitionInternal_t *a = malloc(sizeof(acquisitionInternal_t));
    memset(a, 0, sizeof(acquisitionInternal_t)); // to initialize everything into a definitive state (=0)
    a->samples = malloc(nrOfSamples * 2 * sizeof(float)); // Example
    a->codes = malloc(nrOfSamples * 2 * sizeof(float));
    a->shiftedSamples = malloc(nrOfSamples * 2 * sizeof(float));
    return (acquisition_t*)a;
}

void deleteAcquisition(acquisition_t* acq) {
    acquisitionInternal_t *a = (acquisitionInternal_t*) acq;
    free(a->samples);
    free(a->codes);
    free(a->shiftedSamples);
    free(acq);
}

void enterSample(acquisition_t* acq, float real, float imag) {
    acquisitionInternal_t *a = (acquisitionInternal_t*) acq;
    a->samples[a->sampleCount] = real;
    a->samples[a->sampleCount+1] = imag;
    a->sampleCount += 2;
}

void enterCode(acquisition_t* acq, float real, float imag) {
    acquisitionInternal_t *a = (acquisitionInternal_t*) acq;
    a->codes[a->codeCount] = real;
    a->codes[a->codeCount+1] = imag;
    a->codeCount += 2;
}


__attribute__((noipa))
bool startAcquisition(acquisition_t* acq, int32_t testFreqCount, const int32_t* testFrequencies) {
    // Cast the generic acquisition structure to the internal type
    acquisitionInternal_t *a = (acquisitionInternal_t*)acq;
    
    // Define constants
    float FS = 2000000.0f;    // Sampling frequency in Hz
    int32_t N = 1000;         // Number of samples
    float GAMMA = 0.015f;     // Threshold for successful acquisition
    float pi = 3.14159265358979323846f;  // Pi constant

    // Check if memory for shifted samples is allocated
    if (a->shiftedSamples == NULL) {
        return false;  // Return false if memory is not allocated
    }

    // Initialize variables for best match
    float S_max = 0;  // Maximum correlation value
    int32_t bestCodePhase = 0;  // Best code phase
    int32_t bestDopplerFrequency = 0;  // Best Doppler frequency

    // Calculate the input signal power
    float P_in = 0;
    for (int32_t n = 0; n < N; n++) {
        float real = a->samples[2 * n];      // Get real part of sample
        float imag = a->samples[2 * n + 1];  // Get imaginary part of sample
        P_in += real * real + imag * imag;   // Add squared magnitude to power
    }
    P_in /= N;  // Normalize power by number of samples

    // Loop through each test frequency
    for (int32_t i = 0; i < testFreqCount; i++) {
        int32_t f_d = testFrequencies[i];  // Get current Doppler frequency to test
        
        // Debug: Print current Doppler frequency (commented out)

        // Perform frequency shift on the input samples
        for (int32_t n = 0; n < N; n++) {
            float t = (float)n / FS;  // Time of current sample
            float angle = -2 * pi * f_d * t;  // Angle for frequency shift
            float cos_val = cosf(angle);  // Cosine of angle
            float sin_val = sinf(angle);  // Sine of angle

            float real = a->samples[2 * n];      // Get real part of sample
            float imag = a->samples[2 * n + 1];  // Get imaginary part of sample

            // Apply frequency shift
            a->shiftedSamples[2 * n] = real * cos_val - imag * sin_val;
            a->shiftedSamples[2 * n + 1] = real * sin_val + imag * cos_val;
        }

        // Perform correlation in time domain
        for (int32_t tau = 0; tau < N; tau++) {
            float real_sum = 0;  // Sum of real parts
            float imag_sum = 0;  // Sum of imaginary parts

            for (int32_t n = 0; n < N; n++) {
                int32_t index = (n + tau) % N;  // Circular index for correlation

                // Get sample and code values
                float sample_real = a->shiftedSamples[2 * index];
                float sample_imag = a->shiftedSamples[2 * index + 1];
                float code_real = a->codes[2 * n];
                float code_imag = a->codes[2 * n + 1];

                // Perform complex multiplication and accumulate
                real_sum += sample_real * code_real + sample_imag * code_imag;
                imag_sum += sample_imag * code_real - sample_real * code_imag;
            }

            // Calculate magnitude of correlation
            float magnitude = real_sum * real_sum + imag_sum * imag_sum;

            // Debug: Print current correlation value (commented out)

            // Update best match if current magnitude is larger
            if (magnitude > S_max) {
                S_max = magnitude;
                bestCodePhase = tau;
                bestDopplerFrequency = f_d;
            }
        }
    }
    
    // Compute the Signal-to-Noise Ratio (SNR)
    S_max = S_max/(N*N);  // Normalize S_max
    float SNR = S_max / P_in;  // Calculate SNR

    // Debug: Print the final computed values (commented out)

    // Update the acquisition structure with the results
    if (SNR > GAMMA) {
        // If SNR exceeds threshold, acquisition is successful
        a->codePhase = bestCodePhase;
        a->dopplerFrequency = bestDopplerFrequency;
        // Debug: Print successful acquisition (commented out)

        return true; // Acquisition successful
    } else {
        // If SNR is below threshold, acquisition failed but still update values
        a->codePhase = bestCodePhase;
        a->dopplerFrequency = bestDopplerFrequency;
        // Debug: Print failed acquisition (commented out)

        return false; // Acquisition failed
    }
}
