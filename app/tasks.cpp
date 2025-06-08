#include <Arduino.h>
#include "app.h"

TaskHandle_t automationTaskHandle = NULL;
TaskHandle_t cameraStreamTaskHandle = NULL;
TaskHandle_t sensorMonitorTaskHandle = NULL;

/**
 * Initialize all background tasks
 */
void setupTasks() {
    logger->info("Initializing tasks...");
    
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

    #ifdef AUTOMATION
    xTaskCreate(
        automationTask,         // Task function
        "Automation",           // Task name
        20 * 1024,              // Stack size
        NULL,                   // Parameters
        1,                      // Priority
        &automationTaskHandle   // Task handle
    );
    #endif

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
    const int updateInterval = 50;
    const int sendInterval = 399;
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
        }
        
        // Add cliff detector data if available
        if (cliffLeftDetector && cliffRightDetector && xSemaphoreTake(sensor_handle, 1) == pdTRUE) {
            cliffLeftDetector->update();
            cliffRightDetector->update();
            jsonData["cliff"]["left"] = cliffLeftDetector->isCliffDetected();
            jsonData["cliff"]["right"] = cliffRightDetector->isCliffDetected();
            jsonData.shrinkToFit();
            xSemaphoreGive(sensor_handle);
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
        }
        
        // Add temperature sensor data if available
        if (servos && xSemaphoreTake(sensor_handle, 1) == pdTRUE) {
            int servoHead = map(servos->getHead(), 0, 180, -100, 100);
            int servoHand = map(servos->getHand(), 0, 180, -100, 100);

            
            jsonData["servo"]["head"] = servoHead;
            jsonData["servo"]["hand"] = servoHand;
            jsonData.shrinkToFit();
            xSemaphoreGive(sensor_handle);
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