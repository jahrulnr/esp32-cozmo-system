#include "Examples.h"
#include "SpiAllocator.h"
#include "I2CScanner.h"
#include "I2CManager.h"
#include "Sstring.h"

namespace Utils {

void spiJsonExample() {
    Serial.println("SpiJsonDocument Example:");
    
    // Create a JSON document in external SPI RAM (capacity of 4KB)
    SpiJsonDocument doc;
    
    // Add various data types to the document
    doc["name"] = "Cozmo Robot";
    doc["version"] = 1.0;
    doc["active"] = true;
    
    // Create nested objects - updated for ArduinoJson 7.x
    JsonObject sensors = doc["sensors"].to<JsonObject>();
    sensors["gyro"] = "MPU6050";
    sensors["camera"] = "OV2640";
    sensors["cliff"] = "IR sensor";
    
    // Create nested arrays - updated for ArduinoJson 7.x
    JsonArray motors = doc["motors"].to<JsonArray>();
    motors.add("left_wheel");
    motors.add("right_wheel");
    motors.add("arm");
    
    // Print the JSON document
    Serial.println("JSON Document:");
    serializeJsonPretty(doc, Serial);
    Serial.println();
    
    // Access data from the document
    const char* name = doc["name"];
    float version = doc["version"];
    bool active = doc["active"];
    
    Serial.printf("Name: %s\n", name);
    Serial.printf("Version: %.1f\n", version);
    Serial.printf("Active: %s\n", active ? "true" : "false");
    
    // Access nested data
    const char* gyroSensor = doc["sensors"]["gyro"];
    Serial.printf("Gyro sensor: %s\n", gyroSensor);
    
    // Iterate through an array
    Serial.println("Motors:");
    JsonArray motorsArray = doc["motors"];
    for (JsonVariant motor : motorsArray) {
        Serial.printf("- %s\n", motor.as<const char*>());
    }
    
    // Size of the document (Note: memoryUsage is deprecated in ArduinoJson 7.x)
    size_t docSize = doc.size();
    Serial.printf("Document size (number of elements at root): %zu\n", docSize);
    
    Serial.println("End of SpiJsonDocument Example");
    Serial.println();
}

void i2cScannerExample(int sdaPin, int sclPin) {
    Serial.println("I2C Scanner Example:");
    
    // Initialize and scan I2C bus
    int deviceCount = I2CScanner::initAndScan(sdaPin, sclPin);
    
    // Example using I2CManager
    Serial.println("\nI2CManager Example:");
    I2CManager& manager = I2CManager::getInstance();
    
    // Initialize a bus with a custom name
    if (manager.initBus("main", sdaPin, sclPin)) {
        // Scan for devices
        manager.scanBus("main");
        
        // Example checking if a specific device is present (I2C address 0x68 - common for MPU6050)
        if (manager.devicePresent("main", 0x68)) {
            Serial.println("MPU6050 found at address 0x68");
            
            // Example reading from a register
            uint8_t whoAmI;
            if (manager.readRegister("main", 0x68, 0x75, whoAmI)) {
                Serial.printf("WHO_AM_I register value: 0x%02X\n", whoAmI);
            }
        }
    }
    
    Serial.println("End of I2C Scanner Example");
    Serial.println();
}

void sstringExample() {
    Serial.println("Sstring Example:");
    
    // Create strings in different ways
    Sstring str1;                     // Empty string
    Sstring str2("Hello, World!");    // From C-string
    Sstring str3(str2);               // Copy constructor
    Sstring str4 = "Cozmo Robot";     // Assignment from C-string
    Sstring str5 = String("ESP32");   // From Arduino String
    Sstring str6(42);                 // From integer
    Sstring str7(3.14159f, 2);        // From float with 2 decimal places
    
    // Print the strings
    Serial.printf("str1: '%s' (empty: %s)\n", str1.c_str(), str1.isEmpty() ? "yes" : "no");
    Serial.printf("str2: '%s'\n", str2.c_str());
    Serial.printf("str3: '%s'\n", str3.c_str());
    Serial.printf("str4: '%s'\n", str4.c_str());
    Serial.printf("str5: '%s'\n", str5.c_str());
    Serial.printf("str6: '%s'\n", str6.c_str());
    Serial.printf("str7: '%s'\n", str7.c_str());
    
    // String operations
    Sstring combined = str2 + " " + str4;
    Serial.printf("Combined: '%s'\n", combined.c_str());
    
    str4 += " with ESP32";
    Serial.printf("str4 (after append): '%s'\n", str4.c_str());
    
    // String methods
    Serial.printf("Length of str2: %zu\n", str2.length());
    
    if (str2.contains("World")) {
        Serial.println("str2 contains 'World'");
    }
    
    if (str4.startsWith("Cozmo")) {
        Serial.println("str4 starts with 'Cozmo'");
    }
    
    int position = str2.indexOf("World");
    Serial.printf("Position of 'World' in str2: %d\n", position);
    
    Sstring substr = str2.substring(7, 5);  // "World"
    Serial.printf("Substring of str2: '%s'\n", substr.c_str());
    
    Sstring withSpaces = "  Trim Example  ";
    Sstring trimmed = withSpaces.trim();
    Serial.printf("Before trim: '%s', After trim: '%s'\n", 
                 withSpaces.c_str(), trimmed.c_str());
    
    Sstring replaced = str2;
    replaced.replace(Sstring("World"), Sstring("ESP32"));
    Serial.printf("After replace: '%s'\n", replaced.c_str());
    
    // Numeric conversions
    Sstring numStr("42.5");
    int intValue = numStr.toInt();
    float floatValue = numStr.toFloat();
    Serial.printf("String '%s' to int: %d, to float: %.1f\n", 
                 numStr.c_str(), intValue, floatValue);
    
    Serial.println("End of Sstring Example");
}

} // namespace Utils
