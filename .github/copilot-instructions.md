# ESP32 Cozmo Robot System - AI Coding Guidelines

This is an ESP32-S3-based intelligent robot system with speech recognition, TTS, vision, and automation capabilities. Built using PlatformIO with Arduino framework and a custom MVC web framework.

## 🏗️ System Architecture

### Core Framework
- **PlatformIO project** with `src_dir = ./app` (main code in `app/` not `src/`)
- **ESP32-MVC-Framework**: Custom web framework with routing, controllers, middleware
- **Modular design**: Core components in `app/core/`, libraries in `lib/`, web routes in `app/web/`
- **FreeRTOS tasks**: Multi-core processing with task-based automation and SR

## 📁 Project Structure

### Root Directory Layout
```
├── platformio.ini              # PlatformIO configuration (environments, libraries, build flags)
├── hiesp.csv / hiesp-8mb.csv    # Custom partition tables for ESP-SR model storage
├── boards/                     # Custom board definitions (ESP32-S3-N16R8, Seeed XIAO)
├── app/                        # Main application code (NOT src/ - custom src_dir)
├── lib/                        # Custom libraries and hardware abstraction layers
├── include/                    # Global headers (Config.h, feature flags)
├── data/                       # LittleFS filesystem content (web assets, config)
├── model/                      # ESP-SR speech recognition models (flashed separately)
├── tools/                      # Build tools and utilities
└── tmp/                        # Temporary build artifacts
```

### Application Code Structure (`app/`)
```
app/
├── app.ino                     # Arduino entry point (setup/loop delegation)
├── Constants.h                 # Global constants, event keys, voice commands
├── core/                       # Core robot functionality modules
│   ├── Audio/                  # Audio recording and note playback (AudioRecorder, Note)
│   ├── Automation/             # Automation system (currently empty, under development)
│   ├── Logic/                  # Logic components
│   │   └── Area/               # Area scanning functionality (ScanArea)
│   ├── Motors/                 # Motor and servo control (MotorControl, ServoControl)
│   ├── Sensors/                # Hardware sensors (Camera, Distance, Orientation, Touch, etc.)
│   └── Utils/                  # Memory management, command mapping (SpiAllocator, CommandMapper)
├── display/                    # Display control and face animation system
│   ├── Display.cpp/.h          # Main display controller
│   ├── DisplayFace.cpp         # Eye animation and expressions
│   ├── DisplayGraphic.cpp      # Graphics primitives
│   ├── DisplayText.cpp         # Text rendering
│   ├── Icons.h                 # Icon definitions
│   └── components/             # Complex display UI components
│       ├── Bar/                # Progress bars and indicators
│       ├── Battery/            # Battery status display
│       ├── Cube3D/             # 3D cube animation
│       ├── Face/               # Advanced eye animation system (Eye, Blink, Transition, etc.)
│       ├── Mic/                # Microphone status visualization
│       ├── SpaceGame/          # Interactive space game
│       ├── Status/             # System status displays
│       └── Weather/            # Weather information display
├── repository/                 # Data models and configuration persistence
├── services/                   # External service integrations
│   ├── GPTService.cpp/.h       # OpenAI GPT integration
│   ├── WeatherService.cpp/.h   # BMKG weather API integration
│   └── WiFiService.cpp/.h      # WiFi connection management
├── setup/                      # System initialization and component setup
│   ├── setup.cpp/.h            # Main setup orchestration
│   └── src/                    # 30+ component-specific setup functions
├── tasks/                      # FreeRTOS task definitions (9 task modules)
│   ├── register.h              # Task registration and spawning
│   └── src/                    # Task implementation files
├── web/                        # Web interface (MVC framework)
│   ├── Controllers/            # API endpoint handlers
│   └── Routes/                 # Route definitions
│       ├── api.cpp             # REST API endpoints
│       ├── web.cpp             # Web page routes
│       ├── websocket.cpp       # Real-time communication
│       └── routes.h            # Route registration header
└── callback/                   # Event system callback registration (6 callback modules)
    ├── register.h              # Notification callback setup
    └── src/                    # Component-specific event handlers
```

