#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

/**
 * Initialize all background tasks
 */
void setupTasksCpu0() {
    logger->info("Initializing tasks cpu 0 ...");
    const bool core = 0;

    if (display) {
        xTaskCreateUniversal(
            displayTask, 
            "displayTaskHandler", 
            4096, 
            NULL, 
            5, 
            NULL,
            core
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
        core
    );
    
    // Create camera task
    xTaskCreateUniversal(
        cameraTask,         // Task function
        "cameraTask",           // Task name
        4096,                      // Stack size
        NULL,                      // Parameters
        0,                         // Priority
        NULL,  // Task handle
        core
    );

    delay(1000);
    logger->info("Tasks initialized on cpu 0");
}