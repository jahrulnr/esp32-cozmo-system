#include "IOExtern.h"
#include "Logger.h"

namespace Utils {

bool IOExtern::begin(const char* busName, uint8_t address, uint8_t sda, uint8_t scl) {
    _busName = busName;
    _address = address;
    
    // Check if device is present
    bool connected = isConnected();
    if (!connected) {
        Logger::getInstance().error("IOExtern: Device not found at address 0x%02X on bus %s", _address, _busName);
    } else {
        Logger::getInstance().info("IOExtern: Device initialized at address 0x%02X on bus %s", _address, _busName);
        
        io = new PCF8575(I2CManager::getInstance().getBus(_busName), _address);

        // Initialize all pins as inputs (high)
        if (!io) {
            Logger::getInstance().error("IOExtern: Failed to initialize device state");
            return false;
        }
    }
    
    return connected;
}

bool IOExtern::digitalWrite(uint8_t pin, uint8_t state) {
    if (pin > 15) {
        Logger::getInstance().error("IOExtern: Invalid pin number: %d (valid range is 0-15)", pin);
        return false;
    }

    if (!io){
        Logger::getInstance().error("IOExtern: failed to write %d", pin);
        return false;
    }

    pinMode[pin] = true;
    io->pinMode(pin, OUTPUT, 0);
    
    // Write new state
    return io->digitalWrite(pin, state);
}

int IOExtern::digitalRead(uint8_t pin) {
    if (pin > 15) {
        Logger::getInstance().error("IOExtern: Invalid pin number: %d (valid range is 0-15)", pin);
        return -1;
    }

    if (!io){
        Logger::getInstance().error("IOExtern: failed to read %d", pin);
        return false;
    }

    pinMode[pin] = false;
    io->pinMode(pin, INPUT);
    
    return io->digitalRead(pin);
}

bool IOExtern::isConnected() {
    return I2CManager::getInstance().devicePresent(_busName, _address);
}

} // namespace Utils
