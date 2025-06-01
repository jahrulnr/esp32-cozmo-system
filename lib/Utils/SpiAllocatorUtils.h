#pragma once

#include "SpiAllocator.h"

namespace Utils {

/**
 * @brief Advanced utilities for SPI RAM allocation with ArduinoJson
 * 
 * This file provides additional utilities for working with JSON in SPI RAM
 * including filter policies, converters, and specialized JSON document types.
 */

/**
 * @brief Creates a SpiJsonDocument and populates it from a string
 * 
 * @tparam capacity The capacity of the document in bytes
 * @param input The input string containing JSON data
 * @return SpiJsonDocument<capacity> The populated document
 */
template <size_t capacity>
SpiJsonDocument<capacity> deserializeToSpiRam(const char* input) {
    SpiJsonDocument<capacity> doc;
    DeserializationError error = deserializeJson(doc, input);
    
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
    }
    
    return doc;
}

/**
 * @brief Creates a SpiJsonDocument and populates it from a Stream
 * 
 * @tparam capacity The capacity of the document in bytes
 * @param input The input stream containing JSON data
 * @return SpiJsonDocument<capacity> The populated document
 */
template <size_t capacity>
SpiJsonDocument<capacity> deserializeToSpiRam(Stream& input) {
    SpiJsonDocument<capacity> doc;
    DeserializationError error = deserializeJson(doc, input);
    
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
    }
    
    return doc;
}

/**
 * @brief Utility function to measure the required JSON capacity
 * 
 * Call this function when you need to determine how much memory your JSON will need.
 * Note: It uses a DynamicJsonDocument temporarily, so use it for testing, not in production.
 * 
 * @param jsonString The JSON string to measure
 * @return size_t The required capacity in bytes
 */
inline size_t measureJsonMemoryNeeded(const char* jsonString) {
    // Start with a small document to parse the structure
    ArduinoJson::DynamicJsonDocument tempDoc(1024);
    DeserializationError error = deserializeJson(tempDoc, jsonString);
    
    if (error) {
        Serial.print("JSON measurement failed: ");
        Serial.println(error.c_str());
        return 0;
    }
    
    // Get the memory usage and add a safety margin (20%)
    return tempDoc.memoryUsage() * 1.2;
}

/**
 * @brief Function to determine if the ESP32 has PSRAM available
 * 
 * @return true if PSRAM is available and enabled
 * @return false if PSRAM is not available
 */
inline bool isPSRAMAvailable() {
    return ESP.getPsramSize() > 0;
}

/**
 * @brief Prints memory usage statistics to Serial
 */
inline void printMemoryStats() {
    Serial.println("Memory Stats:");
    Serial.printf("Total Heap: %u\n", ESP.getHeapSize());
    Serial.printf("Free Heap: %u\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %u\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %u\n", ESP.getFreePsram());
}

} // namespace Utils
