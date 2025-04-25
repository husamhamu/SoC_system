## 📡 Abstandsmesssystem auf FPGA mit SpartanMC

Dieses Projekt beschreibt die vollständige Umsetzung eines System-on-Chip (SoC) auf einem FPGA-Board unter Verwendung des SpartanMC Soft-Core-Prozessors. Ziel war die Entwicklung eines Systems zur Echtzeit-Abstandsmessung mit einem SRF02-Ultraschallsensor und einem OLED-Display. Die Firmware wurde vollständig in C entwickelt, inklusive selbst programmierter SPI- und I2C-Treiber sowie interruptgesteuerter Optimierungen.

---

### 📚 Systemarchitektur

- **Physikalische Schicht (PHY):** FPGA, Sensor, Display
- **HDL-Schicht:** SpartanMC, SPI Master, I2C Master, UART Light
- **Firmware-Schicht:** In C geschriebene Treiber, Filter und ISR-Logik


![image](https://github.com/user-attachments/assets/fcaa8caa-9821-43a8-abdb-62230db1d9c0)

---

### 🚀 Projektüberblick

- 🔧 Entwicklung in **C** für einen benutzerdefinierten **18-Bit RISC Soft-Core (SpartanMC)**
- 🔌 Implementierung eigener **SPI- und I2C-Treiber**
- 🖥️ Ansteuerung eines **256x64 OLED-Displays über SPI**
- 📏 Echtzeit-Abstandsmessung mit **SRF02-Ultraschallsensor über I2C**
- 🧼 Rauschunterdrückung durch **Medianfilter mit Ringpuffer**
- ⚡ **Interruptgesteuerte Firmware** mit Zustandsautomaten
- 🖨️ Ausgabe über **UART-Konsole und OLED-Anzeige**
- 🧪 System läuft auf einem **Xilinx SP601 FPGA-Board**
![image](https://github.com/user-attachments/assets/2a0d87b1-523f-40d1-a250-631704dec4bf)

---

### 📁 Projektstruktur

```
├── drivers/                   # Eigene SPI- und I2C-Treiber
│   ├── spi_peri.c
│   └── i2c_peri.c
├── display/                  # Displayfunktionen (Kommando- & Datentransfer)
│   ├── oled_command.c
│   └── oled_data.c
├── filter/                   # Medianfilter mit Ringpuffer
│   └── median_filter.c
├── firmware/                 # Hauptlogik und Zustandsautomat
│   ├── main.c
│   ├── isr.c
│   └── state_machine.c
├── docs/                     # Dokumentation und Abgaben
│   └── Bericht_Versuch_1.pdf
├── Makefile
└── README.md
```

---

### 🧩 Wichtige Komponenten

#### ✅ SPI-Treiber (OLED-Ansteuerung)

Implementierung eines SPI-Treibers mit konfigurierbarer Rahmenbreite und Slaveselection.

> 🔍 Siehe Quelltext `Bericht_Versuch_1.pdf`, Quelltext 2.1, Seite 4–5

#### ✅ I2C-Treiber (Sensor-Ansteuerung)

Realisierung eines Lese-/Schreibprotokolls mit Repeated-Start-Bedingung gemäß I2C-Standard.

> 🔍 Siehe Quelltext `Bericht_Versuch_1.pdf`, Quelltext 2.1, Seite 6

#### 📉 Medianfilter

Filterung von Ausreißern über einen 5-Werte-Ringpuffer und Quicksort-Sortierung zur Medianbestimmung.

> 🔍 Siehe Quelltext `Bericht_Versuch_1.pdf`, Quelltext 2.2, Seite 8

#### ⏱️ Interrupts & Zustandsautomat

Ereignisgesteuertes Design über ISR und ein State-Machine-Konzept. Dadurch entfällt aktives Warten, was die CPU-Auslastung reduziert.

> 🔍 Siehe `Bericht_Versuch_1.pdf`, Quelltext 2.2, Seite 11  
> 📊 Vergleich Polling vs. Interrupt: Tabelle 1, Seite 9

---

### 📷 Systemabbildung

*(Optional: Hier kann ein Blockdiagramm oder Foto der aufgebauten Hardware eingefügt werden.)*

---

### 📈 Ergebnisse

- 🔁 Stabile und präzise Messwerte durch Filterung
- 💻 Gleichzeitige Anzeige auf UART-Konsole und OLED
- 🧠 Effiziente CPU-Ausnutzung durch Interrupt-gesteuertes Design

---

### 💬 Learnings

- Entwicklung eingebetteter Treiber in ressourcenlimitierten Umgebungen
- Tiefes Verständnis für SPI-, I2C- und UART-Kommunikation
- Systemoptimierung durch Zustandsautomaten und Interrupts
- Praxiserfahrung mit FPGA-basierten Softcore-Architekturen
- Anwendung von Signalverarbeitung (Medianfilter) auf Mikrocontroller-Ebene

---

### 📄 Ausführliche Dokumentation

📘 [Bericht_Versuch_1.pdf](./docs/Bericht_Versuch_1.pdf)

---

Wenn du möchtest, kann ich dir die Datei direkt als `README.md` im richtigen Format exportieren oder zusätzlich ein passendes Repository-Skelett aufbauen. Sag einfach Bescheid!
