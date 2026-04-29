# 📌 ESP32 Pin Configuration (Final)

This document defines all GPIO connections used in the system, including heater control, sensors, OLED display, and safety logic.

---

## 🔥 Heater Control System

### 🌡 DS18B20 Temperature Sensor

* **DATA → GPIO 2**
* **VCC → 3.3V**
* **GND → GND**
* ⚠️ **4.7kΩ pull-up resistor required** (DATA ↔ VCC)

---

### 🔌 Relay Module (Heater Control)

* **IN → GPIO 16**
* **VCC → 5V** (recommended)
* **GND → GND**

⚠️ **Notes:**
* Controls **12V ceramic heater**
* Uses **external 12V supply**
* ESP32 provides only control signal

---

### 🔘 Manual Heater Switch

* **GPIO 17** → Switch → **GND**
* Mode: `INPUT_PULLUP`

**Function:**
* **LOW (Pressed)** → Heater Enabled
* **HIGH** → Heater Disabled (Forced OFF)

---

## 🌡 Environmental Sensors

### 🌡 DHT11 (Temperature & Humidity)

* **DATA → GPIO 15**
* **VCC → 3.3V**
* **GND → GND**

---

### 💧 Water Level Sensor (Analog)

* **AO → GPIO 35**

---

### 🧪 TDS Sensor (Analog)

* **AO → GPIO 34**

---

## 🎨 RGB Color Sensor (TCS3200)

| Pin | ESP32 GPIO |
| --- | ---------- |
| S0  | 14         |
| S1  | 27         |
| S2  | 26         |
| S3  | 25         |
| OUT | 33         |

⚠️ **Note:**
* Works on 3.3V logic
* Frequency output read using `pulseIn()`

---

## 🖥 OLED Display (SSD1306 – I2C)

* **SDA → GPIO 21**
* **SCL → GPIO 22**
* **VCC → 3.3V**
* **GND → GND**

---

## 🔘 Reset Button (WiFi Reset)

* **GPIO 4** → Button → **GND**
* Mode: `INPUT_PULLUP`

**Function:**
* Press **> 2 seconds**
* Clears WiFi credentials
* Restarts ESP32

---

## 📡 Summary Table

| Component | GPIO |
| :--- | :--- |
| DS18B20 | 2 |
| Relay | 16 |
| Manual Switch | 17 |
| Reset Button | 4 |
| DHT11 | 15 |
| Water Level | 35 |
| TDS Sensor | 34 |
| TCS3200 S0 | 14 |
| TCS3200 S1 | 27 |
| TCS3200 S2 | 26 |
| TCS3200 S3 | 25 |
| TCS3200 OUT | 33 |
| OLED SDA | 21 |
| OLED SCL | 22 |

---

## ⚙️ Heater Control Logic (Final)

### 🔘 Manual Control
* Heater works only if switch is ON
* If OFF → Heater is always OFF

---

### 🌡 Temperature Control
* **Temp < 68°C** → Heater ON
* **Temp ≥ 70°C** → Heater OFF

---

### 🎨 Color-Based Safety (Silica Detection)
* Detects orange gradient (not exact RGB)
* Indicates silica drying completion
* 👉 Heater is turned OFF

---

### ⏱ Safety Timeout
* Max runtime: **90 minutes**
* After timeout → Heater OFF

---

### 🔒 Fail-Safe Conditions
Heater turns OFF if:
❌ Sensor failure
❌ Manual switch OFF
❌ Timeout reached
❌ Orange color detected

---

## 📺 OLED Display Logic

* Boot screen → "Public AWG"
* Displays sequentially:
  * Temperature
  * Humidity
  * Water Level
  * TDS
  * Water Safety

---

## 📦 Required Libraries

### 🔧 Core
* WiFi
* WiFiClientSecure
* PubSubClient
* WiFiManager

### 🌡 Sensors
* DHT
* Adafruit Unified Sensor
* OneWire
* DallasTemperature

### 📺 Display
* Adafruit_GFX
* Adafruit_SSD1306

### 🎨 Others
* TCS3200 → No library required
* Analog sensors → Built-in

---

## 🚀 System Overview

* **ESP32 reads:**
  * Temperature
  * Humidity
  * Water level
  * TDS
  * Color
* **Controls:**
  * Heater (locally)
* **Sends:**
  * Data to AWS IoT (MQTT)
* **Displays:**
  * Live data on OLED

---

## ⚠️ Important Notes (Refined)
* Use external 12V power for heater
* Ensure common ground across all modules
* DS18B20 requires pull-up resistor
* TDS sensor must not exceed 3.3V
* Avoid powering relay directly from ESP32 if unstable

