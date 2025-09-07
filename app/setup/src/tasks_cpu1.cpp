#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

TaskHandle_t sensorMonitorTaskHandle = NULL;

/**
 * Initialize all background tasks
 */
void setupTasksCpu1() {
    logger->info("Initializing tasks cpu 1 ...");

    if (display) {
        xTaskCreateUniversal(
            displayTask, 
            "displayTaskHandler", 
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
        5,                         // Priority
        &sensorMonitorTaskHandle,  // Task handle
        1
    );

    delay(1000);
    logger->info("Tasks initialized on cpu 1");
}