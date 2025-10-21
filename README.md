# 🎓 PhD Hat Project 
An interactive, sensor-driven celebration hat designed for the PhD defense of our friend Patrick Langer!

For a truly entertaining graduation moment, the hat reacts to **movement**, **alcohol detection**, — creating a fun and responsive experience, a sequence of light patterns, OLED messages, and audio playback.

---

## ✨ Overview

This Arduino-based project transforms an ordinary hat into a **multi-sensory experience** with:
- 🎶 DFPlayer Mini MP3 playback  
- 💡 WS2812 (NeoPixel) LED effects  
- 🖥️ OLED Display (256x64, SSD1322)  
- ⚙️ Motion sensing via MPU6500 IMU  
- 🍷 Alcohol detection via MQ3 gas sensor  

The system runs through several **modes** (Idle, Celebration, Drunk, Fancy), automatically progressing based on sensor input.

---

## 🧩 Hardware Components

| Component | Model | Description |
|------------|--------|-------------|
| Microcontroller | Arduino Nano | Core control unit |
| IMU Sensor | MPU6500 (6-axis) | Detects motion and shaking |
| Alcohol Sensor | MQ-3 | Measures ethanol vapors (for “drunk” detection) |
| Audio Module | DFPlayer Mini | Plays MP3 tracks from SD card |
| Speaker | Loud speaker | Connects to DFPlayer output |
| Display | 256x64 OLED (SSD1322) | Displays text prompts and feedback |
| LED Ring | WS2812 12-pixel (NeoPixel) | Provides visual lighting effects |
| Power | 5V regulated | Shared by all modules |

---

## [📄 Wiring Connection (PDF)](Wiring.pdf)

| Component | Pin | Arduino Pin |
|------------|-----|--------------|
| **DFPlayer Mini** | RX → | D4 |
| | TX → | D5 |
| | BUSY → | D7 |
| | VCC → | +5V |
| | GND → | GND |
| **NeoPixel Ring** | DIN → | D6 |
| **OLED Display (SSD1322)** | CS → | D10 |
| | DC → | D9 |
| | RESET → | D8 |
| | CLK → | D13 (hardware SPI) |
| | MOSI → | D11 (hardware SPI) |
| | VCC → | +5V |
| | GND → | GND |
| **IMU MPU6500** | SDA → | A4 |
| | SCL → | A5 |
| | VCC → | +3.3V |
| | GND → | GND |
| **MQ-3 Sensor** | AOUT → | A0 |
| | VCC → | +5V |
| | GND → | GND |

---

## 🎭 Software Modes

### 💤 Idle Mode
- OLED: “Welcome Dr. Patrick Langer”
- LEDs: Soft white breathing
- Audio: Plays tracks **1–8** in sequence

### 💃 Celebration Mode
- OLED: “Human Activity Detection”
- Plays **audio 9–10**, then waits for shaking detected by MPU6500, LED blue pulse during waiting
- Play **audio 11** and check if shaking is detected ( Δ acceleration > threshold):
  - if yes, OLED → “Body activity detected” + play **audio 12+13+14** + LED turns green
  - if not after time out: OLED → “Body activity not detected" + play **audio 12+13+15** + LED turns red

### 🍷 Drunk Mode
- OLED: “Alcohol detection”
- Plays **audio 16–17**, LED rolling orange/yellow
- Play **audio 18** and check if MQ3 senses ethanol (avgValue > calibrated threshold):
  - if yes, OLED → “Alcohol detected” + play **audio 20+21+23** + LED red when detected
  - if not after time out: OLED → “Alcohol not detected” + play **audio 20+22+23**

### 🎉 Fancy Mode
- OLED: “Time to Party!”
- LED: Full RGB animation (rainbow/flicker)
- Audio: Plays **tracks 24–26**

## ⚙️ Troubleshooting & Practical Notes

### 🧩 When the Arduino Sketch Fails to Upload

If sketch doesn’t upload to the Arduino Nano (or similar board), try:

1. **Select the Correct Processor**  
   - **Tools → Processor → ATmega328P**  
     Use the default one, for the Nano clones require (Old Bootloader).

2. **Explicitly Select Board and Port**  
   - **Tools → Board → Arduino Nano**  
   - **Tools → Port →** correct COM/USB port.

3. **Reset the Board Manually (if stuck)**  
   - Disconnect any external **power supply**.  
   - Briefly **short VCC and GND** to reset.  

4. **Disconnect RX/TX During Upload**  
   - Serial may block communication with the IDE. Unplug RX/TX wires before uploading, reconnect afterward.

---

### 🎵 DFPlayer Mini Usage Notes
1. **SD Card Compatibility**  
   - Works only with **≤ 32GB microSD** cards.  
   - Brand matters. **SanDisk** works well. **Intenso** fails to initialize.

2. **File System & Copying**
   - Avoid **macOS** (creates hidden `.DS_Store` files) to copy files. → Use **Windows** or **Linux** 
   - SD card must have **at least one MP3 file**.  
   - Name MP3 / WAV files exactly as: `0001.mp3`, `0002.mp3`, `0003.mp3`, … up to `0025.mp3`, and stored in the DFPlayer’s SD card root directory.
   - If you needs to update or rename the audios, need to **reformat the SD card**. As the DFPlayer plays files in the **order they were written**, not by name. After renaming, need to copy files in exact order, **ONE by ONE**.  

4. **Wiring Reminder**  
   - Software Serial could work, but need to take care **RX (Arduino)** → **TX (DFPlayer)**, **TX (Arduino)** → **RX (DFPlayer)**  
     
5. **Volume and Power**  
   - High volume (>20) can distort or stop playback.  

---

### 🎙️ Helpful Online Tools for Creating Audio

| Purpose | Tool | Link |
|----------|------|------|
| Robot Voice Generator | LingoJam | [https://lingojam.com/RobotVoiceGenerator](https://lingojam.com/RobotVoiceGenerator) |
| R2D2 Soundboard | 101 Soundboards | [https://www.101soundboards.com/boards/10634-r2-d2-sounds-star-wars](https://www.101soundboards.com/boards/10634-r2-d2-sounds-star-wars) |
| YouTube Downloader | Y2Mate | [https://www-y2mate.com/convert/](https://www-y2mate.com/convert/) |
| MP3 Cutter / Editor | MP3Cut Online | [https://mp3cut.net/de/](https://mp3cut.net/de/) |

