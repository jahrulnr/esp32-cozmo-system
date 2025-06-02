#pragma once

#include "SpiAllocator.h"

namespace Utils {
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
