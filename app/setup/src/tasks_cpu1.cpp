#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"
#include <SendTask.h>

// Task IDs for tracking
String ftpTaskId;
String weatherServiceTaskId;
String srControlTaskId;
String notePlayerTaskId;

/**
 * Initialize all background tasks on CPU 1
 */
void setupTasksCpu1() {
    logger->info("Initializing tasks cpu 1 ...");

    bool core = 1;

    // Create FTP task using SendTask library
    ftpTaskId = SendTask::createLoopTaskOnCore(
        ftpTask,
        "FTPTask",
        1024 * 8,                // Stack size
        1,                       // Priority
        core,                    // Core ID (CPU 1)
        "FTP server task for file management"
    );
    
    if (ftpTaskId.isEmpty()) {
        logger->error("Failed to create FTP task");
    } else {
        logger->info("FTP task created with ID: %s", ftpTaskId.c_str());
    }

    // Create weather service task using SendTask library
    weatherServiceTaskId = SendTask::createLoopTaskOnCore(
        weatherServiceTask,
        "WeatherService",
        1024 * 4,                // Stack size
        0,                       // Priority
        core,                    // Core ID (CPU 1)
        "Weather service task for weather data updates"
    );
    
    if (weatherServiceTaskId.isEmpty()) {
        logger->error("Failed to create weather service task");
    } else {
        logger->info("Weather service task created with ID: %s", weatherServiceTaskId.c_str());
    }

    #if MICROPHONE_ENABLED
    // Start speech recognition using ESP-SR library
    SR::sr_start(core);  // Start on core 1
    logger->info("Speech recognition started on core 1");
    #endif

    #if MICROPHONE_ENABLED
    // Create SR control task for handling ESP-SR pause/resume events using SendTask library
    srControlTaskId = SendTask::createLoopTaskOnCore(
        srControlTask,
        "SRControl",
        4096,                    // Stack size
        0,                       // Priority (higher responsiveness)
        core,                    // Core ID (CPU 1, same as ESP-SR)
        "Speech recognition control task for pause/resume handling"
    );
    
    if (srControlTaskId.isEmpty()) {
        logger->error("Failed to create SR control task");
    } else {
        logger->info("SR control task created with ID: %s", srControlTaskId.c_str());
    }
    #endif

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
    logger->info("Tasks initialized on cpu 1");
}