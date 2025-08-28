#include "../register.h"

/**
 * Sensor monitoring task
 * Reads sensor values
 */
void sensorMonitorTask(void* parameter) {
    logger->info("Sensor monitoring task started");
    const int sendInterval = 500;
    long currentUpdate = millis();
    float distance;
    SemaphoreHandle_t sensor_handle = xSemaphoreCreateMutex();
    
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t updateFrequency = pdMS_TO_TICKS(100);

    // Monitor sensors forever
    while (true) {
        vTaskDelayUntil(&lastWakeTime, updateFrequency);

        // Gyroscope and accelerometer
        if (orientation) {
            orientation->update();
        }

        // Distance sensor
        if (distanceSensor) {
            distance = distanceSensor->measureDistance();
        }

        // Cliff detectors
        if (cliffLeftDetector && cliffRightDetector) {
            cliffLeftDetector->update();
            cliffRightDetector->update();
        }
    }
}