### Custom Libraries (`lib/`)
```
lib/
├── ESP_CSR/                    # ESP-SR speech recognition wrapper
├── FileManager/                # LittleFS and SD card file operations
├── I2CManager/                 # Multi-bus I2C management and scanning
├── IOExtern/                   # PCF8575 I2C expander for motor control
├── Logger/                     # Centralized logging system
├── SendTask/                   # Task execution with status tracking
└── Sstring/                    # Memory-efficient string class
```

### Configuration & Data (`include/` & `data/`)
```
include/
├── Config.h.example            # Template configuration file
├── Config.h                    # Hardware pins, feature flags, credentials
└── README.md                   # Configuration documentation

data/                           # LittleFS filesystem content
├── assets/                     # Web application static files
│   ├── bootstrap.bundle.js     # Frontend framework
│   ├── bootstrap.min.css       # Styling
│   └── joy.js                  # Joystick control library
├── cache/                      # API response caching
├── config/                     # Runtime configuration files
│   ├── templates.txt           # GPT behavior templates
│   ├── wifi.json               # WiFi credentials
│   └── wifi.json.sample        # WiFi template
├── database/                   # Static data files
│   └── administrative_regions.csv  # Location database
└── views/                      # Web application HTML
    └── app.html               # Single-page application
```

### Speech Recognition Models (`model/`)
```
model/
├── srmodels.bin               # Combined SR models (flash to 0x47D000)
├── fst/                       # Finite State Transducer (command grammar)
│   ├── commands.txt           # Voice command definitions
│   ├── fst.txt                # Grammar rules
│   └── tokens.txt             # Token mappings
├── mn5q8_en/                  # Multinet wake word model
├── nsnet2/                    # Noise suppression model
├── picoTTS/                   # Text-to-speech voice data
│   ├── en-US_lh0_sg.bin       # Voice synthesis model
│   └── en-US_ta.bin           # Text analysis model
├── vadnet1_medium/            # Voice activity detection
└── wn9_hiesp/                 # Custom wake word model
```

### Key File Purposes

#### Entry Points
- **`app/app.ino`**: Arduino setup/loop delegation to C++ classes
- **`app/setup/setup.cpp`**: Component initialization orchestration
- **`app/tasks/register.h`**: FreeRTOS task spawning

#### Configuration
- **`include/Config.h`**: Hardware pins, feature flags (`MOTOR_ENABLED`, `CAMERA_ENABLED`)
- **`app/Constants.h`**: Event keys, voice commands, system constants
- **`platformio.ini`**: Build configuration, library dependencies

#### Services
- **`app/callback/register.h`**: Event system callback registration
- **`app/web/Routes/`**: Web API endpoint definitions
- **`app/services/`**: External service integrations

#### Hardware Control
- **`app/core/Motors/`**: Motor and servo control via IOExtern
- **`app/core/Sensors/`**: Hardware sensor interfaces
- **`app/display/`**: Complex face animation system

#### Memory Management
- **`lib/Sstring/`**: Custom string class for memory efficiency
- **`app/core/Utils/`**: SpiJsonDocument for large JSON handling

### Navigation Guidelines for AI Agents

1. **Feature Implementation**: Check `include/Config.h` for feature flags before adding code
2. **Event Communication**: Use constants from `app/Constants.h` for notification keys
3. **Hardware Control**: Implement in `app/core/` with proper abstractions
4. **Web APIs**: Add routes in `app/web/Routes/`, controllers in `app/web/Controllers/`
5. **Component Setup**: Add initialization in `app/setup/src/` and register in `setup.cpp`
6. **Task Creation**: Define in `app/tasks/src/`, register in `app/tasks/register.h`
7. **Custom Libraries**: Place hardware-specific code in `lib/` for reusability
8. **Configuration**: Store runtime config in `data/config/`, compile-time in `include/Config.h`

### Memory Management
- Uses **external SPI RAM** extensively via `Utils::SpiJsonDocument` (custom allocator)
- **Never use** standard `DynamicJsonDocument` - always use `Utils::SpiJsonDocument`
- String handling via `Utils::Sstring` (custom string class for memory efficiency)

