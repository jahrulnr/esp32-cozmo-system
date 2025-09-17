#include "../register.h"

/**
 * Battery monitoring callback - handles battery state changes
 * This callback is triggered when battery critical/low events are sent to the display
 * Note: The sensor task now provides long-term averaged battery readings for stability
 */
void batteryCallback(void* arg) {
    if (!batteryManager) return;

    BatteryState state = batteryManager->getState();
    float voltage = batteryManager->getVoltage();
    int level = batteryManager->getLevel();
    static int predict = 0;
    static size_t lastCheck = millis();

    // Log battery state changes
    logger->info("Battery callback: %.3fV (%d%%) - State: %s",
        voltage, level,
        state == BATTERY_STATE_CRITICAL ? "CRITICAL" :
        state == BATTERY_STATE_LOW ? "LOW" :
        state == BATTERY_STATE_MEDIUM ? "MEDIUM" :
        state == BATTERY_STATE_HIGH ? "HIGH" :
        state == BATTERY_STATE_FULL ? "FULL" : "UNKNOWN");

    // Additional battery-specific actions can be added here
    // Note: The sensor task now handles long-term averaging (10 seconds)
    // to provide stable readings despite power fluctuations from motors,
    // screen, WiFi, and other modules
}
