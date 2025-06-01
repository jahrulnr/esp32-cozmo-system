#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <map>
#include <string>

namespace Utils {

/**
 * @brief A centralized I2C bus management system
 * 
 * This class provides methods for managing multiple I2C buses,
 * synchronizing access with mutexes, and handling common I2C operations.
 */
class I2CManager {
public:
    /**
     * @brief Get the singleton instance of I2CManager
     * @return Reference to the I2CManager instance
     */
    static I2CManager& getInstance();

    /**
     * @brief Initialize an I2C bus
     * 
     * @param busName Unique name to identify this I2C bus
     * @param sda SDA pin number
     * @param scl SCL pin number
     * @param frequency Bus frequency in Hz (default: 100000)
     * @param useWire Use the default Wire object instead of creating a new one (default: true)
     * @return true if initialization was successful
     */
    bool initBus(const char* busName, int sda, int scl, uint32_t frequency = 100000, bool useWire = true);

    /**
     * @brief Get a pointer to a TwoWire instance for a specific bus
     * 
     * @param busName Name of the I2C bus
     * @return TwoWire* Pointer to the bus, or nullptr if not found
     */
    TwoWire* getBus(const char* busName);

    /**
     * @brief Check if a device is present on a bus
     * 
     * @param busName Name of the I2C bus
     * @param address Device address
     * @return true if device is detected
     */
    bool devicePresent(const char* busName, byte address);

    /**
     * @brief Write a byte to a device register
     * 
     * @param busName Name of the I2C bus
     * @param deviceAddress Device address
     * @param registerAddress Register address
     * @param data Data byte to write
     * @return true if write was successful
     */
    bool writeRegister(const char* busName, byte deviceAddress, uint8_t registerAddress, uint8_t data);

    /**
     * @brief Read a byte from a device register
     * 
     * @param busName Name of the I2C bus
     * @param deviceAddress Device address
     * @param registerAddress Register address
     * @param result Reference to store the result
     * @return true if read was successful
     */
    bool readRegister(const char* busName, byte deviceAddress, uint8_t registerAddress, uint8_t &result);

    /**
     * @brief Read multiple bytes from a device register
     * 
     * @param busName Name of the I2C bus
     * @param deviceAddress Device address
     * @param registerAddress Register address
     * @param buffer Buffer to store the results
     * @param length Number of bytes to read
     * @return true if read was successful
     */
    bool readRegisters(const char* busName, byte deviceAddress, uint8_t registerAddress, 
                      uint8_t *buffer, uint8_t length);

    /**
     * @brief Scan the bus for I2C devices and log their addresses
     * 
     * @param busName Name of the I2C bus
     */
    void scanBus(const char* busName);

private:
    // Private constructor for singleton pattern
    I2CManager() {}

    // Prevent copying
    I2CManager(const I2CManager&) = delete;
    I2CManager& operator=(const I2CManager&) = delete;

    // Structure to hold bus information
    struct BusInfo {
        TwoWire* wire;
        SemaphoreHandle_t mutex;
        int sdaPin;
        int sclPin;
        uint32_t frequency;
        bool isDefault;  // Using default Wire object
    };

    // Map of bus names to bus info
    std::map<std::string, BusInfo> _buses;

    // Helper to get bus info and take mutex if available
    BusInfo* takeBus(const char* busName, uint32_t timeoutMs = 100);
    
    // Helper to release a bus mutex
    void releaseBus(BusInfo* bus);
};

} // namespace Utils
