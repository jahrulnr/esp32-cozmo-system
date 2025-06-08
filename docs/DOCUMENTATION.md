# Cozmo-System Robot Platform Documentation

## Overview

Cozmo-System is a versatile ESP32-CAM-based robot platform that provides live video streaming, servo head/hand control, motor drive capabilities, and comprehensive system monitoring. The project is designed with a modular architecture that enhances maintainability and extensibility.

This documentation provides a detailed overview of the system architecture, components, and implementation details to help developers understand and extend the platform.

## Table of Contents

1. [System Architecture](#system-architecture)
2. [Core Components](#core-components)
    - [Sensors](#sensors)
    - [Motors](#motors)
    - [Screen](#screen)
    - [Communication](#communication)
    - [Utils](#utils)
3. [Hardware Requirements](#hardware-requirements)
4. [Software Dependencies](#software-dependencies)
5. [Web Dashboard](#web-dashboard)
6. [API Reference](#api-reference)
7. [Configuration](#configuration)
8. [Safe GPIO Usage](#safe-gpio-usage)

---

## System Architecture

The Cozmo-System follows a modular architecture that separates concerns into distinct components:

### Component Organization

```
app/lib/
├── Communication/  # Network and communication services
├── Motors/         # Motor and servo controls
├── Screen/         # Display and UI components
├── Sensors/        # Camera, gyroscope, and other sensors
└── Utils/          # Core utilities and helpers
```

### Main Application

```
app/
├── lib/            # Component Organization
├── app.ino         # Main application entry point
└── tasks.cpp       # FreeRTOS task definitions
```

### Web Interface Assets

```
data/
├── index.html      # Main dashboard interface
├── config/         # Configuration files
├── css/            # Stylesheet files
├── js/             # JavaScript files
```

### Configuration

```
include/
└── Config.h        # Central configuration settings
```

---

## Core Components

### Sensors

#### Camera (`Sensors::Camera`)

The Camera class manages the ESP32-CAM module, handling initialization, configuration, and frame capture.

**Key Features:**
- Camera initialization with various resolution settings
- Frame capture and buffer management
- Resolution adjustment
- Camera settings control (brightness, contrast, saturation)
- Streaming interval management

**Example Usage:**
```cpp
Sensors::Camera camera;
camera.init();
camera_fb_t* frame = camera.captureFrame();
// Process frame
camera.returnFrame(frame);
```

**Key Files:**
- `lib/Sensors/Camera.h`
- `lib/Sensors/Camera.cpp`
- `lib/Sensors/CameraConfig.h`

#### Gyroscope (`Sensors::Gyro`)

The Gyro class provides access to the MPU6050 gyroscope and accelerometer data.

**Key Features:**
- I2C-based communication with MPU6050
- Rotation measurement in degrees per second
- Acceleration measurement in g (9.81 m/s²)
- Calibration functionality
- Magnitude calculation for acceleration

**Example Usage:**
```cpp
Sensors::Gyro gyro;
gyro.init(14, 15);  // SDA, SCL pins
gyro.update();
float xRotation = gyro.getX();
float zAccel = gyro.getAccelZ();
```

**Key Files:**
- `lib/Sensors/Gyro.h`
- `lib/Sensors/Gyro.cpp`

#### CliffDetector (`Sensors::CliffDetector`)

The CliffDetector class manages infrared sensors to detect edges and prevent falls.

**Key Features:**
- Dual sensor support (left and right)
- Calibration capability
- Edge detection logic

**Key Files:**
- `lib/Sensors/CliffDetector.h`
- `lib/Sensors/CliffDetector.cpp`

---

### Motors

#### MotorControl (`Motors::MotorControl`)

The MotorControl class manages the robot's movement via DC motors.

**Key Features:**
- Control of dual motors (left and right)
- Direction control (forward, backward, left, right, stop)
- Speed adjustment
- Timed movement

**Example Usage:**
```cpp
Motors::MotorControl motors;
motors.init(2, 4, 13, 12);  // left1, left2, right1, right2 pins
motors.move(Motors::MotorControl::FORWARD, 1000);  // Move forward for 1 second
```

**Key Files:**
- `lib/Motors/MotorControl.h`
- `lib/Motors/MotorControl.cpp`

#### ServoControl (`Motors::ServoControl`)

The ServoControl class manages servo motors for camera head/hand movement.

**Key Features:**
- Control of head and hand servos
- Angle setting (0-180 degrees)
- PWM management via ESP32 Servo library

**Example Usage:**
```cpp
Motors::ServoControl servos;
servos.init(12, 13);  // head, hand pins
servos.setHead(90);   // Center position
servos.setHand(45);   // 45-degree position
```

**Key Files:**
- `lib/Motors/ServoControl.h`
- `lib/Motors/ServoControl.cpp`


### Screen

#### Screen (`Screen::Screen`)

The Screen class manages the OLED display for showing status and animations.

**Key Features:**
- Integration with U8g2 library for OLED control
- Drawing utilities (text, shapes, lines)
- Semaphore-based thread safety

**Example Usage:**
```cpp
Screen::Screen screen;
screen.init(14, 15);  // SDA, SCL pins
screen.clear();
screen.drawCenteredText(20, "Cozmo System");
screen.update();
```

**Key Files:**
- `lib/Screen/Screen.h`
- `lib/Screen/Screen.cpp`

#### Face System

The Face component provides animated eye expressions on the OLED display.

**Key Features:**
- Animated eye movements and blinking
- Various expressions (normal, angry, happy, etc.)
- Configurable eye parameters
- Randomized behaviors for lifelike animations

**Key Files:**
- `lib/Screen/Face/*.h`
- `lib/Screen/Face/*.cpp`

---

### Utils

#### Logger (`Utils::Logger`)

The Logger class provides a centralized logging system with multiple output options.

**Key Features:**
- Singleton design pattern
- Multiple log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Output to Serial and/or file (SPIFFS)
- Log level filtering

**Example Usage:**
```cpp
Utils::Logger& logger = Utils::Logger::getInstance();
logger.init(true, true, "/logs.txt");  // Serial and file output
logger.setLogLevel(Utils::LogLevel::INFO);
logger.info("System initialized");
logger.error("Failed to connect to WiFi");
```

**Key Files:**
- `lib/Utils/Logger.h`
- `lib/Utils/Logger.cpp`

#### I2CManager (`Utils::I2CManager`)

The I2CManager class provides a comprehensive I2C bus management system.

**Key Features:**
- Singleton design pattern
- Multiple I2C bus management
- Thread-safe access via FreeRTOS mutexes
- Device detection and scanning
- Register read/write operations

**Example Usage:**
```cpp
Utils::I2CManager& i2cManager = Utils::I2CManager::getInstance();
i2cManager.initBus("main", SDA_PIN, SCL_PIN, 400000);  // name, pins, frequency
i2cManager.scanBus("main");  // Print all detected devices

uint8_t value;
i2cManager.readRegister("main", 0x68, 0x3B, value);  // Bus, device, register, result
```

**Key Files:**
- `lib/Utils/I2CManager.h`
- `lib/Utils/I2CManager.cpp`

#### FileManager (`Utils::FileManager`)

The FileManager class provides an interface to the SPIFFS file system.

**Key Features:**
- File operations (read, write, append, delete)
- Directory operations (create, remove, list)
- File information (existence, size)

**Example Usage:**
```cpp
Utils::FileManager fileManager;
fileManager.init();
fileManager.writeFile("/config.txt", "setting=value");
String content = fileManager.readFile("/config.txt");
```

**Key Files:**
- `lib/Utils/FileManager.h`
- `lib/Utils/FileManager.cpp`

#### SpiAllocator (`Utils::SpiAllocator`)

A custom memory allocator for ArduinoJson that uses external SPI RAM.

**Key Features:**
- Integration with ArduinoJson library
- External SPI RAM usage for large JSON documents
- Memory efficiency for resource-constrained devices

**Example Usage:**
```cpp
#include "Utils/SpiAllocator.h"
#include <ArduinoJson.h>

// Create a JSON document in external SPI RAM
Utils::SpiJsonDocument doc;
doc["name"] = "Cozmo";
doc["sensors"]["camera"] = "OV2640";
```

**Key Files:**
- `lib/Utils/SpiAllocator.h`

#### Base64 (`Utils::Base64`)

Utility for Base64 encoding and decoding, useful for binary data transmission.

**Key Files:**
- `lib/Utils/Base64.h`
- `lib/Utils/Base64.cpp`

---

## Hardware Requirements

- ESP32-CAM module (AI Thinker model recommended)
- Motors and motor driver circuit
- Servo motors for head/hand mechanism
- OLED/LCD display (optional)
- MPU6050 gyroscope/accelerometer module
- WiFi network for remote control

### Pin Configuration

#### ESP32-CAM AI Thinker Model

**Camera Pins:**
```
PWDN_GPIO_NUM 32
RESET_GPIO_NUM -1
XCLK_GPIO_NUM 0
SIOD_GPIO_NUM 26
SIOC_GPIO_NUM 27
Y9_GPIO_NUM 35
Y8_GPIO_NUM 34
Y7_GPIO_NUM 39
Y6_GPIO_NUM 36
Y5_GPIO_NUM 21
Y4_GPIO_NUM 19
Y3_GPIO_NUM 18
Y2_GPIO_NUM 5
VSYNC_GPIO_NUM 25
HREF_GPIO_NUM 23
PCLK_GPIO_NUM 22
```

**Safe GPIO Pins for Other Functions:**
- GPIO 0, 2, 4, 12, 13, 14, 15: Safe for PWM, I2C, and other functions.
- GPIO 1 (UOT), 3 (UOR): Safe if not in debug/serial mode (used for serial communication).
- GPIO 16: Not safe (used for PSRAM).

---

## Software Dependencies

- Arduino core for ESP32
- PlatformIO IDE (recommended) or Arduino IDE with ESP32 support
- Required libraries:
  - `ESPAsyncWebServer` - Asynchronous web server
  - `AsyncTCP` - Asynchronous TCP library
  - `ArduinoJson` v7.x - JSON processing
  - `WebSockets` - WebSocket communication
  - `ESP32Servo` - Servo motor control
  - `U8g2` - Display graphics for OLED/LCD screens

---

## Web Dashboard

The system features a responsive, professional dashboard interface for IoT device control and monitoring. Built with a modern, GitHub-inspired dark theme.

### Dashboard Features

- **Camera:** Binary-based live streaming for memory efficiency, snapshot capture, and camera settings
- **Gyro & Accelerometer:** Real-time sensor data visualization with 3D visualization
- **Joystick Controls:** Interactive 2D joysticks for servo and motor control
- **Chat Card:** Built-in communication system for controlling via text commands
- **WiFi Manager:** WiFi network scanning and connection management
- **Debug Console:** Real-time logging and command execution
- **File Manager:** Upload, download, and manage files on the device
- **Authentication:** Secure login system to protect access

---

## API Reference

### WebSocket API

The system uses WebSockets for real-time communication between the web interface and the ESP32. The WebSocket messages follow a standard format:

```json
{
  "type": "message_type",
  "data": {
    // message-specific data
  }
}
```

**Common Message Types:**

1. **sensor_data**: Gyroscope and accelerometer data
   ```json
   {
     "type": "sensor_data",
     "data": {
       "gyro": {
         "x": "1.23",
         "y": "0.45",
         "z": "-0.67"
       },
       "accel": {
         "x": "0.12",
         "y": "0.34",
         "z": "0.89",
         "magnitude": "1.23"
       }
     }
   }
   ```

2. **motor_command**: Control motors
   ```json
   {
     "type": "motor_command",
     "data": {
       "left": 200,
       "right": 200,
       "duration": 1000
     }
   }
   ```

3. **camera_command**: Control camera
   ```json
   {
     "type": "camera_command",
     "data": {
       "action": "snapshot|zoom|set_resolution|set_setting",
       "resolution": "VGA|SVGA|XGA|...",
       "direction": "in|out",
       "setting": "brightness|contrast|saturation",
       "value": 1
     }
   }
   ```

---

## Configuration

The system configuration is managed through `include/Config.h`. A template `Config.h.example` is provided as a starting point. Key configuration items include:

- WiFi credentials
- Pin assignments for motors and servos
- Display parameters
- Camera settings
- General system parameters

Example configuration:
```cpp
// WiFi credentials
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
   
// Pin assignments for motors and servos
#define SERVO_X_PIN 12  // Pan servo
#define SERVO_Y_PIN 13  // Tilt servo

// Additional pin configuration...
```

---

## Safe GPIO Usage

When working with the ESP32-CAM module, be aware of the following GPIO constraints:

- **GPIO 0, 2, 4, 12, 13, 14, 15**: Safe for general use with PWM, I2C, and other functions.
- **GPIO 16**: Not safe, as it is used for PSRAM.
- **GPIO 1 (UOT), 3 (UOR)**: Safe if not in debug mode. These pins are used for serial monitor communication.

Always refer to the ESP32-CAM pinout diagram when planning your hardware connections to avoid interfering with the camera or other built-in functionality.
