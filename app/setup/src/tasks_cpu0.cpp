#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"
#include <SendTask.h>

// Task IDs for tracking
String cocoHandlerTaskId;
String notePlayerTaskId;

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
        1,
        core,
        "Proccess image to get objects"
    );

    if (cocoHandlerTaskId.isEmpty()) {
        logger->error("Failed to create cocoHandlerTask");
    } else {
        logger->info("pedestrianHandlerTask created with ID: %s", cocoHandlerTaskId.c_str());
    }

    #if SPEAKER_ENABLED
    // Create Note task for musical note playback using SendTask library
    notePlayerTaskId = SendTask::createLoopTaskOnCore(
        notePlayerTask,
        "NotePlayer",
        4096,                    // Stack size
        1,                       // Priority
        core,                    // Core ID (CPU 1)
        "Note musical playback task for audio effects and melodies"
    );

    if (notePlayerTaskId.isEmpty()) {
        logger->error("Failed to create Note task");
    } else {
        logger->info("Note task created with ID: %s", notePlayerTaskId.c_str());
    }
    #endif

    delay(1000);
    logger->info("Tasks initialized on cpu 0");
}