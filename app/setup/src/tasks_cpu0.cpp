#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

/**
 * Initialize all background tasks
 */
void setupTasksCpu0() {
    logger->info("Initializing tasks cpu 0 ...");

    #if PROTECT_COZMO
    // protect cozmo
    xTaskCreateUniversal(
            protectCozmoTask, "protectCozmo", 4 * 1024, NULL, 2, NULL, 0
        );
    #endif
    
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
        1, 
        NULL,
        0
    );

    xTaskCreateUniversal(
        weatherServiceTask, "weatherServiceTaskHandler", 
        1024 * 4, 
        NULL, 
        0, 
        &weatherServiceTaskHandle,
        0
    );

    #if MICROPHONE_ENABLED
    SR::sr_start(0);
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
        0                         // Core 0 - same as ESP-SR
    );
    #endif

    delay(1000);
    logger->info("Tasks initialized on cpu 0");
}