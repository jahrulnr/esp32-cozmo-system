#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

TaskHandle_t cameraStreamTaskHandle = NULL;
TaskHandle_t sensorMonitorTaskHandle = NULL;

/**
 * Initialize all background tasks
 */
void setupTasks() {
    logger->info("Initializing tasks...");

    #if PROTECT_COZMO
    // protect cozmo
    xTaskCreateUniversal(
            protectCozmoTask,       // Task function
            "protectCozmo",         // Task name
            4 * 1024,               // Stack size
            NULL,                   // Parameters
            configMAX_PRIORITIES - 1, // Priority
            NULL,                   // Task handle
            1
        );
    #endif
    
    // Create camera streaming task
    if (camera) {
        xTaskCreateUniversal(
            cameraStreamTask,        // Task function
            "CameraStream",          // Task name
            40 * 1024,               // Stack size
            NULL,                    // Parameters
            4,                      // Priority
            &cameraStreamTaskHandle,  // Task handle
            1
        );
        
        logger->info("Camera streaming task initialized");
    } else {
        logger->warning("Camera not initialized, skipping camera stream task");
    }

    if (screen) {
        xTaskCreateUniversal(
            screenTask, 
            "screenTaskHandler", 
            4096, 
            NULL, 
            5, 
            NULL,
            1
        );
    }
    
    // Create sensor monitoring task
    xTaskCreateUniversal(
        sensorMonitorTask,         // Task function
        "SensorMonitor",           // Task name
        4096,                      // Stack size
        NULL,                      // Parameters
        0,                         // Priority
        &sensorMonitorTaskHandle,  // Task handle
        1
    );
    
    // Create automation task
    if (automation) {
        automation->start();
        automation->setRandomBehaviorOrder();
    }

    delay(1000);
    logger->info("Tasks initialized");
}