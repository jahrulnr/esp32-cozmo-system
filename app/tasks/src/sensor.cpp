#include "../register.h"
#include <SendTask.h>

/**
 * Sensor monitoring task
 * Reads sensor values
 */
void sensorMonitorTask(void* parameter) {
    logger->info("Sensor monitoring task started");
    const int sendInterval = 10000;
    long currentUpdate = millis();

    float distance = -1.;
    float temperature = NAN;
    float batteryVoltage = 0.0;
    int batteryLevel = 0;
    BatteryState batteryState = BATTERY_STATE_CRITICAL;
    
    // Battery long-term sampling variables
    float batteryVoltageSum = 0.0;
    int batterySampleCount = 0;
    unsigned long lastBatteryUpdate = 0;
    const unsigned long batteryUpdateInterval = 10000; // 10 seconds for stable reading
    bool batteryDataReady = false;
    
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t updateFrequency = pdMS_TO_TICKS(50);

    // Monitor sensors forever
    while (true) {
        vTaskDelayUntil(&lastWakeTime, updateFrequency);

        bool sendLog = millis() - currentUpdate > sendInterval;

        // Gyroscope and accelerometer
        if (orientation) {
            orientation->update();

            if (sendLog)
                logger->info("gyro X: %.2f Y: %.2f Z: %.2f | accel X: %.2f Y: %.2f Z: %.2f | mag: %.2f", 
                    orientation->getX(), orientation->getY(), orientation->getZ(),
                    orientation->getAccelX(), orientation->getAccelY(), orientation->getAccelZ(),
                    orientation->getAccelMagnitude());
        }


        scanArea->update();
        ESP_LOGD("ScanArea", "Y: %.2f, D: %.2f", 
            scanArea->getCurrentYaw(), 
            scanArea->getLastDistance());

        // Cliff detectors
        if (cliffLeftDetector && cliffRightDetector) {
            cliffLeftDetector->update();
            cliffRightDetector->update();

            if (sendLog)
                logger->info("cliff R: %s L: %s", 
                    cliffRightDetector->isCliffDetected() ? "yes" : "no",
                    cliffLeftDetector->isCliffDetected() ? "yes" : "no"
                    );
        }

        if (touchDetector) {
            touchDetector->update();

            if (sendLog)
                logger->info("touched: %s", touchDetector->detected() ? "yes":"no");
        }

        if (temperatureSensor) {
            temperature = temperatureSensor->readTemperature();
            
            if (sendLog)
                logger->info("temperature: %.1fC", temperature);
        }

        // Battery monitoring with long-term averaging
        if (batteryManager) {
            unsigned long currentTime = millis();
            
            // Collect battery samples every 50ms for 10 seconds
            batteryManager->update();
            float currentVoltage = batteryManager->getVoltage();
            
            // Skip invalid readings (0V indicates ADC error)
            if (currentVoltage > 0.1) {
                batteryVoltageSum += currentVoltage;
                batterySampleCount++;
            }
            
            // Calculate average after 10 seconds of sampling
            if (currentTime - lastBatteryUpdate >= batteryUpdateInterval && batterySampleCount > 0) {
                // Calculate stable average voltage
                batteryVoltage = batteryVoltageSum / batterySampleCount;
                
                // Update battery manager with our stable reading for level calculation
                // We'll temporarily set the voltage and recalculate level
                float originalVoltage = batteryManager->getVoltage();
                
                // Calculate level based on our averaged voltage
                float voltageMin = 3.3; // From battery manager config
                float voltageMax = 4.2; // From battery manager config
                if (batteryVoltage <= voltageMin) {
                    batteryLevel = 0;
                } else if (batteryVoltage >= voltageMax) {
                    batteryLevel = 100;
                } else {
                    batteryLevel = (int)(((batteryVoltage - voltageMin) / (voltageMax - voltageMin)) * 100.0);
                    batteryLevel = batteryLevel < 0 ? 0 : (batteryLevel > 100 ? 100 : batteryLevel);
                }
                
                // Determine state based on our calculated level
                if (batteryLevel <= 10) batteryState = BATTERY_STATE_CRITICAL;
                else if (batteryLevel <= 25) batteryState = BATTERY_STATE_LOW;
                else if (batteryLevel <= 50) batteryState = BATTERY_STATE_MEDIUM;
                else if (batteryLevel <= 75) batteryState = BATTERY_STATE_HIGH;
                else batteryState = BATTERY_STATE_FULL;
                
                batteryDataReady = true;
                
                // Reset for next sampling period
                batteryVoltageSum = 0.0;
                batterySampleCount = 0;
                lastBatteryUpdate = currentTime;
                
                if (sendLog) {
                    logger->info("Battery averaged over %d samples: %.3fV (%d%%) - %s", 
                        batterySampleCount > 0 ? (int)(batteryUpdateInterval / 50) : 0,
                        batteryVoltage, batteryLevel,
                        batteryState == BATTERY_STATE_CRITICAL ? "CRITICAL" :
                        batteryState == BATTERY_STATE_LOW ? "LOW" :
                        batteryState == BATTERY_STATE_MEDIUM ? "MEDIUM" :
                        batteryState == BATTERY_STATE_HIGH ? "HIGH" : "FULL");
                }
            }
            
            // Log current instantaneous reading if requested (for debugging)
            if (sendLog && batterySampleCount > 0) {
                logger->info("Battery instant: %.3fV (samples: %d, avg so far: %.3fV)", 
                    currentVoltage, batterySampleCount, batteryVoltageSum / batterySampleCount);
            }

            // Send critical battery notifications to display system (only if we have stable data)
            if (batteryDataReady) {
                if (batteryState == BATTERY_STATE_CRITICAL) {
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::BATTERY_CRITICAL);
                } else if (batteryState == BATTERY_STATE_LOW) {
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::BATTERY_LOW);
                }
            }
        }

        if (sendLog) {
            currentUpdate = millis();
        }
    }
}