### Hardware Abstraction
- **IOExtern**: 16-bit I2C expander for motor/sensor control (`lib/IOExtern/`)
- **Conditional compilation**: All features configurable via `Config.h` macros (`MOTOR_ENABLED`, `CAMERA_ENABLED`, etc.)
- **Multi-bus I2C**: Separate buses for display/sensors and extenders

## 🚀 Development Workflows

### Building & Flashing
```bash
# Standard PlatformIO commands
pio run -e esp32s3dev        # Build for ESP32-S3-N16R8
pio run -e esp32s3dev -t upload  # Upload firmware
   
# Flash speech recognition models (required for voice features)
esptool.py --baud 2000000 write_flash 0x47D000 model/srmodels.bin

# Flash PicoTTS models (required for text-to-speech)
esptool.py --baud 2000000 write_flash 0x310000 model/picoTTS/en-US_ta.bin
esptool.py --baud 2000000 write_flash 0x3B0000 model/picoTTS/en-US_lh0_sg.bin
```

### Configuration System
1. Copy `include/Config.h.example` to `include/Config.h`
2. Configure hardware pins, WiFi credentials, API keys
3. Enable/disable features via `#define` macros
4. Configuration persists in LittleFS via `Configuration::set/get`

### Global Variables & Constants
- **Config.h**: Hardware pins, feature flags, and compile-time configuration
- **Constants.h**: Event keys, voice commands, and system constants
- **Always use these files** instead of magic numbers or hardcoded strings

```cpp
// Use Config.h for hardware configuration
#if MOTOR_ENABLED
    motors = new Motors::MotorControl();
    motors->setMotorPins(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
#endif

// Use Constants.h for event communication
notification->send(NOTIFICATION_SPEAKER, &audioData);                           // Not "speaker"
notification->send(NOTIFICATION_DISPLAY, &displayData);                         // Not "display"
notification->send(NOTIFICATION_SR, (void*)EVENT_SR::WAKEWORD);                 // Not "sr_wakeword"
notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::LOOK_LEFT);      // Not "look_left"
notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::PAUSE);    // Not "pause"

// Voice command mapping from Constants.h
static const csr_cmd_t voice_commands[] = {
    {0, "look to left", "LwK To LfFT"},
    {1, "look to right", "LwK To RiT"},
    {2, "close your eyes", "KLbS YeR iZ"},
    {3, "you can play", "Yo KaN PLd"},
    {4, "silent", "SiLcNT"}
    // ESP-SR phonetic representations
};
```

### Web Development
- **Routes**: Defined in `app/web/Routes/` (web.cpp, api.cpp, websocket.cpp)
- **Controllers**: Business logic in `app/web/Controllers/` (AuthController, SystemController)
- **Views**: Single-page app served from `/views/app.html` in LittleFS
- **Authentication**: Session-based with middleware support

## 🔧 Key Patterns & Conventions

### Component Initialization
```cpp
// All components follow this pattern in setup/setup.cpp
extern ComponentType* globalInstance;  // Declared in setup.h

void setupComponent() {
    if (!COMPONENT_ENABLED) return;  // Feature flag check
    globalInstance = new ComponentType();
    if (!globalInstance->init()) {
        logger->error("Component init failed");
        delete globalInstance;
        globalInstance = nullptr;
    }
}
```

### API Response Format
```cpp
// Consistent JSON response structure
Utils::SpiJsonDocument response;
response["success"] = true;
response["data"] = actualData;
response["timestamp"] = millis();
return Response(request.getServerRequest()).status(200).json(response);
```

### Event System
- **Notification class**: Pub/sub for inter-component communication via `esp32-notification`
- **FreeRTOS-style**: Uses void* pointers and string keys like native FreeRTOS APIs
- **Thread-safe**: Mutex-protected for cross-task communication without injection
- **Constants.h**: Defines event types and namespaces for different subsystems
  - **Core notifications**: `NOTIFICATION_SPEAKER`, `NOTIFICATION_DISPLAY`, `NOTIFICATION_AUTOMATION`
  - **Speech Recognition**: `EVENT_SR` namespace (`WAKEWORD`, `COMMAND`, `TIMEOUT`, `PAUSE`, `RESUME`)
  - **Display Control**: `EVENT_DISPLAY` namespace (`WAKEWORD`, `LOOK_LEFT`, `LOOK_RIGHT`, `CLOSE_EYE`, `CLIFF_DETECTED`, `OBSTACLE_DETECTED`)
