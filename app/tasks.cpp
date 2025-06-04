#include <Arduino.h>
#include "init.h"

TaskHandle_t cameraStreamTaskHandle = NULL;
TaskHandle_t sensorMonitorTaskHandle = NULL;

// Function prototypes
void cameraStreamTask(void* parameter);
void sensorMonitorTask(void* parameter);

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
    
    // Monitor sensors forever
    while (true) {
        // Create JSON object for sensor data
        Utils::SpiJsonDocument jsonData;
        
        // Add gyroscope and accelerometer data if available
        if (gyro) {
            gyro->update();
            
            // Create nested objects for gyro and accelerometer data
            jsonData["gyro"]["x"] = String(gyro->getX());
            jsonData["gyro"]["y"] = String(gyro->getY());
            jsonData["gyro"]["z"] = String(gyro->getZ());
            
            jsonData["accel"]["x"] = String(gyro->getAccelX());
            jsonData["accel"]["y"] = String(gyro->getAccelY());
            jsonData["accel"]["z"] = String(gyro->getAccelZ());
            jsonData["accel"]["magnitude"] = String(gyro->getAccelMagnitude());
        
            // Send the data to all connected clients
            String data;
            serializeJson(jsonData, data);
            webSocket->sendJsonMessage(-1, "sensor_data", data);
        }
        
        // Delay before next reading
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}