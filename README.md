# Warema Blinds MQTT Bridge (ESP32)

> **Note:** This project is a fork of the [warema_mqtt_bridge](https://github.com/tombrain/warema_mqtt_bridge) repository by [tombrain](https://github.com/tombrain). **A big thank you to tombrain** for his initial work that made this project possible.

This fork enables control of Warema blinds via 433&nbsp;MHz signals using an ESP32. Commands are received over MQTT and relayed through a 433&nbsp;MHz transmitter to the blinds. The code in this repository consists of two main files:
- **warema_mqtt_bridge.ino** – The main Arduino sketch handling WiFi, MQTT, and command processing.
- **RCSwitchWarema.h** – A class extending the [RCSwitch](https://github.com/sui77/rc-switch) library to handle Warema-specific signal patterns.

---

## Features

1. **MQTT-controlled** – Subscribes to a configurable MQTT topic to receive commands.
2. **ESP32-based** – Uses WiFi connectivity and an on-board MCU to handle real-time reception and transmission.
3. **433&nbsp;MHz Transmission** – Sends the proper signals to your Warema blinds using an inexpensive 433&nbsp;MHz transmitter.
4. **Status LEDs** – Two GPIO pins drive LEDs indicating WiFi and MQTT connection status.
5. **JSON-based Status** – Publishes device info (IP, MAC, RSSI, etc.) in JSON format to a status topic.

---

## Table of Contents

- [Hardware Setup](#hardware-setup)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Topics and Command Payloads](#topics-and-command-payloads)
- [Troubleshooting](#troubleshooting)
- [Change Log](#change-log)
- [Roadmap](#roadmap)
---

## Hardware Setup

1. **ESP32 Board**  
   Any standard ESP32 development board (e.g., ESP32-DevKitC, NodeMCU-32S) can be used.

2. **433&nbsp;MHz Transmitter**  
   - Connect the **Data** pin of the transmitter to GPIO&nbsp;27 on the ESP32 (default in code).
   - Power it with 5&nbsp;V or 3.3&nbsp;V (depending on transmitter specs).
   - Make sure to connect ground to the ESP32 ground.

3. **Status LEDs**  
   - **Red LED**: Connect to GPIO&nbsp;5 (with a suitable resistor). Lights up when connected to MQTT.  
   - **Yellow LED**: Connect to GPIO&nbsp;14 (with a suitable resistor). Lights up when **not** connected to MQTT/WiFi.  
   - Adjust the pin assignments in code if desired.

**Default Pin Mapping**

| Component         | ESP32 Pin |
|-------------------|-----------|
| **Transmitter**   | 27        |
| **Red LED**       | 5         |
| **Yellow LED**    | 14        |

---

## Prerequisites

1. **Arduino IDE** (or PlatformIO)  
   - [Arduino IDE Download](https://www.arduino.cc/en/software)

2. **ESP32 Board Support**  
   - In the Arduino IDE, open **Tools** > **Board** > **Boards Manager**,  
     search for `ESP32 by Espressif Systems`, and install it.

3. **Required Libraries**  
   Install these libraries via the Arduino Library Manager or manually:
   - [PubSubClient](https://github.com/knolleary/pubsubclient)
   - [ArduinoJson](https://arduinojson.org/)
   - [RCSwitch](https://github.com/sui77/rc-switch) (required even though `RCSwitchWarema.h` extends it)

4. **Credentials File (`secure.h`)**  
   You’ll need to create a `secure.h` file in the same directory as `warema_mqtt_bridge.ino`, containing:
   ```cpp
   extern const char* ssid;
   extern const char* password;
   extern const char* mqtt_username;
   extern const char* mqtt_password;
  Replace the placeholders with your actual WiFi and MQTT credentials. This file is not included in the repository for security reasons.

---

## Installation

1. **Clone or Download** this repository into your Arduino sketches folder  
   *(e.g., `Documents/Arduino/warema_mqtt_bridge`).*

2. **Add Libraries**  
   Ensure that the required libraries (PubSubClient, ArduinoJson, and RCSwitch) are installed. You can install them via the Arduino Library Manager or manually.

3. **Open the Project**  
   Open `warema_mqtt_bridge.ino` in the Arduino IDE (or use PlatformIO if that is your preference).

4. **Select Your Board**  
   In the Arduino IDE, go to **Tools** > **Board** and select your ESP32 board (e.g., `ESP32 Dev Module`).

5. **Configure Serial Port**  
   Under **Tools** > **Port**, select the COM port or device name that corresponds to your ESP32.

6. **Compile and Upload**  
   Click the **Upload** button. The code will compile and then be flashed onto your ESP32.

---

## Configuration
- **WiFi Credentials**
  In `secure.h`, fill in your `ssid` and `password`.
- **MQTT Broker Address & Port**
  In `warema_mqtt_bridge.ino` modify:
    ```cpp
    const char* mqtt_broker = "homeassistant.local";
    const int   mqtt_port   = 1883;
  ```
  If you do not have mDNS set up, replace `"homeassistant.local"` with the IP address of your MQTT broker (e.g., `"192.168.1.10"`).
- **MQTT Username/Password**
  Also in `secure.h`, ensure you have:
  ```cpp
  extern const char* mqtt_username = "yourUsername";
  extern const char* mqtt_password = "yourPassword";
- **GPIO Pins**
  If you need different pins, adjust these lines:
  ```cpp
  #define LED_WIFI_RED      5
  #define LED_WIFI_YELLOW   14
  #define TRANSMITTER_PIN   27
  ```
  in `warema_mqtt_bridge.ino` to match your wiring setup.

---

## Usage

1. **Power On the ESP32**
 - The device will attempt to connect to WiFi.
 - Once connected, it tries to connect to the MQTT broker.

2. **LED Indicators**
 - Red LED on: The device is connected to MQTT.
 - Yellow LED on: The device is not connected to WiFi or MQTT.

3. **MQTT Topics**
- **Command Topic** (subscribe):
    ```bash
    cmnd/rf-warema-bridge/data
    ```
    Send any control messages for the blinds to this topic.

- **Connection Info Topic** (publish):
   ```bash
   stat/rf-warema-bridge/connection
   ```
   The device publishes its IP, MAC, RSSI, and SSID here in JSON format.

4. **Sending Blind Commands**
- Publish a message with the appropriate pulse pattern (e.g. `sS0110...`) to `cmnd/rf-warema-bridge/data`.
- The code calls `mySwitch.sendMC(...)` with default timings:
  ```ccp
  mySwitch.sendMC(command, 1780, 5000, 3, 10000);
  ```
  - `command` – String of `s`, `S`, `0`, `1`
  - `1780` – Data bit length (in microseconds)
  - `5000` – Sync pulse length (in microseconds)
  - `3` – Repeat count
  - `10000` – Delay between repeats (in microseconds)
  You may need to adjust the pulse pattern or timing for your specific Warema blinds or device model.


---

## Topics and Command Payloads
###Example Command Payload###
A possible payload to lower the blinds could be:
```nginx
sS00100110
```
___(The actual sequence depends on your specific blind remote signal. You must capture or deduce the correct sequence.)___

- `s` / `S` might indicate a sync pulse (low or high).
- `0` / `1` typically indicate the data bits to send.
When the payload is published to cmnd/rf-warema-bridge/data, the ESP32 will send the pulses via GPIO 27 to your 433 MHz transmitter, which should in turn operate your blinds.

---

## Troubleshooting
1. **No MQTT Connection**
- Check serial monitor for error messages.
- Make sure you have the correct broker address, port, username, and password.

2. **No 433 MHz Transmission**
- Ensure your transmitter is powered correctly (3.3V or 5V, depending on your module).
- Double-check the TRANSMITTER_PIN (GPIO number).
- Verify your code pattern is correct for your blinds.

3. Incorrect Pulse Timings
- If blinds do not respond, adjust the dataLength, syncLength, sendCommand (repeats), or sendDelay in mySwitch.sendMC(...).

4. LEDs Not Working
- Check pin definitions and wiring of resistors and LEDs.

---

## Change Log
### [0.0.1] - 2025-02-26
___Note: this is an untested version___
- Converted code to be compatible with ESP32
- Improved code to prevent memory leaks
- Minor improvements

---

## Roadmap
- Add [OTA](https://docs.arduino.cc/arduino-cloud/features/ota-getting-started/) support with NTP time based updates
- Bugfixes
- Performance

---

**Happy Automating**! If you have issues, please open an issue on GitHub or submit a pull request with your improvements. Enjoy controlling your Warema blinds over MQTT!
