#include <Arduino.h>
#include "init.h"

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

    xTaskCreate(
        automationTask,         // Task function
        "Automation",          // Task name
        4096,                   // Stack size
        NULL,                   // Parameters
        1,                      // Priority
        &automationTaskHandle   // Task handle
    );

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
    
    // Monitor sensors forever
    while (true) {
        // Create JSON object for sensor data
        Utils::SpiJsonDocument jsonData;
        
        // Add gyroscope and accelerometer data if available
        if (orientation) {
            orientation->update();
            
            // Create nested objects for gyro and accelerometer data
            jsonData["gyro"]["x"] = orientation->getX();
            jsonData["gyro"]["y"] = orientation->getY();
            jsonData["gyro"]["z"] = orientation->getZ();
            
            jsonData["accel"]["x"] = orientation->getAccelX();
            jsonData["accel"]["y"] = orientation->getAccelY();
            jsonData["accel"]["z"] = orientation->getAccelZ();
            jsonData["accel"]["magnitude"] = orientation->getAccelMagnitude();
        }
        
        // Add distance sensor data if available
        if (distanceSensor) {
            float distance = distanceSensor->measureDistance();
            jsonData["distance"]["value"] = distance;
            jsonData["distance"]["unit"] = "cm";
            jsonData["distance"]["valid"] = (distance >= 0);
            jsonData["distance"]["obstacle"] = distanceSensor->isObstacleDetected();
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