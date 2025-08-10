# Copilot Instructions for ESP32 Cozmo Robot System

## Project Overview

This is a comprehensive ESP32-based robotics platform that implements a Cozmo-like robot with AI integration. The system uses PlatformIO for firmware development and includes a Go web server for cloud interface.

## Architecture

### Core Components
- **ESP32 Firmware** (`app/`): Arduino-based real-time control system with FreeRTOS tasks
- **Component Libraries** (`app/lib/`): Modular subsystems (Motors, Sensors, Communication, Audio, Screen)
- **Web Interface** (`data/`): WebSocket-based control panel served from SPIFFS
- **Configuration** (`include/Config.h`): Hardware pin mappings and feature toggles

### Data Flow
1. **WebSocket DTO Protocol**: Standardized JSON messages (`{version: "1.0", type: "command_type", data: {...}}`)
2. **Command System**: Bracket notation commands `[MOVE_FORWARD=500]` parsed by `CommandMapper`
3. **AI Integration**: GPT responses contain embedded commands + natural language
4. **Task Communication**: FreeRTOS tasks with mutex-protected shared state

## Key Development Patterns

### Configuration Management
- Copy `include/Config.h.example` to `include/Config.h` for hardware configuration
- Feature toggles: `#define CAMERA_ENABLED true` pattern throughout
- Board-specific builds: `esp32cam` vs `esp32s3dev` environments in `platformio.ini`

### Component Initialization
Components follow this pattern in `app.ino`:
```cpp
// Global instances declared in app.h
Motors::MotorControl* motors = nullptr;

// Setup functions called in setup()
void setupMotors() {
    if (MOTOR_ENABLED) {
        motors = new Motors::MotorControl(screen);
        motors->init();
    }
}
```

### Command System
Commands use bracket notation parsed by `CommandMapper`:
- Basic: `[MOVE_FORWARD]`, `[BEEP]`, `[CAMERA_START]`
- Parameterized: `[MOVE_FORWARD=500]`, `[TURN_LEFT=2s]`, `[SET_VOLUME=50]`
- Time parsing: `500` (ms), `2s` (seconds), `1m` (minutes)

### WebSocket Communication
Messages follow DTO contract:
```json
{
  "version": "1.0",
  "type": "motor_command",
  "data": {
    "direction": "forward", 
    "speed": 50
  }
}
```

### Audio System
- **I2S Speaker** (MAX98357A): High-quality audio with 16kHz sample rate
- **MP3 Support**: Direct playback via Helix decoder library
- **SPIFFS Storage**: Audio files in `/data/audio/` directory
- **Random Playback**: `playSpeakerRandomMP3()` for behavior sounds

## Build & Debug Workflow

### PlatformIO Commands
```bash
./pio.sh run -t upload         # Build and upload firmware
./pio.sh run -e esp32s3dev     # Build for ESP32-S3
./pio.sh device monitor        # Serial monitoring
```

### VS Code Task
Use "Build and Upload Cozmo System" task for integrated development.

### Debug Stack Traces
```bash
./debug.sh 0x40xxxxx          # Decode crash addresses using addr2line
```

## Critical Integration Points

### FreeRTOS Task Management
- **Camera Streaming**: `cameraStreamTask` handles MJPEG encoding
- **Sensor Monitoring**: `sensorMonitorTask` polls all sensors
- **GPT Processing**: `gptChatTask` handles AI requests asynchronously
- **Automation**: `automationTask` executes scripted behaviors

### I2C Bus Sharing
Multiple devices on shared I2C bus (screen, orientation sensor):
```cpp
#define SCREEN_SDA_PIN 16
#define SCREEN_SCL_PIN 15
#define ORIENTATION_SDA_PIN SCREEN_SDA_PIN  // Shared bus
```

### SPIFFS File System
- **Web Interface**: `/data/index.html` served by AsyncWebServer
- **Audio Files**: `/data/audio/*.mp3` for behavior sounds
- **Configuration**: `/data/config/config.json` for runtime settings
- **Logging**: Debug logs written to SPIFFS

### WebSocket Event Handling
Event handlers in `webserver.cpp` route to `CommandMapper`:
```cpp
webSocket->onEvent([](auto* server, auto* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_DATA) {
        auto doc = WebSocketHandler::parseJsonMessage(data, len);
        commandMapper->executeCommand(doc["data"]["command"]);
    }
});
```

## AI Integration Specifics

### GPT Response Format
AI responses contain embedded commands:
```
[MOVE_FORWARD=1s][BEEP][TURN_LEFT=500] I'm moving forward and turning left as requested!
```

### Automation System
- **Template Behaviors**: Loaded from `/data/config/templates.txt`
- **Dynamic Learning**: `fetchAndAddNewBehaviors()` generates new behaviors via GPT
- **Execution**: Commands extracted and executed by `CommandMapper`

## Hardware-Specific Notes

### ESP32-CAM vs ESP32-S3
- **ESP32-CAM**: Limited GPIO, camera-focused build
- **ESP32-S3**: More GPIO, PSRAM support, preferred for full features
- **Pin Conflicts**: Camera module uses specific GPIO on ESP32-CAM

### Audio Hardware
I2S speaker wiring (MAX98357A):
- BCLK: GPIO 26, WCLK: GPIO 27, DIN: GPIO 25
- **Important**: DIN connects to MAX98357 DIN pin, not SD pin

### Motor Control
- **DC Motors**: H-bridge control with configurable pins
- **Servos**: Standard PWM control for head/hand movement
- **Cliff Detection**: Simple digital sensors for safety

## Testing & Validation

### Web Interface Testing
1. Connect to robot's WiFi AP or configure station mode
2. Navigate to `http://robot-ip/` for control interface
3. Use WebSocket console for command testing

### Hardware Validation
- `setupExtender()`: I2C device scanner for connection verification
- `playSpeakerMP3File("/audio/boot.mp3")`: Audio system test on startup
- Camera streaming: Verify MJPEG feed via web interface

## Common Gotchas

- **Config.h**: Must be created from example file before building
- **SPIFFS Upload**: Use PlatformIO's "Upload Filesystem Image" for web files
- **WiFi Credentials**: Set in Config.h or use AP mode for initial setup
- **Memory Management**: Use PSRAM for large allocations (enabled in platformio.ini)
- **WebSocket JSON**: Always include version field in new DTO format
