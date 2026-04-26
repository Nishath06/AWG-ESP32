# 📌 ESP32 Pin Configuration

This document describes all GPIO connections used in the project.

---

## 🔥 Heater Control System

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

* Relay controls **12V ceramic cartridge heater**
* Heater must use **external 12V power supply**
* Do NOT power heater from ESP32

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

## ⚠️ Important Notes

* Use **external power supply** for heater
* Ensure **common ground** between ESP32 and relay module
* Avoid using GPIOs that conflict with boot mode (except defined ones)
* DS18B20 requires pull-up resistor to function properly

---

## 🚀 System Overview

* AWS sends sensor data (unchanged)
* Heater operates **locally only**
* Maintains temperature **≤ 70°C**
* WiFi reset works via hardware button

---

✔ Ready for deployment
✔ Safe GPIO usage
✔ No conflicts with AWS or sensors

---
