# Utility Components for Cozmo-System

This directory contains utility components for the Cozmo-System project including:

1. **SpiAllocator** - A custom allocator for ArduinoJson that uses external SPI RAM
2. **I2CScanner** - A simple utility for scanning I2C buses
3. **I2CManager** - A comprehensive I2C bus management system
4. **Sstring** - A string class that uses external SPI RAM

## SpiAllocator

This utility allows you to use external SPI RAM for ArduinoJson documents, which is especially useful for large JSON documents. Compatible with ArduinoJson 7.x.

### Usage

```cpp
#include "Utils/SpiAllocator.h"
#include <ArduinoJson.h>

// Create a JSON document in external SPI RAM (4KB capacity)
Utils::SpiJsonDocument<4096> doc;

// Use like a regular JsonDocument
doc["name"] = "Cozmo";
doc["sensors"]["camera"] = "OV2640";

// Serialize to a string
String jsonStr;
serializeJson(doc, jsonStr);

// Access the memory usage
size_t usedBytes = doc.memoryUsage();
```

## I2CScanner

A simple utility for scanning I2C buses and detecting connected devices.

### Usage

```cpp
#include "Utils/I2CScanner.h"

// Scan the default I2C bus
int deviceCount = Utils::I2CScanner::scan();

// Initialize and scan a custom I2C bus
deviceCount = Utils::I2CScanner::initAndScan(SDA_PIN, SCL_PIN);

// Check for a specific device
bool devicePresent = Utils::I2CScanner::devicePresent(0x68);
```

## I2CManager

A comprehensive I2C bus management system that supports multiple buses, mutex protection, and common operations.

### Usage

```cpp
#include "Utils/I2CManager.h"

// Get the singleton instance
Utils::I2CManager& i2cManager = Utils::I2CManager::getInstance();

// Initialize a bus
i2cManager.initBus("main", SDA_PIN, SCL_PIN);

// Scan for devices
i2cManager.scanBus("main");

// Read from a device register
uint8_t value;
i2cManager.readRegister("main", DEVICE_ADDR, REGISTER_ADDR, value);
```

## Sstring

A string class that uses external SPI RAM, providing functionality similar to Arduino's String class but with the benefit of using external memory.

### Usage

```cpp
#include "Utils/Sstring.h"

// Create strings in various ways
Utils::Sstring str1;                 // Empty string
Utils::Sstring str2("Hello World");  // From C-string
Utils::Sstring str3(42);             // From integer
Utils::Sstring str4 = str2;          // Copy constructor

// String operations
str1 = str2 + " " + str3;     // Concatenation
str1 += "!";                  // Append
bool contains = str1.contains("World");  // Search
Utils::Sstring sub = str1.substring(6, 5);  // Substring (World)
```

## Examples

To run the examples:

1. Edit `app/config.h` and set the desired example flags to true:
   ```cpp
   #define ENABLE_SPI_JSON_EXAMPLE true
   #define ENABLE_I2C_SCANNER_EXAMPLE true
   #define ENABLE_SSTRING_EXAMPLE true
   ```

2. Compile and upload the project
3. Open the serial monitor to see the example output

Each example demonstrates the core functionality of the respective utility component.
