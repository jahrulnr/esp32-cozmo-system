#pragma once

#include <Arduino.h>
#include <PCF8575.h>
#include "I2CManager.h"

namespace Utils {

/**
 * @brief Driver for IOExtern 16-bit I/O expander
 * 
 * The IOExtern is a 16-bit I/O expander with I2C interface.
 * It provides 16 general purpose I/O pins that can be individually
 * configured as inputs or outputs through software.
 */
class IOExtern {
public:
    /**
     * @brief Initialize IOExtern device
     * 
     * @param busName Name of the I2C bus
     * @param address I2C address of IOExtern (default: 0x20)
     * @return true if initialization was successful
     */
    bool begin(const char* busName, uint8_t address = 0x20, uint8_t sda = -1, uint8_t scl = -1);

    void setMaxPin(int maxpin = 16);
    
    /**
     * @brief Write a specific pin's state
     * 
     * @param pin Pin number (0-15)
     * @param state Pin state (HIGH/LOW)
     * @return true if write was successful
     */
    bool digitalWrite(int pin, int state);
    
    /**
     * @brief Read a specific pin's state
     * 
     * @param pin Pin number (0-15)
     * @return Pin state (HIGH/LOW) or -1 if error
     */
    int digitalRead(int pin, bool force = false);
    
    /**
     * @brief Check if IOExtern device is connected
     * 
     * @return true if device is detected
     */
    bool isConnected();

private:
    typedef enum {
        AS_OUTPUT = 0,
        AS_INPUT,
        AS_INPUT_PULLUP,
        NONE
    } _pinType;

    const char* _busName;       // I2C bus name
    uint8_t _address;           // Device address
    _pinType pinMode[16];
    int _maxPin = 16;
    PCF8575* io;
};

} // namespace Utils
