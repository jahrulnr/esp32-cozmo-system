# Cozmo-System Robot Platform

This project implements a versatile ESP32-CAM-based robot platform with live video streaming, servo head/hand control, motor drive capabilities, and system monitoring. It inherits key features from the ESP32-CAM project and adds improved memory management and stability enhancements.

## Status

**⚠️ WORK IN PROGRESS ⚠️**

This project is currently under active development. Core features are being ported and enhanced from the reference ESP32-CAM project. Some functionality may be incomplete or subject to change.

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Architecture](#architecture)
- [Installation](#installation)
- [Project Structure](#project-structure)
- [Utility Components](#utility-components)
- [Safe GPIO Configuration](#safe-gpio-pins-for-esp32-cam)
- [License](#license)

## Features

### Currently Implemented
- **Memory Management System**
  - Optimized SPI RAM Allocator for ArduinoJson 7.x compatibility
  - Efficient buffer management to prevent memory-related crashes
  - PSRAM utilization for large data structures

### Planned Features (In Development)
The following features are planned to be ported from the ESP32-CAM project:

- **Live Camera Streaming**: Real-time video feed with adjustable resolution and quality
- **Servo Control System**: Head/Hand camera movement with speed control (0-200%)
- **Motor Control**: Forward, backward, and turning movements for mobility
- **Web Interface**: Responsive design with touch/keyboard controls
- **System Monitoring**: CPU usage, memory, temperature, and WiFi statistics
- **OLED/LCD Screen Support**: Status display and eye animations
- **I2C Device Management**: Comprehensive bus management and scanning utilities

## Hardware Requirements

- ESP32-CAM module (AI Thinker model recommended)
- Motors and motor driver
- Servo motors for head/hand mechanism
- OLED/LCD display (optional)
- WiFi network for remote control

## Software Requirements

- PlatformIO IDE (recommended) or Arduino IDE with ESP32 support
- Required libraries (automatically managed by PlatformIO):
  - `ESPAsyncWebServer` - Asynchronous web server
  - `AsyncTCP` - Asynchronous TCP library
  - `ArduinoJson` v7.x - JSON processing
  - `WebSockets` - WebSocket communication
  - `ESP32Servo` - Servo motor control
  - `U8g2` - Display graphics for OLED/LCD screens

## Architecture

The project follows a modular architecture that separates concerns and enhances maintainability:

### Component Organization

```
lib/
├── Communication/  # Network and communication services
├── Motors/         # Motor and servo controls
├── Screen/         # Display and UI components
├── Sensors/        # Camera, gyroscope, and other sensors
└── Utils/          # Core utilities and helpers
```

### Main Application Structure

```
app/
├── app.ino         # Main application entry point
├── config.h        # Configuration settings
└── tasks.cpp       # FreeRTOS task definitions
```

## Installation

1. Clone this repository:
   ```sh
   git clone https://github.com/yourusername/cozmo-system.git
   cd cozmo-system
   ```

2. Open the project in PlatformIO IDE.

3. Libraries will be automatically installed by PlatformIO.

4. Copy `app/config.h.example` to `app/config.h` and configure your settings:
   ```cpp
   // WiFi credentials
   #define WIFI_SSID "YOUR_SSID"
   #define WIFI_PASSWORD "YOUR_PASSWORD"
   
   // Pin assignments for motors and servos
   #define SERVO_X_PIN 12  // Pan servo
   #define SERVO_Y_PIN 13  // Tilt servo
   // Additional pin configuration...
   ```

5. Build and upload the project:
   ```sh
   # Using PlatformIO CLI
   pio run --target upload
   
   # Or use the PlatformIO IDE build/upload buttons
   ```

## Project Structure

```
cozmo-system/
├── app/                  # Main application code
├── cert/                 # SSL certificates
├── data/                 # Web interface assets
├── include/              # Global header files
└── lib/                  # Module libraries
    ├── Communication/    # WiFi, WebServer, WebSocket handlers
    ├── Motors/           # Motor and servo control
    ├── Screen/           # Display control
    ├── Sensors/          # Camera, gyroscope, etc.
    └── Utils/            # Utility components
```

## Utility Components

The project includes several utility components designed to enhance functionality:

### SpiAllocator

A custom memory allocator for ArduinoJson that uses external SPI RAM:

```cpp
#include "Utils/SpiAllocator.h"
#include <ArduinoJson.h>

// Create a JSON document in external SPI RAM (4KB capacity)
Utils::SpiJsonDocument<4096> doc;

// Use like a regular JsonDocument
doc["name"] = "Cozmo";
doc["sensors"]["camera"] = "OV2640";
```

### I2CManager

A comprehensive I2C bus management system:

```cpp
#include "Utils/I2CManager.h"

// Get the singleton instance
Utils::I2CManager& i2cManager = Utils::I2CManager::getInstance();

// Initialize a bus
i2cManager.initBus("main", SDA_PIN, SCL_PIN);

// Scan for devices
i2cManager.scanBus("main");
```

### Sstring

A string class that uses external SPI RAM:

```cpp
#include "Utils/Sstring.h"

// Create strings in various ways
Utils::Sstring str1;                 // Empty string
Utils::Sstring str2("Hello World");  // From C-string

// String operations
str1 = str2 + " Robot";     // Concatenation
str1 += "!";                // Append
```

## Safe GPIO Pins for ESP32-CAM

- **GPIO 0, 2, 4, 12, 13, 14, 15**: Safe for PWM, I2C, and other functions.
- **GPIO 16**: Not safe, as it is used for PSRAM.
- **GPIO 1 (UOT), 3 (UOR)**: Safe if not in debug/serial mode. These pins are used for the serial monitor.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---

*Note: This project is a work in progress and derives from the ESP32-CAM project. Features and documentation are being actively developed.*
