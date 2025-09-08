#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

// Battery level states
enum BatteryState {
    BATTERY_STATE_CRITICAL,
    BATTERY_STATE_LOW,
    BATTERY_STATE_MEDIUM,
    BATTERY_STATE_HIGH,
    BATTERY_STATE_FULL
};

// Battery charging states
enum ChargingState {
    CHARGING_UNKNOWN,
    CHARGING_NOT_CONNECTED,
    CHARGING_IN_PROGRESS,
    CHARGING_COMPLETE
};

class BatteryManager {
private:
    const char* TAG;
    int batteryPin;                  // ADC pin for battery measurement
    int chargePin;                   //
    float voltageMax;                // Maximum voltage
    float voltageMin;                // Minimum voltage
    float voltageDivider;            // Voltage divider ratio
    float adcResolution;
    
    unsigned long lastUpdate;        // Last update timestamp
    unsigned long updateInterval;    // Update interval in ms
    
    float currentVoltage;            // Current measured voltage
    int currentLevel;                // Current battery percentage (0-100)
    BatteryState currentState;       // Current battery state
    ChargingState chargingState;     // Current charging state
    
    bool notifyCritical;             // Whether to notify on critical
    bool notifyLow;                  // Whether to notify on low
    bool wasLowNotified;             // Whether low notification was shown
    bool wasCriticalNotified;        // Whether critical notification was shown
    
    void setup();

    // Private methods
    float readVoltage();             // Read raw voltage from ADC
    int calculateLevel(float voltage); // Calculate percentage from voltage
    BatteryState determineState(int level); // Determine state from percentage
    
public:
    BatteryManager();
    ~BatteryManager();
    
    void init(int pin);
    void update();

    // Setters
    void setPin(int battery = 1, int charger = -1){
        batteryPin = battery;
        chargePin = charger;
    };
    void setVoltage(float min, float max, float divider);
    void setVoltageMax(float value) { voltageMax = value; }
    void setVoltageMin(float value) { voltageMin = value; }
    void setVoltageDivider(float value) { voltageDivider = value; }
    void setAdcResolution(float value) { adcResolution = value; };
    
    void setInterval(unsigned long value) { updateInterval = value; }
    
    // Getters
    float getVoltage() const { return currentVoltage; }
    int getLevel() const { return currentLevel; }
    BatteryState getState() const { return currentState; }
    ChargingState getChargingState() const { return chargingState; }
    
    // Utility functions
    bool isCritical() const { return currentState == BATTERY_STATE_CRITICAL; }
    bool isLow() const { return currentState == BATTERY_STATE_LOW; }
    bool isCharging() const { return chargingState == CHARGING_IN_PROGRESS; }
    
    // Charging detection (if supported by hardware)
    void setChargingState(ChargingState state);
    
    // Settings
    void setUpdateInterval(unsigned long interval);
    void enableNotifications(bool critical, bool low);
    void clearNotificationFlags();
    
    // Debug info
    void printStatus() const;
};

#endif // BATTERY_MANAGER_H
