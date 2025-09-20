#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"
#include <SendTask.h>

// Task IDs for tracking
String cocoHandlerTaskId;

/**
 * Initialize all background tasks on CPU 0
 */
void setupTasksCpu0() {
    logger->info("Initializing tasks cpu 0 ...");

    bool core = 0;

    cocoHandlerTaskId = SendTask::createLoopTaskOnCore(
        cocoHandlerTask,
        "cocoHandlerTask",
        4096,
        0,
        core,
        "Proccess image to get objects"
    );

    if (cocoHandlerTaskId.isEmpty()) {
        logger->error("Failed to create cocoHandlerTask");
    } else {
        logger->info("pedestrianHandlerTask created with ID: %s", cocoHandlerTaskId.c_str());
    }

    delay(1000);
    logger->info("Tasks initialized on cpu 0");
}