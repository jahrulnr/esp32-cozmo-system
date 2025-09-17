#include "setup/setup.h"

Utils::IOExtern oExpander;
Utils::IOExtern iExpander;

// PCF8575 driver
void setupExtender() {
    // Initialize I2C bus first (if not already initialized elsewhere)
    Utils::I2CManager::getInstance().initBus("base", SCREEN_SDA_PIN, SCREEN_SCL_PIN);

    // Initialize PCF8575 with default address (0x20)
    if (oExpander.begin("base", 0x20, SCREEN_SDA_PIN, SCREEN_SCL_PIN)) {
        Utils::Logger::getInstance().info("Output extender initialized successfully");
    } else {
        Utils::Logger::getInstance().error("Output initialization failed");
    }

    delay(10);

    // Initialize PCF8575 with address (0x26)
    if (iExpander.begin("base", 0x26, SCREEN_SDA_PIN, SCREEN_SCL_PIN)) {
        iExpander.setMaxPin(8);
        Utils::Logger::getInstance().info("Input extender initialized successfully");
    } else {
        Utils::Logger::getInstance().error("Input initialization failed");
    }
}