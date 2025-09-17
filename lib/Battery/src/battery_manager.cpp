#include "battery_manager.h"

#define BATTERY_CRITICAL   10    // Critical battery level
#define BATTERY_LOW        25    // Low battery level
#define BATTERY_MEDIUM     50    // Medium battery level
#define BATTERY_HIGH       75    // High battery level
#define BATTERY_SAMPLES    10    // Number of samples to average for stable reading

#define BATTERY_NOTIFY_CRITICAL true  // Notify when battery is critical
#define BATTERY_NOTIFY_LOW      true  // Notify when battery is low
BatteryManager::BatteryManager(): TAG("BatteryManager") {
    batteryPin = 1;               // Default ADC pin
    chargePin = -1;               // Default no charge detection
    voltageMax = 4.2;             // Li-ion maximum voltage
    voltageMin = 3.3;             // Li-ion minimum voltage
    voltageDivider = 2.0;         // Voltage divider ratio (R1=R2)
    adcResolution = 4095;         // 12-bit ADC
    updateInterval = 5000;        // 5 seconds default

    lastUpdate = 0;
    currentVoltage = 0;
    currentLevel = 0;
    currentState = BATTERY_STATE_CRITICAL;
    chargingState = CHARGING_UNKNOWN;

    notifyCritical = BATTERY_NOTIFY_CRITICAL;
    notifyLow = BATTERY_NOTIFY_LOW;
    wasLowNotified = false;
    wasCriticalNotified = false;
}

BatteryManager::~BatteryManager() {
    // Nothing to clean up
}

void BatteryManager::init(int pin){
    setPin(pin);
    setup();
}

void BatteryManager::setup() {
    ESP_LOGI(TAG, "BatteryManager: Initializing...");

    // Configure ADC
    analogReadResolution(12); // Set ADC resolution to 12 bits (0-4095)
    adcResolution = 4095; // Set the actual resolution value

    // Get initial readings
    update();

    ESP_LOGI(TAG, "BatteryManager: Initialization complete");
    printStatus();
}

void BatteryManager::setVoltage(float min, float max, float divider){
    setVoltageMin(min);
    setVoltageMax(max);
    setVoltageDivider(divider);
}

void BatteryManager::update() {
    unsigned long currentTime = millis();

    // Only update at the specified interval
    if ((currentTime - lastUpdate) >= updateInterval) {
        // Read and calculate battery voltage and level
        currentVoltage = readVoltage();
        currentLevel = calculateLevel(currentVoltage);
        BatteryState newState = determineState(currentLevel);

        // Check if state has changed
        if (newState != currentState) {
            currentState = newState;

            // Handle notifications for low and critical states
            if (currentState == BATTERY_STATE_CRITICAL && notifyCritical && !wasCriticalNotified) {
                // Critical battery notification
                ESP_LOGI(TAG, "BatteryManager: CRITICAL BATTERY LEVEL!");
                wasCriticalNotified = true;
            }
            else if (currentState == BATTERY_STATE_LOW && notifyLow && !wasLowNotified) {
                // Low battery notification
                ESP_LOGI(TAG, "BatteryManager: Low battery level");
                wasLowNotified = true;
            }

            // Reset notification flags if battery level improved
            if (currentState > BATTERY_STATE_LOW) {
                wasLowNotified = false;
            }
            if (currentState > BATTERY_STATE_CRITICAL) {
                wasCriticalNotified = false;
            }
        }

        // Update timestamp
        lastUpdate = currentTime;
    }
}

float BatteryManager::readVoltage() {
    // Take multiple samples to stabilize reading
    long sum = 0;
    for (int i = 0; i < BATTERY_SAMPLES; i++) {
        sum += analogRead(batteryPin);
        delay(2); // Small delay between readings
    }

    // Average the readings
    float rawValue = (float)sum / BATTERY_SAMPLES;

    // Convert ADC reading to voltage (considering voltage divider)
    // First calculate the voltage at the ADC pin: ADC value * (3.3V reference / resolution)
    float adcVoltage = rawValue * (3.3 / adcResolution);

    // Then calculate the actual battery voltage using the voltage divider formula
    // For two equal resistors (100k), the voltage is doubled from what the ADC reads
    float voltage = adcVoltage * voltageDivider;

    ESP_LOGI(TAG, "Raw ADC: %.0f, ADC Voltage: %.2fV, Battery Voltage: %.2fV, Level: %d%%",
                 rawValue, adcVoltage, voltage, calculateLevel(voltage));

    return voltage;
}

int BatteryManager::calculateLevel(float voltage) {
    // Calculate battery percentage based on voltage
    // Linear mapping from min voltage (0%) to max voltage (100%)
    if (voltage <= voltageMin) return 0;
    if (voltage >= voltageMax) return 100;

    // Linear interpolation
    int level = (int)(((voltage - voltageMin) / (voltageMax - voltageMin)) * 100.0);
    return constrain(level, 0, 100); // Ensure level is between 0-100
}

BatteryState BatteryManager::determineState(int level) {
    // Determine battery state based on percentage
    if (level <= BATTERY_CRITICAL) return BATTERY_STATE_CRITICAL;
    if (level <= BATTERY_LOW) return BATTERY_STATE_LOW;
    if (level <= BATTERY_MEDIUM) return BATTERY_STATE_MEDIUM;
    if (level <= BATTERY_HIGH) return BATTERY_STATE_HIGH;
    return BATTERY_STATE_FULL;
}

void BatteryManager::setUpdateInterval(unsigned long interval) {
    updateInterval = interval;
}

void BatteryManager::enableNotifications(bool critical, bool low) {
    notifyCritical = critical;
    notifyLow = low;
}

void BatteryManager::clearNotificationFlags() {
    wasLowNotified = false;
    wasCriticalNotified = false;
}

void BatteryManager::setChargingState(ChargingState state) {
    chargingState = state;
}

void BatteryManager::printStatus() const {
    ESP_LOGI(TAG, "======== Battery Status ========");
    ESP_LOGI(TAG, "Voltage: %.2fV", currentVoltage);
    ESP_LOGI(TAG, "Level: %d%%", currentLevel);

    // Print state
    const char* stateStr = "UNKNOWN";
    switch (currentState) {
        case BATTERY_STATE_CRITICAL: stateStr = "CRITICAL"; break;
        case BATTERY_STATE_LOW:      stateStr = "LOW"; break;
        case BATTERY_STATE_MEDIUM:   stateStr = "MEDIUM"; break;
        case BATTERY_STATE_HIGH:     stateStr = "HIGH"; break;
        case BATTERY_STATE_FULL:     stateStr = "FULL"; break;
        default:                     stateStr = "UNKNOWN"; break;
    }
    ESP_LOGI(TAG, "State: %s", stateStr);

    // Print charging state
    const char* chargingStr = "Unknown";
    switch (chargingState) {
        case CHARGING_NOT_CONNECTED: chargingStr = "Not connected"; break;
        case CHARGING_IN_PROGRESS:   chargingStr = "In progress"; break;
        case CHARGING_COMPLETE:      chargingStr = "Complete"; break;
        default:                     chargingStr = "Unknown"; break;
    }
    ESP_LOGI(TAG, "Charging: %s", chargingStr);

    // Print calibration info
    ESP_LOGI(TAG, "Voltage range: %.2fV - %.2fV", voltageMin, voltageMax);
    ESP_LOGI(TAG, "Voltage divider: %.2f", voltageDivider);
    ESP_LOGI(TAG, "==============================");
}
