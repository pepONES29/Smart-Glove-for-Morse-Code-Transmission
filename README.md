# Smart-Glove-for-Morse-Code-Transmission
An assistive wearable (AAC) that translates finger flexion depth into Morse code text. Utilises an ESP32 for wireless BLE transmission to mobile devices and provides real-time OLED feedback.
Unlike traditional Morse code systems that rely on precise pulse timing, this "Smart Glove" uses amplitude-based classification, allowing users to send "dots" and "dashes" simply by controlling the depth of a finger bend.


🧞‍♂️ **Key Innovations**

- **Depth-Based Input:** Accessible for individuals with motor impairments; a "light bend" triggers a dot, while a "deep bend" triggers a dash.

- **Word Buffering:** Decoded characters are stored locally and transmitted to the mobile device only after a word is completed (3.5s inactivity), improving readability.

- **GATT Architecture:** The ESP32 acts as a GATT Server hosting the Nordic UART Service (NUS), pushing data asynchronously to a smartphone.


🛠️ **Hardware Requirements**

The system is built on a portable, autonomous architecture providing roughly 60+ hours of continuous operation.

- Central processor: **ESP32 (GroundStudio Carbon V3)** - *Dual-core, 4MB Flash, 12-bit ADC*
- Gesture input: **5.5 cm Flex Sensor** - *Resistance: 10kOhms (flat) to 30kOhms+ (bent)*
- Local Feedback: **SSD1306 OLED Display** - *128x64 resolution, I2C protocol*
- Power Supply: **Hama Powerbank** - *10000 mAh capacity*

**Signal Conditioning:**
The flex sensor acts as a variable resistor in a voltage divider circuit. The output voltage Vout read by the ESP32's ADC is calculated as:

$$V_{out} = V_{in} \times \frac{R_{static}}{R_{static} + R_{flex}}$$


💻 **Software Logic**

The firmware is designed as a Finite State Machine (FSM) using non-blocking timing (millis()) to ensure the BLE connection remains stable.

- Sampling: ADC samples at ~60Hz to identify the peak of a bend.

- Noise Filtering: An 80ms minimum duration threshold (debouncing) discards accidental movements.

- Classification: 

  - Dot (.): ADC value between 650 and 950.

  - Dash (-): ADC value below 650.

- Timeouts: 

  - 1.5s: Triggers character translation.

  - 3.5s: Triggers word transmission via BLE.


🔬 **Results and Experiments**

Empirical testing mapped raw ADC values to physical states with a 95% success rate.

- Relaxed (Flat): ~1010 ADC units.

- Morse "Dot": ~780 ADC units.

- Morse "Dash": ~520 ADC units.

- Connectivity: BLE range remains stable up to 8 meters in indoor environments
