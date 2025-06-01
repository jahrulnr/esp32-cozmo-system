#include <Arduino.h>
#include "config.h"

// Include core libraries
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include "../lib/Sensors/Camera.h"
#include "../lib/Sensors/Gyro.h"
#include "../lib/Motors/MotorControl.h"
#include "../lib/Motors/ServoControl.h"
#include "../lib/Communication/WiFiManager.h"
#include "../lib/Communication/WebSocketHandler.h"
#include "../lib/Screen/Screen.h"
#include "../lib/Utils/Logger.h"

// External component instances (defined in app.ino)
extern Sensors::Camera* camera;
extern Sensors::Gyro* gyro;
extern Motors::MotorControl* motors;
extern Motors::ServoControl* servos;
extern Communication::WiFiManager* wifiManager;
extern Communication::WebSocketHandler* webSocket;
extern Screen::Screen* screen;
extern Utils::Logger* logger;

// Task handles
TaskHandle_t cameraStreamTaskHandle = NULL;
TaskHandle_t sensorMonitorTaskHandle = NULL;

// Function prototypes
void cameraStreamTask(void* parameter);
void sensorMonitorTask(void* parameter);

/**
 * Initialize all background tasks
 */
void initTasks() {
    logger->info("Initializing tasks...");
    
    // Create camera streaming task
    xTaskCreatePinnedToCore(
        cameraStreamTask,        // Task function
        "CameraStream",          // Task name
        8192,                    // Stack size
        NULL,                    // Parameters
        1,                       // Priority
        &cameraStreamTaskHandle, // Task handle
        1                        // Core (1 = second core)
    );
    
    // Create sensor monitoring task
    xTaskCreatePinnedToCore(
        sensorMonitorTask,         // Task function
        "SensorMonitor",           // Task name
        4096,                      // Stack size
        NULL,                      // Parameters
        1,                         // Priority
        &sensorMonitorTaskHandle,  // Task handle
        1                          // Core (1 = second core)
    );
    
    logger->info("Tasks initialized");
}

/**
 * Camera streaming task
 * Captures frames from the camera and streams them via WebSocket
 */
void cameraStreamTask(void* parameter) {
    // Check if camera and WebSocket are initialized
    if (!camera || !webSocket) {
        logger->error("Camera streaming task failed: components not initialized");
        vTaskDelete(NULL);
        return;
    }
    
    logger->info("Camera streaming task started");
    
    // Stream frames forever
    while (true) {
        // Get a frame from the camera
        camera_fb_t* fb = camera->captureFrame();
        
        if (fb) {
            // Send the frame to all connected clients
            webSocket->sendBinary(-1, fb->buf, fb->len);
            
            // Return the frame buffer to the camera
            camera->returnFrame(fb);
        }
        
        // Short delay to prevent watchdog triggering
        vTaskDelay(1000 / CAMERA_FPS / portTICK_PERIOD_MS);
    }
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
        String jsonData = "{";
        
        // Add gyroscope data if available
        if (gyro) {
            gyro->update();
            jsonData += "\"gyro\":{";
            jsonData += "\"x\":" + String(gyro->getX()) + ",";
            jsonData += "\"y\":" + String(gyro->getY()) + ",";
            jsonData += "\"z\":" + String(gyro->getZ());
            jsonData += "}";
        }
        
        jsonData += "}";
        
        // Send the data to all connected clients
        webSocket->sendText(-1, jsonData);
        
        // Delay before next reading
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}