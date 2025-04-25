#include "peripherals.h"
#include <i2c_treiber.h>
#include <stdio.h>
#include <sleep.h>
#include <moduleParameters.h>
#include <spi_treiber.h>
#include "display.h"
#include <interrupt.h>

DEFINE_SLEEP_US(sleep_us, SB_SPARTANMC_FREQUENCY)
DEFINE_SLEEP_MS(sleep_ms, SB_SPARTANMC_FREQUENCY)
FILE *stdout = &UART_LIGHT_FILE;

#define SRF02_ADDR 0xE0 // SRF02 sensor I2C address
#define SRF02_CMD_REG 0x00 // SRF02 command register
#define SRF02_RANGE_REG 0x02 // SRF02 range register
unsigned char data[2];

#define WINDOW_SIZE 5
int sensorReadings[WINDOW_SIZE]; // Array to store sensor readings
int readIndex = 0; // Index to keep track of the current position in the array
int filteredReading = 0; // Filtered sensor reading

// State machine states
typedef enum {
    ST_WRITE_START,
    ST_READ_START,
    ST_READ_REPEATED,
    TIMER_INTERRUPT,
    ST_READ_DATA
} i2c_state_t;

i2c_state_t i2c_state;

// Function to swap two elements
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Partition function for quicksort
int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}

// Quicksort function
void quicksort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

// Function to add a new sensor reading and calculate the filtered output
void addReading(int reading) {
    sensorReadings[readIndex] = reading; // Replace the oldest reading with the new one
    readIndex = (readIndex + 1) % WINDOW_SIZE; // Move the index to the next position

    // Create a temporary array to store the current window of readings
    int temp[WINDOW_SIZE];
    for (int i = 0; i < WINDOW_SIZE; i++) {
        temp[i] = sensorReadings[i];
    }

    // Sort the temporary array using quicksort
    quicksort(temp, 0, WINDOW_SIZE - 1);

    // Get the median value from the sorted array
    filteredReading = temp[WINDOW_SIZE / 2];
}


void display_distance(int distance){
	char buffer[30];
    // Write the distance in the buffer
	snprintf(buffer, sizeof(buffer), "Distance: %i    ", distance);
    // Show the distance on the display
	Show_String_25664(1, buffer, 0, 0);
}

void display_filteredReading(int filteredReading){
	char buffer[30];
	snprintf(buffer, sizeof(buffer), "Filtered distance: %i    ", filteredReading);
	Show_String_25664(1, buffer, 0, 10);
}

void interrupt_start(){
    I2C_MASTER.data[0] = SRF02_ADDR;
    // Send the register address
    I2C_MASTER.data[1] = SRF02_CMD_REG;
    // Send the data
    unsigned char cmd = 0x51; // Start ranging, result in centimeters
    I2C_MASTER.data[2] = cmd; // Write the command
    I2C_MASTER.cmd = I2C_CMD_STA | I2C_CMD_STO |(3 << 3); // Start bit, end bit, length of data
    i2c_state = TIMER_INTERRUPT; // Update the state machine to start the timer afterwards
}


void state_machine(){
    switch (i2c_state) {
        case TIMER_INTERRUPT:
            // Start the timer to wait 70ms for sensor to deliver Messdaten
			TIMER.control |= TIMER_EN;
            // Update the state machine to start read data afterwards
			i2c_state = ST_READ_START;
			break;
			
        case ST_READ_START:
            // Send the slave address with write bit
            I2C_MASTER.data[0] = SRF02_ADDR;
            // Send the register address
            I2C_MASTER.data[1] = SRF02_RANGE_REG;
            I2C_MASTER.cmd = I2C_CMD_STA | (2 << 3); // Start bit, length of data
            // Update the state machine to send repeated start afterwards 
            i2c_state = ST_READ_REPEATED;
            
            break;

        case ST_READ_REPEATED:
            // Send repeated start condition and slave address with read bit
            I2C_MASTER.data[0] = SRF02_ADDR | 1;
            I2C_MASTER.cmd = I2C_CMD_STA | I2C_CMD_RD | I2C_CMD_STO |((2 +1)<< 3);// Start bit, read transfer, stop bit, length of data
            // Update the state machine to read the data afterwards
            i2c_state = ST_READ_DATA;
            break;

        case ST_READ_DATA:
            // Read the data
            for (int i = 0; i < 2; i++) {
                data[i] = I2C_MASTER.data[i + 1];
            }
            // NOt necessary but we can Check of valid data and start a read transfer agian if not data are not valid
            if((data[0] == 6 && data[1] == 0) || (data[0] == 2 && data[1] == 81)){
				I2C_MASTER.data[0] = SRF02_ADDR | 1;
				I2C_MASTER.cmd = I2C_CMD_STA | I2C_CMD_RD | I2C_CMD_STO |((2 +1)<< 3);
				break;
			}
            // Filter the reading using the median filter
            int reading = (data[0] | data[1]);
            addReading(reading);
            // Display the filtered data on the display
            display_filteredReading(filteredReading);
            // Start the Abstandmesser all over again
            interrupt_start();
            break;

        default:
            break;
    }
}
// ISR for handling I2C transfer states
ISR(0)() {
	state_machine();
}

// Setting up the timer parameters with compare unit
void setup_timer(){
    // Limit value to which the timer up-counts
	TIMER.limit = 20000;
    // Starting value of timer
	TIMER.value = 0;
    // Enable prescaler and set prescaler value
	TIMER.control = TIMER_PRE_EN | TIMER_PRE_256;
    // Compare unit value 
	COMPARE.CMP_DAT = 20000;	
    // Enable interrupt for the compare unit
	COMPARE.CMP_CTRL = COMPARE_EN | COMPARE_EN_INT;	
	
}

// Disable the timer
void disable_timer(){
    // Reset the interrupt falg
	COMPARE.CMP_CTRL = COMPARE_EN | COMPARE_EN_INT;
    // Disable the timer
	TIMER.control &= ~TIMER_EN;
}

// ISR for handling timer interrupts 
ISR(1)() {
    // First disable the timer
	disable_timer();
    // Second call the state machine 
	state_machine();
	
}

void abstandmesser_with_interrupt(){
	
	// Enable the I2C peripheral with a prescaler value of 134
	i2c_peri_enable();
	// Enable the SPI master
    spi_peri_enable();
	// Select slave 1
    spi_peri_select();
	// Initialize the display
    OLED_Init_25664();
	// Enable interrupts
	interrupt_enable();
	// Configure timer
	setup_timer();
	// Trigger the start of interrupt
	interrupt_start();
	while(1){
	}
}

int main() {
    // Abstandmesser with interrupts implementation
	abstandmesser_with_interrupt();
	while(1){
		
	}
    return 0;
}
