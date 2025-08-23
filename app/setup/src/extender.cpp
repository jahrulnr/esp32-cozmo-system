#include "setup/setup.h"

Utils::IOExtern ioExpander;

// PCF8575 driver
void setupExtender() {
    // Initialize I2C bus first (if not already initialized elsewhere)
    Utils::I2CManager::getInstance().initBus("base", SCREEN_SDA_PIN, SCREEN_SCL_PIN, 0);
    
    // Initialize PCF8575 with default address (0x20)
    if (ioExpander.begin("base", 0x20, SCREEN_SDA_PIN, SCREEN_SCL_PIN)) {
        Utils::Logger::getInstance().info("PCF8575 extender initialized successfully");
    } else {
        Utils::Logger::getInstance().error("PCF8575 initialization failed");
    }
}