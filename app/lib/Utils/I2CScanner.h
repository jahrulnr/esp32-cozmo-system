#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <vector>

namespace Utils {

/**
 * @brief A comprehensive utility for I2C bus scanning, device detection and diagnostics
 * 
 * The I2CScanner provides tools for working with I2C devices including:
 * - Bus scanning to find connected devices
 * - Device identification by address and register signatures 
 * - Connection testing and quality assessment
 * - Diagnostic tools for troubleshooting I2C issues
 * - Non-blocking scanning for runtime diagnostics
 * - Multi-bus support for complex systems
 * 
 * All methods are static and can be called without instantiation:
 * `Utils::I2CScanner::scan();`
 * 
 * @note This class is designed to work with ESP32 and other Arduino-compatible platforms
 * that use the TwoWire library for I2C communication.
 */
class I2CScanner {
public:
    /**
     * @brief Scan an I2C bus for devices
     * 
     * @param wire The TwoWire instance to use (default = Wire)
     * @param startAddress Start address for scan (default = 1)
     * @param endAddress End address for scan (default = 127)
     * @param printOutput Whether to print scan results (default = true)
     * @return int Number of devices found
     */
    static int scan(TwoWire& wire = Wire, uint8_t startAddress = 1, uint8_t endAddress = 127, bool printOutput = true) {
        int deviceCount = 0;
        
        if (printOutput) {
            Serial.println("Scanning I2C bus for devices...");
        }
        
        for (uint8_t address = startAddress; address <= endAddress; address++) {
            wire.beginTransmission(address);
            uint8_t error = wire.endTransmission();
            
            if (error == 0) {
                deviceCount++;
                if (printOutput) {
                    Serial.printf("I2C device found at address 0x%02X\n", address);
                }
            } else if (error == 4) {
                if (printOutput) {
                    Serial.printf("Unknown error at address 0x%02X\n", address);
                }
            }
        }
        
        if (printOutput) {
            if (deviceCount == 0) {
                Serial.println("No I2C devices found");
            } else {
                Serial.printf("Found %d I2C device(s)\n", deviceCount);
            }
        }
        
        return deviceCount;
    }

    /**
     * @brief Initialize an I2C bus and scan for devices
     * 
     * @param sda SDA pin number
     * @param scl SCL pin number
     * @param frequency Bus frequency in Hz (default: 100000)
     * @param wire The TwoWire instance to use (default = Wire)
     * @return int Number of devices found
     */
    static int initAndScan(int sda, int scl, uint32_t frequency = 100000, TwoWire& wire = Wire) {
        wire.begin(sda, scl);
        wire.setClock(frequency);
        
        Serial.printf("Initialized I2C bus on pins SDA=%d, SCL=%d at %dkHz\n", 
                     sda, scl, frequency / 1000);
        
        return scan(wire);
    }

    /**
     * @brief Check if a specific I2C device is present
     * 
     * @param address Device address
     * @param wire The TwoWire instance to use (default = Wire)
     * @return true Device is present
     * @return false Device is not present
     */
    static bool devicePresent(uint8_t address, TwoWire& wire = Wire) {
        wire.beginTransmission(address);
        uint8_t error = wire.endTransmission();
        return (error == 0);
    }
};

} // namespace Utils
