# Cozmo-System Robot Platform

This project implements a versatile ESP32-CAM-based robot platform featuring live video streaming, servo-controlled head/hand movements, motor drive capabilities, and system monitoring. It extends the ESP32-CAM project with enhanced memory management and improved stability.

## Dashboard Interface

The platform provides a responsive, professional web dashboard for IoT device control and monitoring, featuring a modern, GitHub-inspired dark theme.

### Dashboard Features

- **Camera Control:** Binary-based live streaming with memory optimization, snapshot capture, and configurable settings
- **Motion Sensors:** Real-time gyroscope and accelerometer data with 3D visualization
- **Motion Control:** Interactive 2D joysticks for precise servo and motor control
- **Command Interface:** Text-based command system with chat functionality
- **Network Management:** WiFi network scanning and connection configuration
- **System Console:** Real-time system logging and command execution
- **File System:** Integrated file upload, download, and management interface
- **Security:** Role-based authentication system with secure access control

## Status

**⚠️ WORK IN PROGRESS ⚠️**

This project is currently under active development. Core features are being ported and enhanced from the reference ESP32-CAM project. Some functionality may be incomplete or subject to change.

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Architecture](#architecture)
- [Installation](#installation)
- [Communication Protocol](#communication-protocol)
- [Project Structure](#project-structure)
- [Utility Components](#utility-components)
- [Web Application](#web-application)
- [Safe GPIO Configuration](#safe-gpio-pins-for-esp32-cam)
- [Documentation](#documentation)
- [License](#license)

## Features

### Currently Implemented
- **Memory Management System**
  - Optimized SPI RAM allocation for ArduinoJson 7.x integration
  - Memory-safe buffer management system
  - Efficient PSRAM utilization for data operations
- **Web Interface**
  - Responsive design with GitHub-inspired dark theme
  - Single-page application architecture
  - Binary-optimized WebSocket communication
  - Mobile-optimized interface with collapsible navigation
- **Access Control**
  - Role-based authentication system
  - Multi-user session management (up to 5 concurrent users)
- **File System Management**
  - Complete file system operations interface
  - Support for text and binary file handling
- **Communication Layer**
  - WebSocket-based real-time messaging
  - Standardized DTO communication protocol
  - Binary-optimized camera streaming

### Planned Features
The following enhancements are under development:

- **Video System:** Real-time streaming with configurable quality settings
- **Motion System:** Precision head/hand movement control (0-200% speed range)
- **Navigation System:** Multi-directional movement capabilities
- **Interface System:** Touch and keyboard-optimized controls
- **Monitoring System:** Real-time performance metrics tracking
- **Display System:** OLED/LCD status display with animation support
- **I2C Framework:** Advanced device management and configuration

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
app/lib/
├── Communication/  # Network and communication services
├── Motors/         # Motor and servo controls
├── Screen/         # Display and UI components
├── Sensors/        # Camera, gyroscope, and other sensors
└── Utils/          # Core utilities and helpers
```

### Main Application Structure

```
app/
├── lib/            # Component Organization
├── app.ino         # Main application entry point
├── websocket.cpp   # WebSocket event handling
├── camera.cpp      # Camera initialization and control
├── wifi.cpp        # WiFi connection management
├── webserver.cpp   # Web server setup
└── tasks.cpp       # FreeRTOS task definitions
```

### Configuration
```
include/
└── Config.h        # Configuration settings
```

## Installation

1. Clone this repository:
   ```sh
   git clone https://github.com/yourusername/cozmo-system.git
   cd cozmo-system
   ```

2. Open the project in PlatformIO IDE.

3. Libraries will be automatically installed by PlatformIO.

4. Copy `include/Config.h.example` to `include/Config.h` and configure your settings:
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

## Communication Protocol

The system uses WebSockets for real-time bidirectional communication between the robot and clients.

### Authentication

All WebSocket commands except `login` require authentication:

```json
{
  "type": "login",
  "data": {
    "username": "admin",
    "password": "admin"
  }
}
```

### Message Format

All WebSocket messages follow a standardized Data Transfer Object (DTO) format:

```json
{
  "version": "1.0",
  "type": "command_type",
  "data": {
    // Command-specific properties
  }
}
```

### Key Command Types

- **Motor Control**: `motor_command` - Control robot movement
- **Camera Control**: `camera_command` - Start/stop streaming, change resolution
- **System Status**: `system_status` - Request system information
- **File Operations**: `list_files`, `read_file`, `delete_file` - Manage files
- **WiFi Management**: `wifi_list`, `connect_wifi` - Configure WiFi

See the [`docs/dto_contract/README.md`](docs/dto_contract/README.md) file for complete communication protocol details.

## Communication Subsystems

### SPI Communication
The platform includes a robust SPI communication system for master-slave interaction:

- **Multiple Device Support:** Register and communicate with multiple slave devices
- **Ping Protocol:** Verify communication with connected devices
- **Data Transfer:** Efficiently send and request data from slave devices
- **Camera Data Transfer:** Request and receive camera/pixel data from slave devices
- **Thread Safety:** Mutex-protected operations for reliable multi-threaded access

See [Camera Data SPI Transfer Documentation](docs/camera_data_spi_transfer.md) for details on the camera data transfer protocol.

## Project Structure

```
cozmo-system/
├── app/                  # Main application code
├── cert/                 # SSL certificates
├── data/                 # Web interface assets
│   ├── config/           # Configuration files
│   ├── css/              # Stylesheets
│   └── js/               # JavaScript files
├── docs/                 # Documentation
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
Utils::SpiJsonDocument doc;

// Use like a regular JsonDocument
doc["name"] = "Cozmo";
doc["sensors"]["camera"] = "OV2640";
```

### FileManager

A utility for managing files on the SPIFFS file system:

```cpp
#include "Utils/FileManager.h"

Utils::FileManager fileManager;
if (fileManager.init()) {
  // List files in a directory
  std::vector<Utils::FileManager::FileInfo> files = fileManager.listFiles("/");
  
  // Write a file
  fileManager.writeFile("/config/settings.json", jsonString);
  
  // Delete a file
  fileManager.deleteFile("/logs/old.log");
}
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

## Web Application

The dashboard interface is served from the `data/` directory and includes:

- **Real-time monitoring** of system status
- **Interactive controls** for robot movement
- **Camera streaming** with adjustable settings
- **File management interface** for uploading/downloading files
- **WiFi configuration** for connectivity management

## Safe GPIO Pins for ESP32-CAM

- **GPIO 0, 2, 4, 12, 13, 14, 15**: Safe for PWM, I2C, and other functions.
- **GPIO 16**: Not safe, as it is used for PSRAM.
- **GPIO 1 (UOT), 3 (UOR)**: Safe if not in debug/serial mode. These pins are used for the serial monitor.

## Documentation

Detailed documentation is available in the `docs/` directory:

- **[DOCUMENTATION.md](docs/DOCUMENTATION.md)**: Complete system documentation
- **[DTO_FORMAT.md](docs/DTO_FORMAT.md)**: WebSocket communication protocol
- **[MOTORS_SENSORS_DOCS.md](docs/MOTORS_SENSORS_DOCS.md)**: Motors and sensors documentation
- **[SCREEN_FACE_DOCS.md](docs/SCREEN_FACE_DOCS.md)**: Display and face animation system
- **[UTILS_I2C_DOCS.md](docs/UTILS_I2C_DOCS.md)**: Utility classes and I2C management

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---

*Note: This project is a work in progress and derives from the ESP32-CAM project. Features and documentation are being actively developed.*
