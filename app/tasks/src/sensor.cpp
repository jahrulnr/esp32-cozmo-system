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

        // Distance sensor
        if (distanceSensor) {
            distance = distanceSensor->measureDistance();

            if (sendLog)
                logger->info("distance: %.2fcm", distance);
        }

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

        if (sendLog) {
            currentUpdate = millis();
        }
    }
}