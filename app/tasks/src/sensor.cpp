#include "../register.h"

/**
 * Sensor monitoring task
 * Reads sensor values and broadcasts them via WebSocket
 */
void sensorMonitorTask(void* parameter) {
    // Check if WebSocket is initialized
    if (!webSocket) {
        logger->error("Sensor monitoring task failed: WebSocket not initialized");
        vTaskDelete(NULL);
        return;
    }
    
    logger->info("Sensor monitoring task started");
    const int sendInterval = 500;
    long currentUpdate = millis();
    float distance;
    SemaphoreHandle_t sensor_handle = xSemaphoreCreateMutex();
    
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t updateFrequency = pdMS_TO_TICKS(33);

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

        // Send the data to all connected clients using the DTO v1.0 format
        if (millis() - currentUpdate >= sendInterval) {
            Utils::SpiJsonDocument jsonData;
            if (orientation) {
                jsonData["gyro"]["x"] = orientation->getX();
                jsonData["gyro"]["y"] = orientation->getY();
                jsonData["gyro"]["z"] = orientation->getZ();

                jsonData["accel"]["x"] = orientation->getAccelX();
                jsonData["accel"]["y"] = orientation->getAccelY();
                jsonData["accel"]["z"] = orientation->getAccelZ();
                jsonData["accel"]["magnitude"] = orientation->getAccelMagnitude();
            }

            // Distance sensor
            if (distanceSensor) {
                jsonData["distance"]["value"] = distance;
                jsonData["distance"]["unit"] = "cm";
                jsonData["distance"]["valid"] = (distance >= 0);
                jsonData["distance"]["obstacle"] = distanceSensor->isObstacleDetected();
            }

            // Cliff detectors
            if (cliffLeftDetector && cliffRightDetector) {
                jsonData["cliff"]["left"] = cliffLeftDetector->isCliffDetected();
                jsonData["cliff"]["right"] = cliffRightDetector->isCliffDetected();
            }

            // Temperature sensor
            if (temperatureSensor && temperatureSensor->isSupported()) {
                float temperature = temperatureSensor->readTemperature();
                if (!isnan(temperature)) {
                    jsonData["temperature"]["value"] = temperature;
                    jsonData["temperature"]["unit"] = "C";

                    static unsigned long lastTempBehaviorCheck = 0;
                    if (millis() - lastTempBehaviorCheck > 5000) {
                        checkTemperature();
                        lastTempBehaviorCheck = millis();
                    }
                }
            }

            // Microphone sensor
          #if MICROPHONE_I2S
            if (microphone && microphone->isInitialized()) {
                jsonData["microphone"]["level"] = microphone->readLevel();
                jsonData["microphone"]["peak"] = microphone->readLevel();
                jsonData["microphone"]["initialized"] = true;
                jsonData["microphone"]["recording"] = false;
          #elif MICROPHONE_ANALOG
            if (amicrophone && amicrophone->isInitialized()) {
                jsonData["microphone"]["level"] = amicrophone->readPeakLevel();
                jsonData["microphone"]["peak"] = amicrophone->readLevel();
                jsonData["microphone"]["initialized"] = true;
                jsonData["microphone"]["recording"] = false;
          #endif
            } else {
                jsonData["microphone"]["level"] = 0;
                jsonData["microphone"]["peak"] = 0;
                jsonData["microphone"]["initialized"] = false;
                jsonData["microphone"]["recording"] = false;
            }

            // Speaker status
            #if SPEAKER_ENABLED
            jsonData["speaker"]["enabled"] = true;
            jsonData["speaker"]["playing"] = isSpeakerPlaying();
            jsonData["speaker"]["volume"] = getSpeakerVolume();
            jsonData["speaker"]["type"] = getSpeakerType();
            #else
            jsonData["speaker"]["enabled"] = false;
            jsonData["speaker"]["playing"] = false;
            jsonData["speaker"]["volume"] = 0;
            jsonData["speaker"]["type"] = "None";
            #endif

            // Servo positions
            if (servos) {
                int servoHead = map(servos->getHead(), 0, 180, -100, 100);
                int servoHand = map(servos->getHand(), 0, 180, -100, 100);
                jsonData["servo"]["head"] = servoHead;
                jsonData["servo"]["hand"] = servoHand;
            }

            webSocket->sendJsonMessage(-1, "sensor_data", jsonData);
            currentUpdate = millis();
        }
    }
}