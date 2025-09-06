# ESP32 Cozmo Robot System - AI Coding Guidelines

This is an ESP32-S3-based intelligent robot system with speech recognition, TTS, vision, and automation capabilities. Built using PlatformIO with Arduino framework and a custom MVC web framework.

## 🏗️ System Architecture

### Core Framework
- **PlatformIO project** with `src_dir = ./app` (main code in `app/` not `src/`)
- **ESP32-MVC-Framework**: Custom web framework with routing, controllers, middleware
- **Modular design**: Core components in `app/core/`, libraries in `lib/`, web routes in `app/web/`
- **FreeRTOS tasks**: Multi-core processing with task-based automation and SR

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

# Flash ESP-SR models separately (required for voice recognition)
esptool.py --baud 2000000 write_flash 0x47D000 model/srmodels.bin
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
  - **Automation Control**: `EVENT_AUTOMATION` namespace (`PAUSE`, `RESUME`)
- **Callback registration**: Components register in `callback/register.h`

```cpp
// Event communication pattern
notification->send("temperature", &sensorValue);  // Producer
void* data = notification->consume("temperature", pdMS_TO_TICKS(1000)); // Consumer
if (data) {
    int* temp = (int*)data;  // Manual casting
    processTemperature(*temp);
}
```

### Robot Control Architecture
```cpp
// Motors controlled via IOExtern or direct GPIO
motors->forward(speed);    // Non-blocking movement
motors->setDisplay(display); // Link display for feedback

// Automation system with GPT integration
automation->start();       // Background behavior generation
automation->updateManualControlTime(); // Pause automation on manual control
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

### Automation & AI Integration
- **GPT behavior generation**: Automation system uses gpt-4.1-nano-2025-04-14 to generate robot behaviors
- **Behavior format**: `[ACTION=time][ACTION2=time] *Complete vocalization*` (50 behaviors per batch)
- **Weather integration**: BMKG API integration with caching, location-based forecasts
- **Command validation**: Strict behavior validation prevents invalid GPT responses

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
- **SendTask library**: Command execution with task tracking (`Command::Send()`, task status monitoring)
- **Task scheduling**: Uses `xTaskCreateUniversal()` and `xTaskCreatePinnedToCore()` for core affinity

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
- **Task monitoring**: Use `Command::Send()` for trackable task execution with status monitoring
- **Display debugging**: Two-phase init - basic display in setup(), full thread-safety after setupTasks()

## 🎯 GPIO Pin Reference (ESP32-S3)

**Display & I2C**: GPIO 3 (SDA), GPIO 46 (SCL) | **Audio**: GPIO 21,47,14 (mic), GPIO 42,2,41 (speaker) | **Servos**: GPIO 19 (head), GPIO 20 (hand) | **Sensors**: GPIO 0 (ultrasonic trigger), GPIO 45 (echo) | **Motors**: Via PCF8575 I2C expander | **SD Card**: GPIO 39,38,40 (SD_MMC)

When modifying this system, always verify component initialization order in `setupApp()` and ensure hardware feature flags match your target configuration.
