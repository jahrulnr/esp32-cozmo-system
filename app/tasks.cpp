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
    xTaskCreate(
            protectCozmoTask,       // Task function
            "protectCozmo",         // Task name
            4 * 1024,               // Stack size
            NULL,                   // Parameters
            10,                      // Priority
            NULL                    // Task handle
        );
    #endif
    
    // Create camera streaming task
    if (camera) {
        xTaskCreateUniversal(
            cameraStreamTask,        // Task function
            "CameraStream",          // Task name
            40 * 1024,               // Stack size
            NULL,                    // Parameters
            8 ,                      // Priority
            &cameraStreamTaskHandle,  // Task handle
            0
        );
        
        logger->info("Camera streaming task initialized");
    } else {
        logger->warning("Camera not initialized, skipping camera stream task");
    }

    if (screen) {
        xTaskCreateUniversal([](void *param){
            while(1) {
                screen->mutexUpdate();
                vTaskDelay(pdMS_TO_TICKS(33)); 
            }
        }, "screenUpdate", 4096, NULL, 5, NULL, 0);
    }
    
    // Create sensor monitoring task
    xTaskCreateUniversal(
        sensorMonitorTask,         // Task function
        "SensorMonitor",           // Task name
        4096,                      // Stack size
        NULL,                      // Parameters
        5,                         // Priority
        &sensorMonitorTaskHandle,  // Task handle
        0
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

    if (SPEAKER_ENABLED) {
        xTaskCreateUniversal([](void *param){
            while(1) {
                if (isSpeakerPlaying()) {
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    continue;
                }

                if (playSpeakerRandomMP3()){
                    logger->info("success play a random mp3");
                }
                
                vTaskDelay(pdMS_TO_TICKS(10000)); 
		        taskYIELD();
            }
        }, "autoSound", 4 * 1024, NULL, 5, NULL, 0);
    }

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
                jsonData["microphone"]["initialized"] = true;
                jsonData["microphone"]["recording"] = isVoiceRecording();
                jsonData["microphone"]["voice_detected"] = isVoiceDetected();
            } else {
                jsonData["microphone"]["level"] = 0;
                jsonData["microphone"]["peak"] = 0;
                jsonData["microphone"]["detected"] = false;
                jsonData["microphone"]["initialized"] = false;
                jsonData["microphone"]["recording"] = false;
                jsonData["microphone"]["voice_detected"] = false;
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

        vTaskDelay(pdMS_TO_TICKS(updateInterval));
		taskYIELD();
    }
}