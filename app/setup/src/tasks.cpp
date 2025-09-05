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
            protectCozmoTask, "protectCozmo", 4 * 1024, NULL, 10, NULL, 0
        );
    #endif

    if (screen) {
        xTaskCreateUniversal(
            screenTask, 
            "screenTaskHandler", 
            4096, 
            NULL, 
            5, 
            NULL,
            0
        );
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
    
    // Create automation task
    if (automation) {
        automation->start();
        automation->setRandomBehaviorOrder();
    }

    xTaskCreateUniversal(
        ftpTask, 
        "ftpTaskHandler", 
        1024 * 8, 
        NULL, 
        5, 
        NULL,
        0
    );

    xTaskCreateUniversal(
        weatherServiceTask, "weatherServiceTaskHandler", 
        1024 * 8, 
        NULL, 
        1, 
        &weatherServiceTaskHandle,
        0
    );

    delay(1000);
    logger->info("Tasks initialized");
}