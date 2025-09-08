#include "../setup.h"

BatteryManager* batteryManager = nullptr;

void setupBatteryManager() {
    if (!BATTERY_ENABLED) {
        logger->info("Battery monitoring disabled");
        return;
    }

    logger->info("Setting up battery manager...");
    
    batteryManager = new BatteryManager();
    if (!batteryManager) {
        logger->error("Failed to allocate memory for battery manager");
        return;
    }

    // Configure battery manager
    batteryManager->setPin(BATTERY_ADC_PIN, BATTERY_CHARGE_PIN);
    batteryManager->setVoltage(BATTERY_VOLTAGE_MIN, BATTERY_VOLTAGE_MAX, BATTERY_VOLTAGE_DIVIDER);
    batteryManager->setAdcResolution(BATTERY_ADC_RESOLUTION);
    batteryManager->setUpdateInterval(BATTERY_UPDATE_INTERVAL);
    batteryManager->enableNotifications(true, true); // Enable critical and low battery notifications

    // Initialize the battery manager
    batteryManager->init(BATTERY_ADC_PIN);
    
    logger->info("Battery manager setup complete");
}
