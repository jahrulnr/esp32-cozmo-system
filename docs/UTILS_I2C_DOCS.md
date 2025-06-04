# Utils & I2C Components Documentation

## Overview

The Utils directory contains core utility components for the Cozmo-System project that provide essential functionality across the system. These utilities include logging, I2C management, file management, and memory optimization tools.

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

The `I2CScanner` class provides utilities for detecting I2C devices on a bus and diagnosing connection issues.

### Features

- Static methods for easy access without instantiation
- Comprehensive bus scanning with address reporting
- Device presence checking
- Custom I2C bus initialization and scanning

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
    }
}
```

### Implementation Details

The I2CScanner is designed as a utility class with all static methods, requiring no instantiation. It provides a simple interface for scanning I2C buses and detecting devices, useful during development and debugging phases.

The scan method systematically checks each address on the bus for a response, reporting any detected devices and building a map of the I2C bus topology.

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
