#include "I2CManager.h"

namespace Utils {

// Static instance for singleton pattern
I2CManager& I2CManager::getInstance() {
    static I2CManager instance;
    return instance;
}

bool I2CManager::initBus(const char* busName, int sda, int scl, uint32_t frequency, bool useWire) {
    // Check if bus already exists
    if (_buses.find(busName) != _buses.end()) {
        return true;
    }

    BusInfo bus;
    bus.mutex = xSemaphoreCreateMutex();
    if (!bus.mutex) {
        Serial.printf("Failed to create mutex for I2C bus '%s'\n", busName);
        return false;
    }

    bus.sdaPin = sda;
    bus.sclPin = scl;
    bus.frequency = frequency;
    bus.isDefault = useWire;

    if (useWire) {
        bus.wire = &Wire;
    } else {
        #if defined(ESP32)
        bus.wire = new TwoWire(0);
        #else
        Serial.printf("Custom Wire instances not supported on this platform\n");
        vSemaphoreDelete(bus.mutex);
        return false;
        #endif
    }

    // Initialize I2C bus
    if (!bus.wire->begin(sda, scl)) {
        Serial.printf("Failed to initialize I2C bus '%s' on pins SDA=%d, SCL=%d\n", 
                     busName, sda, scl);
        if (!useWire) {
            delete bus.wire;
        }
        vSemaphoreDelete(bus.mutex);
        return false;
    }

    // Set bus frequency
    bus.wire->setClock(frequency);
    Serial.printf("Initialized I2C bus '%s' on pins SDA=%d, SCL=%d at %dkHz\n", 
                 busName, sda, scl, frequency / 1000);

    // Add to map
    _buses[busName] = bus;
    return true;
}

TwoWire* I2CManager::getBus(const char* busName) {
    auto it = _buses.find(busName);
    if (it != _buses.end()) {
        return it->second.wire;
    }
    Serial.printf("I2C bus '%s' not found\n", busName);
    return nullptr;
}

bool I2CManager::devicePresent(const char* busName, byte address) {
    BusInfo* bus = takeBus(busName);
    if (!bus) {
        return false;
    }

    bus->wire->beginTransmission(address);
    byte error = bus->wire->endTransmission();
    
    releaseBus(bus);
    return (error == 0);
}

bool I2CManager::writeRegister(const char* busName, byte deviceAddress, 
                              uint8_t registerAddress, uint8_t data) {
    BusInfo* bus = takeBus(busName);
    if (!bus) {
        return false;
    }

    bus->wire->beginTransmission(deviceAddress);
    
    if (bus->wire->write(registerAddress) != 1) {
        Serial.printf("Failed to write register address 0x%02X to device 0x%02X on bus '%s'\n", 
                     registerAddress, deviceAddress, busName);
        bus->wire->endTransmission();
        releaseBus(bus);
        return false;
    }
    
    if (bus->wire->write(data) != 1) {
        Serial.printf("Failed to write data 0x%02X to register 0x%02X on device 0x%02X on bus '%s'\n", 
                     data, registerAddress, deviceAddress, busName);
        bus->wire->endTransmission();
        releaseBus(bus);
        return false;
    }
    
    uint8_t error = bus->wire->endTransmission();
    releaseBus(bus);
    
    if (error != 0) {
        Serial.printf("I2C transmission error %d when writing to device 0x%02X on bus '%s'\n", 
                     error, deviceAddress, busName);
        return false;
    }
    
    return true;
}

bool I2CManager::readRegister(const char* busName, byte deviceAddress, 
                             uint8_t registerAddress, uint8_t &result) {
    BusInfo* bus = takeBus(busName);
    if (!bus) {
        return false;
    }

    bus->wire->beginTransmission(deviceAddress);
    
    if (bus->wire->write(registerAddress) != 1) {
        Serial.printf("Failed to write register address 0x%02X to device 0x%02X on bus '%s'\n",
                     registerAddress, deviceAddress, busName);
        bus->wire->endTransmission();
        releaseBus(bus);
        return false;
    }
    
    if (bus->wire->endTransmission(false) != 0) {
        Serial.printf("I2C transmission failed on bus '%s' when setting register\n", busName);
        releaseBus(bus);
        return false;
    }
    
    if (bus->wire->requestFrom(deviceAddress, (uint8_t)1) != 1) {
        Serial.printf("Failed to request data from device 0x%02X on bus '%s'\n", deviceAddress, busName);
        releaseBus(bus);
        return false;
    }
    
    if (bus->wire->available()) {
        result = bus->wire->read();
        releaseBus(bus);
        return true;
    }
    
    Serial.printf("No data available from device 0x%02X on bus '%s'\n", deviceAddress, busName);
    releaseBus(bus);
    return false;
}

bool I2CManager::readRegisters(const char* busName, byte deviceAddress, 
                              uint8_t registerAddress, uint8_t *buffer, uint8_t length) {
    if (!buffer) {
        Serial.printf("Buffer is NULL for readRegisters call on bus '%s'\n", busName);
        return false;
    }

    BusInfo* bus = takeBus(busName);
    if (!bus) {
        return false;
    }

    bus->wire->beginTransmission(deviceAddress);
    
    if (bus->wire->write(registerAddress) != 1) {
        Serial.printf("Failed to write register address 0x%02X to device 0x%02X on bus '%s'\n",
                     registerAddress, deviceAddress, busName);
        bus->wire->endTransmission();
        releaseBus(bus);
        return false;
    }
    
    if (bus->wire->endTransmission(false) != 0) {
        Serial.printf("I2C transmission failed on bus '%s' when setting register\n", busName);
        releaseBus(bus);
        return false;
    }
    
    uint8_t bytesReceived = bus->wire->requestFrom(deviceAddress, length);
    if (bytesReceived != length) {
        Serial.printf("Requested %d bytes, received %d from device 0x%02X on bus '%s'\n", 
                     length, bytesReceived, deviceAddress, busName);
    }
    
    for (uint8_t i = 0; i < bytesReceived && bus->wire->available(); i++) {
        buffer[i] = bus->wire->read();
    }
    
    releaseBus(bus);
    return bytesReceived > 0;
}

void I2CManager::scanBus(const char* busName) {
    BusInfo* bus = takeBus(busName, 1000);  // Longer timeout for scanning
    if (!bus) {
        return;
    }

    Serial.printf("Scanning I2C bus '%s' (SDA=%d, SCL=%d) for devices...\n", 
                 busName, bus->sdaPin, bus->sclPin);
    
    uint8_t deviceCount = 0;
    
    for (byte address = 1; address < 127; address++) {
        bus->wire->beginTransmission(address);
        byte error = bus->wire->endTransmission();
        
        if (error == 0) {
            Serial.printf("Device found at address 0x%02X on bus '%s'\n", address, busName);
            deviceCount++;
        } else if (error == 4) {
            Serial.printf("Unknown error at address 0x%02X on bus '%s'\n", address, busName);
        }
    }
    
    if (deviceCount == 0) {
        Serial.printf("No I2C devices found on bus '%s'\n", busName);
    } else {
        Serial.printf("Found %d I2C device(s) on bus '%s'\n", deviceCount, busName);
    }
    
    releaseBus(bus);
}

I2CManager::BusInfo* I2CManager::takeBus(const char* busName, uint32_t timeoutMs) {
    auto it = _buses.find(busName);
    if (it == _buses.end()) {
        Serial.printf("I2C bus '%s' not found\n", busName);
        return nullptr;
    }
    
    BusInfo* bus = &(it->second);
    
    if (bus->mutex) {
        if (xSemaphoreTake(bus->mutex, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) {
            Serial.printf("Failed to take mutex for I2C bus '%s', timeout occurred\n", busName);
            return nullptr;
        }
    }
    
    return bus;
}

void I2CManager::releaseBus(BusInfo* bus) {
    if (bus && bus->mutex) {
        xSemaphoreGive(bus->mutex);
    }
}

} // namespace Utils
