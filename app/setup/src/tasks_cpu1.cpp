#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"
#include <SendTask.h>

// Task IDs for tracking
String cocoFeedTaskId;
String updaterTaskId;

/**
 * Initialize all background tasks on CPU 1
 */
void setupTasksCpu1() {
    logger->info("Initializing tasks cpu 1 ...");

    bool core = 1;

    #if MICROPHONE_ENABLED
    // Start speech recognition using ESP-SR library
    SR::sr_start(core, ~core);  // Start on core 1
    vTaskDelay(1000);
    logger->info("Speech recognition started on core 1");
    #endif

    cocoFeedTaskId = SendTask::createLoopTaskOnCore(
        cocoFeedTask,
        "cocoFeedTask",
        4096,
        0,
        core
    );

    if (cocoFeedTaskId.isEmpty()) {
        logger->error("Failed to create cocoFeedTask");
    } else {
        logger->info("cocoFeedTask created with ID: %s", cocoFeedTaskId.c_str());
    }

    // Create sensor monitoring task using SendTask library
    updaterTaskId = SendTask::createLoopTaskOnCore(
        updaterTask,
        "UpdaterTask",
        4096,                        // Stack size
        5,                           // Priority
        core                         // Core ID (CPU 0)
    );

    if (updaterTaskId.isEmpty()) {
        logger->error("Failed to create updater task");
    } else {
        logger->info("UpdaterTask monitor task created with ID: %s", updaterTaskId.c_str());
    }

    delay(1000);
    logger->info("Tasks initialized on cpu 1");
}