- **Callback registration**: Components register in `callback/register.h`

```cpp
// Event communication pattern
notification->send("temperature", &sensorValue);  // Producer
void* data = notification->consume("temperature", pdMS_TO_TICKS(1000)); // Consumer
if (data) {
    int* temp = (int*)data;  // Manual casting
    processTemperature(*temp);
}

// Task implementation pattern (from tasks/src/)
void sensorMonitorTask(void* parameter) {
    logger->info("Sensor monitoring task started");
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t updateFrequency = pdMS_TO_TICKS(50);
    
    while (true) {
        vTaskDelayUntil(&lastWakeTime, updateFrequency);
        // Sensor reading logic
        if (sendLog) logger->info("sensor data...");
    }
}
```

### Robot Control Architecture
```cpp
// Motors controlled via IOExtern or direct GPIO
motors->forward(speed);    // Non-blocking movement
motors->setDisplay(display); // Link display for feedback
```

## 🎯 Critical Integration Points

### Dependency Injection & Communication Architecture
- **No Constructor Injection**: Components communicate via global instances and event system
- **Event-Driven**: `esp32-notification` library enables decoupled task communication
- **Global Component Registry**: All components accessible via extern pointers in `setup.h`
- **Notification Keys**: String-based event types defined in `Constants.h`

```cpp
// Instead of dependency injection, use global components + events
extern Motors::MotorControl* motors;     // Global component access
extern Notification* notification;       // Event system

// Component interaction via events (no direct dependencies)
void MotorController::moveForward() {
    motors->forward(speed);
    notification->send("motor_started", &speed);  // Notify other components
}

// Other components consume events independently
void* data = notification->consume("motor_started", pdMS_TO_TICKS(1000));
if (data) {
    int* motorSpeed = (int*)data;
    display->showMotorStatus(*motorSpeed);
}
```

### Voice Recognition Pipeline
1. **Microphone** (analog/I2S) → **ESP-SR** → **Command mapping**
2. **Wake word detection** → **Speech recognition** → **Command execution**
3. Models stored in `model/` directory, flashed separately to partition
4. Phonetic command definitions in `Constants.h` voice_commands array
5. **Phonetic generation**: Use `python tools/multinet_g2p.py --text="new command"` for ESP-SR phonetic representations
6. **Microphone callback**: `mic_fill_callback()` handles volume control and audio processing for ESP-SR system

### AI Integration
- **GPT behavior generation**: Automation system uses gpt-4.1-nano-2025-04-14 to generate robot behaviors
- **Behavior format**: `[ACTION=time][ACTION2=time] *Complete vocalization*` (50 behaviors per batch)
- **Weather integration**: BMKG API integration with caching, location-based forecasts
- **Command validation**: Strict behavior validation prevents invalid GPT responses
- **Automation state**: Core automation framework exists with pause/resume events, but specific automation logic is under development

### Web Framework Flow
```cpp
// Route registration pattern with middleware chaining
void registerApiRoutes(Router* router) {
    router->group("/api/v1", [&](Router& api) {
        api.middleware({"json", "auth"});  // Apply middleware chain
        api.get("/status", [](Request& request) -> Response {
            return SystemController::getStats(request);
        });
    });
}

// Middleware execution chain (from esp32-mvc-framework)
Response executeMiddleware(const std::vector<String>& middleware, Request& request, 
                          std::function<Response(Request&)> finalHandler) {
    // Builds reverse-order chain for proper middleware flow
    std::function<Response(Request&)> next = finalHandler;
    for (auto it = middleware.rbegin(); it != middleware.rend(); ++it) {
        auto middlewareObj = middlewares.find(*it)->second;
        next = [middlewareObj, next](Request& req) -> Response {
            return middlewareObj->handle(req, next);
        };
    }
    return next(request);
}
```

