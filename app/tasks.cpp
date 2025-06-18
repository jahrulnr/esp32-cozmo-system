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
            40 * 1024,               // Stack size
            NULL,                    // Parameters
            1,                       // Priority
            &cameraStreamTaskHandle  // Task handle
        );
        
        logger->info("Camera streaming task initialized");
    } else {
        logger->warning("Camera not initialized, skipping camera stream task");
    }
    
    // Create sensor monitoring task
    xTaskCreate(
        sensorMonitorTask,         // Task function
        "SensorMonitor",           // Task name
        4096,                      // Stack size
        NULL,                      // Parameters
        1,                         // Priority
        &sensorMonitorTaskHandle   // Task handle
    );
    
    // Initialize automation variables
    g_automationEnabled = AUTOMATION_ENABLED;
    g_lastManualControlTime = millis();
    
    // Create automation task
    if (automation) {
        automation->start();
        automation->setRandomBehaviorOrder();
    }

    // Task to ping the slave device periodically
    xTaskCreate([](void *param){
        while(1) {
            sendPingToSlave();
            vTaskDelay(pdMS_TO_TICKS(3000)); // Ping every 5 seconds
        }
    }, "pingDevices", 4096, NULL, 10, NULL);

    // Task to request camera data from the slave device periodically 
    // and process complete frames
    xTaskCreate([](void *param){
        // Wait a bit before starting to ensure system is fully initialized
        vTaskDelay(pdMS_TO_TICKS(2000));

        bool ready = true;
        unsigned long requestTimeout = 10000;
        unsigned long now = millis();
        
        while(1) {

            if (ready) {
                // Request new camera data
                bool requestSent = requestCameraDataFromSlave();
                if (requestSent) {
                    logger->debug("Camera data requested from slave");
                    ready = false;
                    now = millis();
                } else {
                    logger->warning("Failed to request camera data from slave");
                }
            }
            
            // Check if a complete frame is available
            if (isSlaveCameraFrameComplete()) {
                logger->info("Complete slave camera frame received");
                
                // Get frame information
                uint16_t width = 0, height = 0;
                getSlaveCameraImageDimensions(&width, &height);
                uint32_t imageSize = getSlaveCameraImageSize();
                
                logger->info("Camera frame details - Size: %u bytes, Resolution: %dx%d", 
                            imageSize, width, height);
                
                if (isSlaveCameraDataJPEG()) {
                    logger->info("Frame format: JPEG");
                } else {
                    logger->info("Frame format: Raw data");
                }
                
                // Process the frame (just logs in this implementation)
                processSlaveCameraFrame();
                
                // After processing, reset the camera data to free memory
                // If you want to keep the last frame, comment out this line
                resetSlaveCameraData();
                ready = true;
            } 
            else if (millis() - now >= requestTimeout) {
                ready = true;
                logger->warning("Timeout: Request camera data from slave");
                
                // Reset any partial data that might be there
                resetSlaveCameraData();
            } 
            else {
                // Follow up request for missing blocks
                if (slaveCameraData.dataAvailable && !slaveCameraData.frameComplete) {
                    // Check if we have block information
                    if (slaveCameraData.blockReceived && slaveCameraData.totalBlocks > 0) {
                        // Find first missing block
                        bool missingBlockFound = false;
                        for (uint16_t i = 0; i < slaveCameraData.totalBlocks; i++) {
                            if (!slaveCameraData.blockReceived[i]) {
                                // Request this missing block
                                logger->debug("Requesting missing block %d of %d", i, slaveCameraData.totalBlocks);
                                requestCameraDataBlockFromSlave(i);
                                missingBlockFound = true;
                                break; // Only request one block at a time
                            }
                        }
                        
                        if (!missingBlockFound) {
                            // All blocks marked as received, but frame not marked complete?
                            // This should not happen, but just in case:
                            if (slaveCameraData.receivedBlocks == slaveCameraData.totalBlocks) {
                                logger->info("All blocks received but frame not marked complete. Marking as complete.");
                                slaveCameraData.frameComplete = true;
                            }
                        }
                    }
                }
            }
            
            // Wait before next request
            vTaskDelay(pdMS_TO_TICKS(33)); // Request camera data every 1 second
        }
    }, "slaveCameraRequest", 1024 * 40, NULL, 5, NULL);

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
    const int sendInterval = 1000;
    long currentUpdate = millis();
    SemaphoreHandle_t sensor_handle = xSemaphoreCreateMutex();
    
    // Monitor sensors forever
    while (true) {

        // Create JSON object for sensor data
        Utils::SpiJsonDocument jsonData;
        
        // Add gyroscope and accelerometer data if available
        if (orientation && xSemaphoreTake(sensor_handle, 1) == pdTRUE) {
            orientation->update();
            
            // Create nested objects for gyro and accelerometer data
            jsonData["gyro"]["x"] = orientation->getX();
            jsonData["gyro"]["y"] = orientation->getY();
            jsonData["gyro"]["z"] = orientation->getZ();
            
            jsonData["accel"]["x"] = orientation->getAccelX();
            jsonData["accel"]["y"] = orientation->getAccelY();
            jsonData["accel"]["z"] = orientation->getAccelZ();
            jsonData["accel"]["magnitude"] = orientation->getAccelMagnitude();
            jsonData.shrinkToFit();
            xSemaphoreGive(sensor_handle);
            vTaskDelay(pdMS_TO_TICKS(updateInterval));
        }
        
        // Add distance sensor data if available
        if (distanceSensor && xSemaphoreTake(sensor_handle, 1) == pdTRUE) {
            float distance = distanceSensor->measureDistance();
            jsonData["distance"]["value"] = distance;
            jsonData["distance"]["unit"] = "cm";
            jsonData["distance"]["valid"] = (distance >= 0);
            jsonData["distance"]["obstacle"] = distanceSensor->isObstacleDetected();
            jsonData.shrinkToFit();
            xSemaphoreGive(sensor_handle);
            vTaskDelay(pdMS_TO_TICKS(updateInterval));
        }
        
        // Add cliff detector data if available
        if (cliffLeftDetector && cliffRightDetector && xSemaphoreTake(sensor_handle, 1) == pdTRUE) {
            cliffLeftDetector->update();
            cliffRightDetector->update();
            jsonData["cliff"]["left"] = cliffLeftDetector->isCliffDetected();
            jsonData["cliff"]["right"] = cliffRightDetector->isCliffDetected();
            jsonData.shrinkToFit();
            xSemaphoreGive(sensor_handle);
            vTaskDelay(pdMS_TO_TICKS(updateInterval));
        }
        
        // Add temperature sensor data if available
        if (temperatureSensor && temperatureSensor->isSupported() && xSemaphoreTake(sensor_handle, 1) == pdTRUE) {
            float temperature = temperatureSensor->readTemperature();
            if (!isnan(temperature)) {
                jsonData["temperature"]["value"] = temperature;
                jsonData["temperature"]["unit"] = "C";
                jsonData.shrinkToFit();
                
                // Periodically trigger temperature-based behavior check
                static unsigned long lastTempBehaviorCheck = 0;
                if (millis() - lastTempBehaviorCheck > 5000) { // Check every 5 seconds
                    checkTemperature();
                    lastTempBehaviorCheck = millis();
                }
            }
            xSemaphoreGive(sensor_handle);
            vTaskDelay(pdMS_TO_TICKS(updateInterval));
        }
        
        // Add temperature sensor data if available
        if (temperatureSensor && temperatureSensor->isSupported() && xSemaphoreTake(sensor_handle, 1) == pdTRUE) {
            float temperature = temperatureSensor->readTemperature();
            if (!isnan(temperature)) {
                jsonData["temperature"]["value"] = temperature;
                jsonData["temperature"]["unit"] = "C";
                jsonData.shrinkToFit();
                
                // Periodically trigger temperature-based behavior check
                static unsigned long lastTempBehaviorCheck = 0;
                if (millis() - lastTempBehaviorCheck > 5000) { // Check every 5 seconds
                    checkTemperature();
                    lastTempBehaviorCheck = millis();
                }
            }
            xSemaphoreGive(sensor_handle);
            vTaskDelay(pdMS_TO_TICKS(updateInterval));
        }
        
        // Add temperature sensor data if available
        if (servos && xSemaphoreTake(sensor_handle, 1) == pdTRUE) {
            int servoHead = map(servos->getHead(), 0, 180, -100, 100);
            int servoHand = map(servos->getHand(), 0, 180, -100, 100);

            
            jsonData["servo"]["head"] = servoHead;
            jsonData["servo"]["hand"] = servoHand;
            jsonData.shrinkToFit();
            xSemaphoreGive(sensor_handle);
            vTaskDelay(pdMS_TO_TICKS(updateInterval));
        }
        
        // Send the data to all connected clients using the DTO v1.0 format
        if (millis() - currentUpdate >= sendInterval){
            webSocket->sendJsonMessage(-1, "sensor_data", jsonData);
            currentUpdate = millis();
        }
        
        // Delay before next reading
        vTaskDelay(pdMS_TO_TICKS(updateInterval));
    }
}