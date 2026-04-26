# 📌 ESP32 Pin Configuration (Updated)

This document describes all GPIO connections used in the project, including **manual heater control and safety logic updates**.

---

## 🔥 Heater Control System (Updated)

### 🌡 DS18B20 Temperature Sensor

* **DATA → GPIO 2**
* **VCC → 3.3V**
* **GND → GND**
* ⚠️ Use a **4.7kΩ resistor** between DATA and VCC

---

### 🔌 Relay Module (12V Heater Control)

* **IN → GPIO 16**
* **VCC → 5V (or 3.3V depending on module)**
* **GND → GND**

⚠️ Notes:

* Controls **12V ceramic cartridge heater**
* Uses **external 12V power supply**
* ESP32 only sends control signal

---

### 🔘 Manual Heater Switch (NEW)

* **Pin → GPIO 17**
* **Other side → GND**
* Mode: `INPUT_PULLUP`

### Function:

* Switch OFF → Heater **always OFF**
* Switch ON → Heater control logic activated

---

## 🌡 Environment Sensors

### 🌡 DHT11 (Temperature & Humidity)

* **DATA → GPIO 15**
* **VCC → 3.3V**
* **GND → GND**

---

### 💧 Water Level Sensor

* **Analog OUT → GPIO 34**

---

### 🧪 TDS Sensor

* **Analog OUT → GPIO 35**

---

## 🎨 RGB Color Sensor (TCS3200)

| Pin | ESP32 GPIO |
| --- | ---------- |
| S0  | GPIO 14    |
| S1  | GPIO 27    |
| S2  | GPIO 26    |
| S3  | GPIO 25    |
| OUT | GPIO 33    |

---

## 🔘 Reset Button (WiFi Reset)

* **Pin → GPIO 4**
* **Other side → GND**
* Mode: `INPUT_PULLUP`

### Function:

* Press and hold **> 2 seconds**
* Clears WiFi credentials
* Restarts ESP32

---

## 📡 Summary Table

| Component          | GPIO |
| ------------------ | ---- |
| DS18B20            | 2    |
| Relay              | 16   |
| Manual Switch      | 17   |
| Reset Button       | 4    |
| DHT11              | 15   |
| Water Level Sensor | 34   |
| TDS Sensor         | 35   |
| TCS3200 S0         | 14   |
| TCS3200 S1         | 27   |
| TCS3200 S2         | 26   |
| TCS3200 S3         | 25   |
| TCS3200 OUT        | 33   |

---

## ⚙️ Heater Control Logic (Updated)

The heater system now follows **hybrid control (manual + automatic)**:

### 🔘 Manual Control

* Heater will **NOT start automatically**
* User must enable using switch

---

### 🌡 Temperature Control

* **< 68°C → Heater ON**
* **≥ 70°C → Heater OFF**

---

### 🎨 Color-Based Control (NEW)

* Detects **orange gradient (not exact color)**
* When silica reaches drying stage → Heater OFF

---

### ⏱ Safety Timeout (NEW)

* Maximum continuous run time: **90 minutes**
* After timeout → Heater OFF (even if switch ON)

---

### 🔒 Fail-Safe Conditions

Heater turns OFF if:

* Sensor error
* Switch OFF
* Timeout reached
* Orange gradient detected

---

## ⚠️ Important Notes

* Use **external power supply** for heater
* Maintain **common ground**
* DS18B20 requires pull-up resistor
* Manual switch gives user full control over power usage

---

# 📦 Required Libraries (ESP32 Project)

## 🔧 Core Libraries

* WiFi (built-in)
* WiFiClientSecure (built-in)
* PubSubClient → MQTT
* WiFiManager → WiFi portal

---

## 🌡 Sensor Libraries

* DHT sensor library (Adafruit)
* Adafruit Unified Sensor
* OneWire
* DallasTemperature

---

## 🎨 Others

* TCS3200 → No library required
* Analog sensors → built-in

---

## 📋 Summary

| Library           | Purpose               |
| ----------------- | --------------------- |
| WiFi              | Network               |
| WiFiClientSecure  | AWS secure connection |
| PubSubClient      | MQTT                  |
| WiFiManager       | Setup portal          |
| DHT               | Temp/Humidity         |
| OneWire           | DS18B20               |
| DallasTemperature | DS18B20               |

---

## 🚀 System Overview

* AWS → Receives sensor data (unchanged)
* Heater → Fully local control
* User → Decides when to activate
* System → Ensures safety and efficiency

---

✔ Manual + Smart control
✔ Safe heater operation
✔ Cloud-independent heating logic
✔ Ready for real-world deployment

---
