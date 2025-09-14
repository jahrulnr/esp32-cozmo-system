#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"
#include <SendTask.h>

// Task IDs for tracking
String taskMonitorerId;
String displayTaskId;
String sensorMonitorTaskId;
String cameraTaskId;

/**
 * Initialize all background tasks on CPU 0
 */
void setupTasksCpu0() {
    logger->info("Initializing tasks cpu 0 ...");
    
    bool core = 0;
    
    // Create display task using SendTask library
    if (display) {
        displayTaskId = SendTask::createLoopTaskOnCore(
            displayTask,
            "DisplayTask",
            4096,                    // Stack size
            5,                       // Priority
            core,                    // Core ID (CPU 0)
            "Display task for face animation and UI updates"
        );
        
        if (displayTaskId.isEmpty()) {
            logger->error("Failed to create display task");
        } else {
            logger->info("Display task created with ID: %s", displayTaskId.c_str());
        }
    }
    
    // Create sensor monitoring task using SendTask library
    sensorMonitorTaskId = SendTask::createLoopTaskOnCore(
        sensorMonitorTask,
        "SensorMonitor",
        4096,                        // Stack size
        5,                           // Priority
        core,                        // Core ID (CPU 0)
        "Sensor monitoring task for distance, orientation, and cliff detection"
    );
    
    if (sensorMonitorTaskId.isEmpty()) {
        logger->error("Failed to create sensor monitor task");
    } else {
        logger->info("Sensor monitor task created with ID: %s", sensorMonitorTaskId.c_str());
    }
    
    // Create camera task using SendTask library
    cameraTaskId = SendTask::createLoopTaskOnCore(
        cameraTask,
        "CameraTask",
        4096,                        // Stack size
        0,                           // Priority (lower priority)
        core,                        // Core ID (CPU 0)
        "Camera capture and processing task"
    );
    
    if (cameraTaskId.isEmpty()) {
        logger->error("Failed to create camera task");
    } else {
        logger->info("Camera task created with ID: %s", cameraTaskId.c_str());
    }

    // Create taskMonitorer task using SendTask library
    // taskMonitorerId = SendTask::createLoopTaskOnCore(
    //     taskMonitorer,
    //     "taskMonitorer",
    //     4096,                        // Stack size
    //     0,                           // Priority (lower priority)
    //     core,                        // Core ID (CPU 0)
    //     "Task monitor"
    // );
    
    // if (taskMonitorerId.isEmpty()) {
    //     logger->error("Failed to create task monitor");
    // } else {
    //     logger->info("Task monitor created with ID: %s", taskMonitorerId.c_str());
    // }

    delay(1000);
    logger->info("Tasks initialized on cpu 0");
}