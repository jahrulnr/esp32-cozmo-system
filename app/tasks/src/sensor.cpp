#include "../register.h"
#include <SendTask.h>

/**
 * Sensor monitoring task
 * Reads sensor values
 */
void sensorMonitorTask(void* parameter) {
    logger->info("Sensor monitoring task started");
    const int sendInterval = 1000;
    long currentUpdate = millis();
    SemaphoreHandle_t sensor_handle = xSemaphoreCreateMutex();

    float distance = -1.;
    float temperature = NAN;

    String orientationTask = "orientationTask";
    String distanceSensorTask = "distanceSensorTask";
    String cliffLeftDetectorTask = "cliffLeftDetectorTask";
    String temperatureSensorTask = "temperatureSensorTask";
    
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t updateFrequency = pdMS_TO_TICKS(100);

    // Monitor sensors forever
    while (true) {
        vTaskDelayUntil(&lastWakeTime, updateFrequency);

        bool sendLog = millis() - currentUpdate > sendInterval;

        // Gyroscope and accelerometer
        if (orientation) {
            orientation->update();
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

        // if (touchDetector) {
        //     touchDetector->update();

        //     if (sendLog)
        //         logger->info("touched: %s", touchDetector->detected() ? "yes":"no");
        // }

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