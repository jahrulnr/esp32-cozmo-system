#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

TaskHandle_t sensorMonitorTaskHandle = NULL;

/**
 * Initialize all background tasks
 */
void setupTasksCpu1() {
    logger->info("Initializing tasks cpu 1 ...");
    const bool core = 1;

    #if PROTECT_COZMO
    // protect cozmo
    xTaskCreateUniversal(
        protectCozmoTask, "protectCozmo", 4 * 1024, NULL, 2, NULL, core
    );
    #endif
    
    // Create automation task
    if (automation) {
        automation->start(core);
        automation->setRandomBehaviorOrder();
    }

    xTaskCreateUniversal(
        ftpTask, 
        "ftpTaskHandler", 
        1024 * 8, 
        NULL, 
        1, 
        NULL,
        core
    );

    xTaskCreateUniversal(
        weatherServiceTask, "weatherServiceTaskHandler", 
        1024 * 4, 
        NULL, 
        0, 
        &weatherServiceTaskHandle,
        core
    );

    #if MICROPHONE_ENABLED
    SR::sr_start(core);
    #endif

    #if MICROPHONE_ENABLED
    // Create SR control task for handling ESP-SR pause/resume events
    xTaskCreateUniversal(
        srControlTask,
        "SRControl",
        4096,                     // Small stack size - just handles notifications
        NULL,
        0,                        // Higher priority for responsiveness
        NULL,
        core                      // Core 0 - same as ESP-SR
    );
    #endif

    delay(1000);
    logger->info("Tasks initialized on cpu 1");
}