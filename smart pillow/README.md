# Smart Pillow - Silent Wake-Up Alarm

## Overview

Smart Pillow is an IoT-based silent alarm system designed to wake users without disturbing others. Unlike traditional alarms that use sound, this project uses a vibration motor embedded inside a pillow to provide a gentle and comfortable wake-up experience.

The system is especially useful for students, hostel residents, and people living in shared environments where loud alarms can disturb roommates or family members.

## Problem Statement

Traditional alarm systems create noise and can disturb people nearby. Wearable vibration devices are often uncomfortable, while commercial smart pillows are expensive. This project provides a simple, affordable, and silent alternative.

## Features

* Silent wake-up using vibration technology
* ESP32-based control system
* Telegram Bot integration for alarm management
* Wi-Fi communication support
* Snooze and Stop alarm options
* Portable and low-power design
* User-friendly interface
* Cost-effective implementation

## Technologies Used

### Hardware

* ESP32 Microcontroller
* Vibration Motor
* Li-ion/Li-Po Battery
* TP4056 Charging Module
* BC547/2N2222 Transistor
* 1N4007 Diode
* Connecting Wires and Switches

### Software

* Arduino IDE
* Telegram Application
* Telegram Bot API
* Embedded C/C++
* Wi-Fi Libraries

## System Architecture

1. User sets the alarm using Telegram.
2. The Telegram Bot sends the alarm time to the ESP32.
3. ESP32 stores and continuously monitors the current time.
4. When the alarm time matches, the vibration motor is activated.
5. The pillow vibrates silently to wake the user.
6. Users can stop or snooze the alarm through Telegram.

## Project Modules

* User Module (Telegram Application)
* ESP32 Control Module
* Alarm Scheduling Module
* Communication Module
* Vibration Motor Module
* Power Management Module

## Installation

1. Clone the repository.
2. Open the project in Arduino IDE.
3. Install the required libraries:

   * WiFi.h
   * WiFiClientSecure.h
   * UniversalTelegramBot.h
   * ArduinoJson.h
   * RTClib.h
4. Configure the following:

   * Wi-Fi SSID and Password
   * Telegram Bot Token
   * Telegram Chat ID
5. Upload the code to ESP32.
6. Connect all hardware components as per the circuit diagram.
7. Power ON the device and set alarms using Telegram.

## Telegram Commands

| Command         | Description                     |
| --------------- | ------------------------------- |
| `/start`        | Display available commands      |
| `/set HH:MM:SS` | Set alarm time                  |
| `/time`         | Display current time            |
| `/status`       | Check alarm status              |
| `/stop`         | Stop the active alarm           |
| `/snooze`       | Snooze the alarm for 30 seconds |

## Advantages

* Does not disturb others.
* Comfortable and easy to use.
* Low-cost implementation.
* Energy efficient.
* Suitable for daily use.
* Portable and lightweight.

## Future Enhancements

* Sleep pattern monitoring
* Adjustable vibration intensity
* Multiple alarm support
* Voice assistant integration
* Wireless charging
* Cloud connectivity
* Enhanced mobile application UI

## Results

The Smart Pillow system was successfully tested and demonstrated:

* Accurate alarm triggering
* Reliable Telegram communication
* Effective vibration-based wake-up
* Comfortable user experience
* Stable system performance

## Team Members

* MADAN R K (710724104079)
* MARUTHUPANDI M (710724104085)
* NAVEEN T (710724104101)

## Guide

Mr. E. P. Prakash, M.E. (Ph.D.)
Assistant Professor, Department of Computer Science and Engineering
Dr. N.G.P. Institute of Technology, Coimbatore

## License

This project is developed for academic and educational purposes under the Department of Computer Science and Engineering, Dr. N.G.P. Institute of Technology.

---

*"Wake Up Silently, Start Your Day Comfortably."*
