#include <Arduino.h>
#include "app.h"

TaskHandle_t cameraStreamTaskHandle = NULL;
TaskHandle_t sensorMonitorTaskHandle = NULL;

/**
 * Initialize all background tasks
 */
void setupTasks() {
    logger->info("Initializing tasks...");

    #if PROTECT_COZMO
    // protect cozmo
    xTaskCreatePinnedToCore(
            protectCozmoTask,       // Task function
            "protectCozmo",         // Task name
            4 * 1024,               // Stack size
            NULL,                   // Parameters
            1,                      // Priority
            NULL,                    // Task handle
            1
        );
    #endif
    
    // Create camera streaming task
    if (camera) {
        xTaskCreate(
            cameraStreamTask,        // Task function
            "CameraStream",          // Task name
            80 * 1024,               // Stack size
            NULL,                    // Parameters
            1,                       // Priority
            &cameraStreamTaskHandle  // Task handle
        );
        
        logger->info("Camera streaming task initialized");
    } else {
        logger->warning("Camera not initialized, skipping camera stream task");
    }

    if (screen) {
        xTaskCreate([](void *param){
            while(1) {
                screen->mutexUpdate();
                vTaskDelay(pdMS_TO_TICKS(33)); 
            }
        }, "screenUpdate", 4096, NULL, 8, NULL);
    }
    
    // Create sensor monitoring task
    xTaskCreate(
        sensorMonitorTask,         // Task function
        "SensorMonitor",           // Task name
        4096,                      // Stack size
        NULL,                      // Parameters
        10,                         // Priority
        &sensorMonitorTaskHandle   // Task handle
    );
    
    // Initialize automation variables
    _enableAutomation = AUTOMATION_ENABLED;
    _lastManualControlTime = millis();
    
    // Create automation task
    if (automation) {
        automation->start();
        automation->setRandomBehaviorOrder();
    }

    // Task to ping the slave device periodically
    // xTaskCreate([](void *param){
    //     while(1) {
    //         sendPingToSlave();
    //         vTaskDelay(pdMS_TO_TICKS(3000)); // Ping every 5 seconds
    //     }
    // }, "pingDevices", 4096, NULL, 10, NULL);

    delay(1000);
    logger->info("Tasks initialized");
}

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
    const int updateInterval = 3;
    const int sendInterval = 500;
    long currentUpdate = millis();
    float distance;
    SemaphoreHandle_t sensor_handle = xSemaphoreCreateMutex();

    // Monitor sensors forever
    while (true) {
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

        // Microphone sensor
        if (microphoneSensor) {
            checkMicrophone();
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
            if (microphoneSensor && microphoneSensor->isInitialized()) {
                jsonData["microphone"]["level"] = getCurrentSoundLevel();
                jsonData["microphone"]["peak"] = getPeakSoundLevel();
                jsonData["microphone"]["detected"] = isSoundDetected();
            }

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

        vTaskDelay(pdMS_TO_TICKS(updateInterval));
    }
}