### Task Management
- **Main loop exits** via `vTaskDelete(NULL)` - system runs on FreeRTOS tasks
- **Task registration** in `tasks/register.h`, spawned in `setupTasks()`
- **Watchdog configured** for 120s timeout (long for ESP-SR processing)
- **SendTask library**: Advanced task creation and management with core affinity (`SendTask::createTaskOnCore()`, `SendTask::createLoopTaskOnCore()`, task status monitoring)
- **Task scheduling**: Uses `SendTask` for managed task creation with tracking, plus direct `xTaskCreateUniversal()` and `xTaskCreatePinnedToCore()` for core affinity
- **Task tracking**: Global task ID strings defined in `register.h` for monitoring and cleanup (`taskMonitorerId`, `displayTaskId`, etc.)
- **Task utilities**: `printTaskStatus()` and `cleanupTasks()` for debugging and resource management

### File System Layout
- **LittleFS**: Configuration, web assets, logs (`/data/` directory contents)
- **SD_MMC**: Optional storage for media files
- **Partition table**: Custom `hiesp.csv` for ESP-SR model storage

## ⚠️ Important Constraints

### Memory Limitations
- ESP32-S3 with 16MB flash, 8MB PSRAM
- Use `Utils::SpiJsonDocument` for JSON > 4KB
- Avoid large stack allocations in tasks
- Monitor heap usage: `ESP.getFreeHeap()` and `ESP.getFreePsram()`

### Thread Safety & Display System
- **Display mutex creation**: Must happen in `init()` not constructor (FreeRTOS timing issue)
- **Two-phase display init**: Basic display works in `setup()`, thread-safety enabled after `setupTasks()`
- **Display namespaces**: Current system uses `Display::Display` class (migrated from `Utils::Display`)
- **Face animation**: Complex eye animation system with expressions, blinking, and looking behaviors

### Hardware Dependencies
- **PCF8575** I2C expander required when `MOTOR_IO_EXTENDER=true`
- **Camera models** defined by `CAMERA_MODEL_*` macros
- **I2S pins** fixed for MAX98357 speaker, configurable in Config.h
- **GPIO mapping**: Pin assignments in Config.h follow ESP32-S3 constraints (see pin mapping documentation)

### Framework-Specific
- **Route order matters** - more specific routes must be registered first
- **Global component pointers** managed in setup.h, check for nullptr
- **Authentication required** for most API endpoints via AuthController
- **CORS middleware** needed for web app API calls

## 🔍 Debugging Tips

- **Serial output**: 115200 baud with ESP exception decoder enabled
- **Logging levels**: Configurable via `LOG_LOCAL_LEVEL` and `CORE_DEBUG_LEVEL`
- **Memory debugging**: Monitor via SystemController `/api/v1/system/stats`
- **Component status**: Check global pointers and `logger->error()` messages
- **Network debugging**: mDNS service at `devicename.local`, FTP server for file access
- **Task monitoring**: Use `SendTask::createTaskOnCore()` for trackable task execution with status monitoring, or legacy `Command::Send()` interface
- **Display debugging**: Two-phase init - basic display in setup(), full thread-safety after setupTasks()
- **Voice debugging**: Use `tools/multinet_g2p.py` to generate phonetic representations for new commands
- **Volume control**: Microphone volume multiplier handled in `mic_fill_callback()` for ESP-SR system

## 🎯 GPIO Pin Reference (ESP32-S3)

**Display & I2C**: GPIO 3 (SDA), GPIO 46 (SCL) | **Audio**: GPIO 21,47,14 (mic), GPIO 42,2,41 (speaker) | **Servos**: GPIO 19 (head), GPIO 20 (hand) | **Sensors**: GPIO 0 (ultrasonic trigger), GPIO 45 (echo) | **Motors**: Via PCF8575 I2C expander | **SD Card**: GPIO 39,38,40 (SD_MMC)

When modifying this system, always verify component initialization order in `setupApp()` and ensure hardware feature flags match your target configuration.
