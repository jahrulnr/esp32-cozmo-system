# Utils & I2C Components Documentation

## Overview

The Utils directory contains core utility components for the Cozmo-System project that provide essential functionality across the system. ThThe I2CScanner class provides comprehensive utilities for detecting I2C devices on a bus, identifying common components, and diagnosing connection issues. It is designed to simplify the process of identifying and troubleshooting I2C peripherals during both development and production stages of your project.

For detailed troubleshooting information, refer to the dedicated [I2C Troubleshooting Guide](./I2C_TROUBLESHOOTING.md).

For information about specific compatible devices, see the [I2C Compatible Devices](./I2C_COMPATIBLE_DEVICES.md) reference.

### Features utilities include logging, I2C management, file management, and memory optimization tools.

## Table of Contents

1. [Logger](#logger)
2. [I2CManager](#i2cmanager)
3. [I2CScanner](#i2cscanner)
4. [FileManager](#filemanager)
5. [SpiAllocator](#spiallocator)
6. [Base64](#base64)
7. [HealthCheck](#healthcheck)
8. [Sstring](#sstring)

---

## Logger

The `Logger` class provides a centralized logging system with multiple output options and log level filtering.

### Features

- Singleton design pattern ensures system-wide access to a single logger instance
- Multiple log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Output to Serial and/or file (SPIFFS)
- Log level filtering to control verbosity

### API Reference

```cpp
namespace Utils {

enum class LogLevel {
    DEBUG,    // Detailed information for debugging
    INFO,     // General information
    WARNING,  // Warning conditions
    ERROR,    // Error conditions
    CRITICAL  // System-critical errors
};

class Logger {
public:
    static Logger& getInstance();
    
    bool init(bool serialEnabled = true, 
              bool fileEnabled = false, 
              const String& fileName = "/logs.txt");
              
    void setLogLevel(LogLevel level);
    
    void debug(const String& message);
    void info(const String& message);
    void warning(const String& message);
    void error(const String& message);
    void critical(const String& message);
    void log(LogLevel level, const String& message);

private:
    // Private constructor and member variables
};
}
```

### Example Usage

```cpp
#include "Utils/Logger.h"

void setup() {
    // Get the singleton instance
    Utils::Logger& logger = Utils::Logger::getInstance();
    
    // Initialize with both serial and file output
    logger.init(true, true, "/logs.txt");
    
    // Set minimum log level
    logger.setLogLevel(Utils::LogLevel::INFO);
    
    // Log messages
    logger.info("System initialization started");
    logger.debug("This message won't be logged due to log level");
    logger.error("Failed to connect to WiFi");
}
```

### Implementation Details

The Logger follows the singleton pattern, ensuring only one instance exists throughout the application. It manages both Serial and SPIFFS file output streams and filters log messages based on the configured log level.

Each log message is formatted with a timestamp, log level, and the message text:
```
1234567 [INFO] System started
```

---

## I2CManager

The `I2CManager` class provides a comprehensive I2C bus management system with support for multiple buses, mutex protection, and common operations.

### Features

- Singleton design pattern ensures system-wide access to a single manager instance
- Support for multiple I2C buses with named references
- Thread-safe access via FreeRTOS mutexes
- Device detection and bus scanning
- Register read/write operations with error handling

### API Reference

```cpp
namespace Utils {

class I2CManager {
public:
    static I2CManager& getInstance();
    
    // Bus initialization
    bool initBus(const char* busName, int sda, int scl, 
                uint32_t frequency = 100000, bool useWire = true);
    
    // Get TwoWire instance for a bus
    TwoWire* getBus(const char* busName);
    
    // Check if a device is present on a bus
    bool devicePresent(const char* busName, byte address);
    
    // Write a byte to a device register
    bool writeRegister(const char* busName, byte deviceAddress, 
                      uint8_t registerAddress, uint8_t data);
    
    // Read a byte from a device register
    bool readRegister(const char* busName, byte deviceAddress, 
                     uint8_t registerAddress, uint8_t &result);
    
    // Read multiple bytes from a device register
    bool readRegisters(const char* busName, byte deviceAddress, 
                      uint8_t registerAddress, uint8_t *buffer, uint8_t length);
    
    // Scan a bus for I2C devices
    void scanBus(const char* busName);

private:
    // Private constructor and member variables
    // Bus information structure and map
    // Mutex handling methods
};
}
```

### Example Usage

```cpp
#include "Utils/I2CManager.h"

void setup() {
    // Get the singleton instance
    Utils::I2CManager& i2cManager = Utils::I2CManager::getInstance();
    
    // Initialize a bus named "main"
    i2cManager.initBus("main", SDA_PIN, SCL_PIN, 400000);
    
    // Scan for devices
    i2cManager.scanBus("main");
    
    // Check if a device is present
    if (i2cManager.devicePresent("main", 0x68)) {
        // Read from a device register
        uint8_t value;
        if (i2cManager.readRegister("main", 0x68, 0x3B, value)) {
            Serial.printf("Register value: 0x%02X\n", value);
        }
    }
}
```

### Implementation Details

The I2CManager uses a map to store information about each bus, including:
- TwoWire instance for I2C communication
- Mutex for thread-safe access
- Pin configurations and frequency
- Default status flag

All operations on an I2C bus are protected by a mutex to prevent concurrent access issues in a multi-tasking environment. This is particularly important when multiple components (like a display and sensors) share the same I2C bus.

---

## I2CScanner

The `I2CScanner` class provides utilities for detecting I2C devices on a bus and diagnosing connection issues. It is designed to simplify the process of identifying and troubleshooting I2C peripherals during development and production.

### Features

- Static methods for easy access without instantiation
- Comprehensive bus scanning with address reporting
- Device presence checking
- Custom I2C bus initialization and scanning
- Address resolution to identify common I2C devices automatically
- Detailed diagnostic information for connection troubleshooting
- Support for multiple I2C buses concurrently
- Non-blocking scanning modes for use during runtime

### API Reference

```cpp
namespace Utils {

class I2CScanner {
public:
    // Scan an I2C bus for devices
    static int scan(TwoWire& wire = Wire, 
                   uint8_t startAddress = 1, 
                   uint8_t endAddress = 127, 
                   bool printOutput = true);
    
    // Initialize an I2C bus and scan for devices
    static int initAndScan(int sda, int scl, 
                          uint32_t frequency = 100000, 
                          TwoWire& wire = Wire);
    
    // Check if a specific I2C device is present
    static bool devicePresent(uint8_t address, 
                             TwoWire& wire = Wire);
                             
    // Identify a device by its address and signature
    static String identifyDevice(uint8_t address, TwoWire& wire = Wire);
    
    // Enhanced scan with device identification
    static void advancedScan(TwoWire& wire = Wire);
    
    // Non-blocking scan operations
    static void beginAsyncScan(TwoWire& wire = Wire);
    static bool isAsyncScanComplete();
    static std::vector<uint8_t> getAsyncScanResults();
    
    // Get detailed diagnostic information for a device
    static bool testDeviceConnection(uint8_t address, 
                                   TwoWire& wire = Wire,
                                   bool printOutput = true);
                                   
    // Diagnose common I2C issues
    static void diagnoseConnectionIssues(uint8_t address,
                                       TwoWire& wire = Wire);
};
}
```

### Example Usage

```cpp
#include "Utils/I2CScanner.h"

void setup() {
    Serial.begin(115200);
    
    // Scan the default I2C bus
    int deviceCount = Utils::I2CScanner::scan();
    Serial.printf("Found %d devices\n", deviceCount);
    
    // Initialize and scan a custom I2C bus
    deviceCount = Utils::I2CScanner::initAndScan(SDA_PIN, SCL_PIN, 400000);
    
    // Check for a specific device
    if (Utils::I2CScanner::devicePresent(0x68)) {
        Serial.println("MPU6050 found!");
        
        // Get device identification
        String deviceType = Utils::I2CScanner::identifyDevice(0x68);
        Serial.printf("Device identified as: %s\n", deviceType.c_str());
        
        // Test connection quality
        Utils::I2CScanner::testDeviceConnection(0x68);
    }
    
    // Run advanced scan with more detailed information
    Utils::I2CScanner::advancedScan();
}
```

### Implementation Details

The I2CScanner is designed as a utility class with all static methods, requiring no instantiation. It provides a simple interface for scanning I2C buses and detecting devices, useful during development and debugging phases.

The scan method systematically checks each address on the bus for a response, reporting any detected devices and building a map of the I2C bus topology.

### Advanced Features

#### Device Identification

The `identifyDevice` method attempts to recognize common I2C devices by their address and signature registers. It works by querying identification registers specific to each device type and comparing the results against a database of known device signatures.

```cpp
// Identify a device by its I2C address and signature registers
static String identifyDevice(uint8_t address, TwoWire& wire = Wire);
```

This function can identify many common devices including:
- MPU6050/9250 (0x68-0x69): Accelerometer and gyroscope sensors
- SSD1306/1309 (0x3C-0x3D): OLED display controllers
- BMP280/BME280 (0x76-0x77): Pressure and environmental sensors
- ADS1115/1015 (0x48-0x4B): Analog-to-digital converters
- PCA9685 (0x40): PWM/Servo controller
- DS3231/DS1307 (0x68): Real-time clock modules
- VL53L0X (0x29): Time-of-flight distance sensor
- APDS9960 (0x39): Gesture, proximity, RGB, and ambient light sensor
- MCP23017 (0x20-0x27): 16-bit I/O port expander
- And many more

The identification algorithm:
1. First checks if the device responds to the given address
2. Attempts to read from device-specific identification registers
3. Compares results with known signatures in the internal database
4. Returns the device name if a match is found, otherwise "Unknown device"

The device database can be extended by the application for custom or specialized devices.

Example usage:
```cpp
if (Utils::I2CScanner::devicePresent(0x76)) {
    String deviceName = Utils::I2CScanner::identifyDevice(0x76);
    Serial.printf("Device at 0x76: %s\n", deviceName.c_str());
}
```

#### Advanced Bus Scanning

The advanced scanner performs a comprehensive scan with detailed device information and connection quality metrics:

```cpp
// Advanced scan with device identification and connection quality metrics
static void advancedScan(TwoWire& wire = Wire);
```

This method:
1. Scans the entire I2C address space (1-127)
2. Measures response times for each device
3. Attempts to identify each device found
4. Tests connection quality and reliability
5. Outputs a formatted table with comprehensive information

The tabular output includes:
- I2C address (hex)
- Response time (microseconds)
- Identified device type
- Connection quality indicator
- Recommended actions (if issues detected)

Example output:
```
I2C Bus Analysis Report:
Address | Response (µs) | Device            | Status          | Notes
---------------------------------------------------------------------
0x3C    | 132          | SSD1306 OLED      | Good            | -
0x68    | 245          | MPU6050 IMU       | Good            | -
0x76    | 1853         | BME280 Sensor     | Warning (slow)  | Consider lower frequency
0x48    | 56           | ADS1115 ADC       | Good            | -
0x50    | 789          | AT24C32 EEPROM    | Marginal        | Check pull-ups
0x57    | ERROR        | Unknown           | Not responding  | Check wiring
```

Connection quality is evaluated based on:
- Response time (latency)
- Consistency of responses in repeated queries
- Success rate for basic register operations
- Stability across multiple transactions

#### Non-blocking Scanning

The non-blocking scanning feature allows I2C bus diagnostics to be performed during runtime without interrupting the main application flow. This is especially useful for:
- Runtime device hot-plug detection
- Background health monitoring
- Dynamic device discovery
- Safety monitoring during critical operations

```cpp
// Begin a non-blocking scan process in the background
static void beginAsyncScan(TwoWire& wire = Wire);

// Check if async scan is complete
static bool isAsyncScanComplete();

// Get scan results when complete (vector of found addresses)
static std::vector<uint8_t> getAsyncScanResults();
```

Example usage:
```cpp
// Start a scan in the background
Utils::I2CScanner::beginAsyncScan();

// Continue with other operations...
doSomethingElse();

// Later, in your loop:
if (Utils::I2CScanner::isAsyncScanComplete()) {
    auto devices = Utils::I2CScanner::getAsyncScanResults();
    Serial.printf("Found %d devices\n", devices.size());
    
    // Process each found device
    for (auto address : devices) {
        String deviceName = Utils::I2CScanner::identifyDevice(address);
        Serial.printf("Device at 0x%02X: %s\n", address, deviceName.c_str());
    }
}
```

Implementation notes:
- The asynchronous scan runs in small increments to avoid blocking
- It uses a static state machine to track progress
- Multiple calls to `beginAsyncScan()` while a scan is in progress will be ignored
- For ESP32, the scan leverages the multi-core architecture for true parallelism
- For single-core devices, the scan divides work into small chunks across multiple loop cycles

#### Connection Testing and Diagnostics

The I2CScanner provides advanced tools for testing and diagnosing I2C connection issues:

```cpp
// Test the quality of connection to a specific device
static bool testDeviceConnection(uint8_t address, TwoWire& wire = Wire, bool printOutput = true);

// Provide comprehensive diagnostics for connection issues
static void diagnoseConnectionIssues(uint8_t address, TwoWire& wire = Wire);
```

The connection testing functionality:
- Measures response time (latency)
- Performs repeated transactions to test reliability
- Executes standard and stressed I2C operations
- Evaluates connection quality
- Provides specific recommendations for improvement

Example usage:
```cpp
// Test connection to an MPU6050 at address 0x68
if (Utils::I2CScanner::devicePresent(0x68)) {
    // Basic connection test
    bool connectionOk = Utils::I2CScanner::testDeviceConnection(0x68);
    
    if (!connectionOk) {
        // If issues detected, run comprehensive diagnostics
        Utils::I2CScanner::diagnoseConnectionIssues(0x68);
    }
}
```

Example output from connection test:
```
Testing connection to device at 0x68:
- Device responds: Yes
- Response time: 245µs (Good)
- Read test: Success
- Write test: Success
- Stress test: 99/100 successful transactions (Good)
- Connection quality: Good
```

Example output from diagnostic function:
```
I2C Connection Diagnostics for device at 0x68:
- Basic connectivity test: PASSED
- Device identified as: MPU6050/MPU9250
- Response time: 954µs (Warning: higher than expected)
- Register read test: PASSED
- Register write test: PASSED
- Stability test: 82/100 successful (Warning: unstable connection)

Potential issues:
- Long or noisy I2C lines
- Weak pull-up resistors
- Bus capacitance too high
- Power supply instability

Recommendations:
- Reduce I2C bus frequency (try 100kHz instead of 400kHz)
- Check power supply quality at the device
- Verify pull-up resistor values (recommended: 2.2kΩ to 4.7kΩ)
- Shorten I2C lines or use shielded cables
- Add bypass capacitor (0.1µF) near the device
```

The diagnostics function performs deep analysis of the I2C connection by:
1. Testing basic connectivity
2. Identifying the specific device type
3. Evaluating response time and latency
4. Testing register read/write operations
5. Performing a stress test with multiple transactions
6. Analyzing error patterns to identify specific issues
7. Providing targeted recommendations based on the observed symptoms

---

## FileManager

The `FileManager` class provides an interface to the ESP32's SPIFFS file system with comprehensive file and directory operations.

### Features

- File operations (read, write, append, delete)
- Directory operations (create, remove, list)
- File information (existence, size)
- File listing with metadata

### API Reference

```cpp
namespace Utils {

class FileManager {
public:
    struct FileInfo {
        String name;
        size_t size;
        bool isDirectory;
    };
    
    FileManager();
    ~FileManager();
    
    bool init();
    String readFile(const String& path);
    bool writeFile(const String& path, const String& content);
    bool appendFile(const String& path, const String& content);
    bool deleteFile(const String& path);
    bool exists(const String& path);
    int getSize(const String& path);
    std::vector<FileInfo> listFiles(const String& path = "/");
    bool createDir(const String& path);
    bool removeDir(const String& path);

private:
    bool _initialized;
};
}
```

### Example Usage

```cpp
#include "Utils/FileManager.h"

void setup() {
    Utils::FileManager fileManager;
    
    // Initialize the file system
    if (fileManager.init()) {
        // Write content to a file
        fileManager.writeFile("/config.txt", "setting=value");
        
        // Read file content
        String content = fileManager.readFile("/config.txt");
        Serial.println("File content: " + content);
        
        // List files in the root directory
        auto files = fileManager.listFiles("/");
        for (const auto& file : files) {
            Serial.printf("%s (%d bytes)\n", file.name.c_str(), file.size);
        }
    }
}
```

### Implementation Details

The FileManager provides a wrapper around the ESP32's SPIFFS file system API, simplifying common operations and adding error handling and validation. The class maintains an initialization flag to prevent operations on an uninitialized file system.

---

## SpiAllocator

The `SpiAllocator` class provides a custom memory allocator for ArduinoJson that uses external SPI RAM instead of internal memory.

### Features

- Integration with ArduinoJson library
- External SPI RAM usage for large JSON documents
- Memory efficiency for resource-constrained devices
- Transparent fallback to internal RAM when needed

### API Reference

```cpp
namespace Utils {

struct SpiRamAllocator : ArduinoJson::Allocator {
    void* allocate(size_t size) override;
    void deallocate(void* pointer) override;
};

class SpiJsonDocument : public ArduinoJson::JsonDocument {
public:
    explicit SpiJsonDocument(size_t capa = 1024);
};
}
```

### Example Usage

```cpp
#include "Utils/SpiAllocator.h"
#include <ArduinoJson.h>

void createJsonDocument() {
    // Create a JSON document in external SPI RAM (4KB capacity)
    Utils::SpiJsonDocument doc;
    
    // Use like a regular JsonDocument
    doc["name"] = "Cozmo";
    doc["sensors"]["camera"] = "OV2640";
    doc["motors"] = true;
    
    // Serialize to a string
    String jsonStr;
    serializeJson(doc, jsonStr);
    Serial.println(jsonStr);
}
```

### Implementation Details

The SpiAllocator leverages the ESP32's PSRAM (Pseudo Static RAM) when available, which provides much more memory than the internal RAM but at a slightly slower access speed. This makes it ideal for larger data structures like JSON documents that might otherwise cause out-of-memory errors.

The implementation includes memory allocation tracking and fall-back mechanisms to internal RAM if PSRAM allocation fails or is unavailable.

---

## Base64

The `Base64` class provides utilities for encoding and decoding data using the Base64 algorithm, useful for binary data transmission in text-based protocols.

### Features

- Encoding binary data to Base64 strings
- Decoding Base64 strings to binary data
- Length calculation for memory allocation

### API Reference

```cpp
class Base64 {
public:
    // Encode binary data to base64 string
    static size_t encode(uint8_t* output, const uint8_t* input, size_t inputLength);
    
    // Calculate length of base64 encoded string
    static size_t encodedLength(size_t inputLength);
    
    // Decode base64 string to binary data
    static size_t decode(char* output, const char* input, size_t inputLength);
    
    // Calculate length of decoded binary data
    static size_t decodedLength(const char* input, size_t inputLength);
};
```

### Example Usage

```cpp
#include "Utils/Base64.h"

void encodeData() {
    // Binary data to encode
    const uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    
    // Calculate required buffer size
    size_t encodedLen = Base64::encodedLength(sizeof(data));
    
    // Allocate output buffer
    uint8_t* encoded = new uint8_t[encodedLen];
    
    // Encode the data
    Base64::encode(encoded, data, sizeof(data));
    
    // Use the encoded string
    Serial.println((char*)encoded);
    
    // Clean up
    delete[] encoded;
}
```

### Implementation Details

The Base64 implementation follows the standard encoding scheme where 3 bytes of binary data are converted into 4 ASCII characters. It handles padding appropriately when the input length is not a multiple of 3.

---

## HealthCheck

The `HealthCheck` class provides a system for monitoring the health of various components and services.

### API Reference

```cpp
namespace Utils {

enum class Status {
    UNKNOWN,
    HEALTHY,
    WARNING,
    ERROR,
    CRITICAL
};

struct Check {
    String name;
    Status status;
    std::function<Status()> checkFunction;
    String message;
    unsigned long lastCheckTime;
};

class HealthCheck {
public:
    HealthCheck();
    ~HealthCheck();
    
    bool init(unsigned long checkIntervalMs = 10000);
    bool addCheck(const String& name, std::function<Status()> checkFunction);
    void runChecks();
    Status getOverallStatus() const;
    const std::vector<Check>& getChecks() const;
    void setStatusChangeCallback(std::function<void(const String&, Status, Status)> callback);
    void update();

private:
    // Private member variables
};
}
```

### Example Usage

```cpp
#include "Utils/HealthCheck.h"

Utils::HealthCheck healthCheck;

// Define a check function
Utils::Status checkWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return Utils::Status::HEALTHY;
    } else {
        return Utils::Status::ERROR;
    }
}

void setup() {
    // Initialize health check system
    healthCheck.init(5000);  // Check every 5 seconds
    
    // Add checks
    healthCheck.addCheck("WiFi", checkWiFi);
    
    // Set callback for status changes
    healthCheck.setStatusChangeCallback([](const String& name, Utils::Status oldStatus, Utils::Status newStatus) {
        Serial.printf("Health check '%s' changed from %d to %d\n", name.c_str(), (int)oldStatus, (int)newStatus);
    });
}

void loop() {
    // Update health checks
    healthCheck.update();
    
    // Do other stuff
    delay(100);
}
```

---

## Sstring

The `Sstring` class provides a string implementation that uses external SPI RAM to store string data, freeing up internal memory.

### Features

- ESP32 PSRAM utilization for string storage
- Similar interface to Arduino's String class
- Automatic memory management

### Example Usage

```cpp
#include "Utils/Sstring.h"

void useExternalRamStrings() {
    // Create strings in various ways
    Utils::Sstring str1;                 // Empty string
    Utils::Sstring str2("Hello World");  // From C-string
    
    // String operations
    str1 = str2 + " Robot";     // Concatenation
    str1 += "!";                // Append
    
    // Use the string
    Serial.println(str1.c_str());
}
```

### Implementation Details

The Sstring class allocates memory from the ESP32's external PSRAM (Pseudo Static RAM) when available, which provides much more space than the limited internal RAM. This makes it suitable for applications that need to store large amounts of text data.
