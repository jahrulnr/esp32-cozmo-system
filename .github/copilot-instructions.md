# ESP32 Cozmo Robot System - AI Coding Instructions

## Architecture Overview

This is an **ESP32-S3 based robotics system** with dual-core FreeRTOS tasks, AI vision capabilities, speech recognition/synthesis, and a web-based control interface. The system implements a **custom MVC framework** for web API endpoints and uses **PlatformIO with Arduino framework**.

### Core Components Structure
```
app/
├── core/           # Hardware abstraction layers
│   ├── AI/         # Vision ML models (COCO detection, pedestrian)
│   ├── Sensors/    # Camera, distance, orientation, cliff detectors
│   ├── Motors/     # Motor and servo control
│   └── Utils/      # CommandMapper, I/O expanders, allocators
├── web/            # MVC framework (Controllers, Routes)
├── tasks/          # FreeRTOS task implementations
├── display/        # OLED display and face animations
└── services/       # WiFi, GPT, Weather services
```

## Critical Build & Flash Process

**Memory partitioning is complex** - the system uses custom partition tables for AI models:

```bash
# Build for ESP32-S3 (16MB Flash, 8MB PSRAM)
pio run -e esp32s3dev

# Upload firmware 
pio run -e esp32s3dev -t upload

# Flash speech recognition models (REQUIRED for voice features)
esptool.py --baud 2000000 write_flash 0x47D000 model/srmodels.bin

# Flash PicoTTS models (REQUIRED for text-to-speech)
esptool.py --baud 2000000 write_flash 0x310000 model/picoTTS/en-US_ta.bin
esptool.py --baud 2000000 write_flash 0x3B0000 model/picoTTS/en-US_lh0_sg.bin

# Upload web interface files
pio run -e esp32s3dev -t uploadfs
```

**Partition layout**: See `coco_detect_yolo11n.csv` - AI models have dedicated flash partitions that must be flashed separately from firmware.

## Key Configuration Patterns

All hardware features are controlled via `app/Config.h` with `#define` flags:
- **Enable/disable entire subsystems**: `CAMERA_ENABLED`, `MOTOR_ENABLED`, `PICOTTS_ENABLED`
- **GPIO pin assignments**: All pins are configurable (I2C, servos, sensors, audio)
- **Hardware variants**: Different camera models, I/O expander usage
- **Memory management**: PSRAM allocation, task priorities, buffer sizes

**Copy `app/Config.h.example` to `app/Config.h` before building**.

## Task Management & Memory

**Dual-core FreeRTOS architecture**:
- **Core 0**: High-priority tasks (camera, audio processing, AI inference)
- **Core 1**: Lower-priority tasks (sensors, web server, display)
- **120-second watchdog** timeout for all tasks
- **Task registration system** in `tasks/register.h` with cleanup utilities

**Memory constraints**:
- Enable PSRAM with `heap_caps_malloc_extmem_enable(128)`
- AI models load into dedicated flash partitions
- Camera frames use DMA-capable memory
- Web assets must fit in LittleFS partition

## Command System Architecture

**Central automation via CommandMapper** (`core/Utils/CommandMapper.h`):
```cpp
// Execute robot behaviors via command strings
commandMapper->executeCommandString("[TEXT=Hello!][FACE_HAPPY=5s][MOVE_FORWARD=3s]");
```

**Command format**: `[COMMAND=PARAMETER]` where parameter can be duration (`5s`, `1m`) or value.

**Integration points**:
- GPT responses → parsed for embedded commands
- Web API → direct command execution  
- Voice recognition → command mapping
- Automation templates → behavior sequences

## Web Framework Patterns

**Custom MVC framework** (`web/Controllers/`, `web/Routes/`):
- **Authentication required** for robot control APIs
- **WebSocket + REST API** dual interface
- **Session management** with `sessions[]` global array
- **Route registration** in `routes.h` with separate API/web/WebSocket routes

**API patterns**:
- `/api/v1/robot/motor/move` - Robot movement commands
- `/api/v1/robot/sensors` - Sensor data aggregation
- WebSocket for real-time bidirectional communication

## Hardware Integration Patterns

**I2C device management** via `I2CManager` with address scanning:
- `0x20` - PCF8575 I/O expander (motors/cliff detectors)
- `0x3C` - SSD1306 OLED display
- `0x68` - MPU6050 orientation sensor

**Sensor polling architecture**:
- Each sensor class has `update()` method called from dedicated tasks
- Sensor data aggregated in `tasks/src/sensor.cpp`
- Thread-safe access via task synchronization

**Display system** (`display/`):
- **Thread-safety critical** - display updates from dedicated task only
- Face animation system with predefined expressions
- Graphics, text, and face rendering separated into different modules

## AI Vision Pipeline

**COCO object detection**:
- Model loaded from flash partition during setup
- Camera frames processed via `cocoHandlerTask` and `cocoFeedTask`
- Results available globally via `cocoResult` pointer
- **Camera blocks ESP-SR** - avoid simultaneous use

**Integration pattern**:
```cpp
// Setup in setup.cpp
setupCoco();        // Load AI model from flash
setupCamera();      // Initialize camera sensor

// Tasks in tasks/src/coco.cpp  
cocoHandlerTask();  // Process detections
cocoFeedTask();     // Feed camera frames
```

## Audio System Architecture

**Dual audio pipeline**:
- **I2S microphone** (`GPIO 21, 47, 14`) → speech recognition 
- **I2S speaker** (`GPIO 42, 2, 41`) → PicoTTS output
- **Speech recognition** models in dedicated flash partitions
- **PicoTTS** requires separate model flashing

**Integration with automation**:
- Voice commands → CommandMapper execution
- GPT responses → TTS output
- Audio recording → file storage in `/recordings`

## Development Workflow

**Debugging**:
- Monitor via `pio device monitor` with exception decoder
- Task status via `printTaskStatus()` function
- Logger system writes to both Serial and files
- Memory usage tracking via heap functions

**Common issues**:
- **Brownout detection** disabled in main loop
- **Camera and ESP-SR conflict** - don't enable simultaneously  
- **I2C device conflicts** - check address scanning output
- **Memory fragmentation** - use heap caps for large allocations

**Testing robot functions**:
- Web interface at `http://[device-ip]/views/app.html`
- REST API testing via `/api/v1/` endpoints
- Voice command testing requires proper microphone placement
- Motor movement testing needs sufficient power supply

## File Organization Conventions

- **Hardware interfaces**: `core/Sensors/`, `core/Motors/`
- **Business logic**: `services/`, `core/Utils/`
- **Task implementations**: `tasks/src/`
- **Web interface**: `web/` (backend), `data/` (frontend)
- **Configuration**: `app/Config.h`, partition CSV files
- **Models**: `model/` directory with binary files

When adding new features, follow the existing setup → task → integration pattern used throughout the codebase.
