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
    xTaskCreate(
            protectCozmoTask,       // Task function
            "protectCozmo",         // Task name
            4 * 1024,               // Stack size
            NULL,                   // Parameters
            10,                      // Priority
            NULL                    // Task handle
        );
    #endif
    
    // Create camera streaming task
    if (camera) {
        xTaskCreateUniversal(
            cameraStreamTask,        // Task function
            "CameraStream",          // Task name
            40 * 1024,               // Stack size
            NULL,                    // Parameters
            8 ,                      // Priority
            &cameraStreamTaskHandle,  // Task handle
            0
        );
        
        logger->info("Camera streaming task initialized");
    } else {
        logger->warning("Camera not initialized, skipping camera stream task");
    }

    if (screen) {
        xTaskCreateUniversal([](void *param){
            while(1) {
                screen->mutexUpdate();
                vTaskDelay(pdMS_TO_TICKS(33)); 
            }
        }, "screenUpdate", 4096, NULL, 5, NULL, 0);
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
    
    // Initialize automation variables
    _enableAutomation = AUTOMATION_ENABLED;
    _lastManualControlTime = millis();
    
    // Create automation task
    if (automation) {
        automation->start();
        automation->setRandomBehaviorOrder();
    }

    // Task to ping the slave device periodically
    // xTaskCreate([](void *param){
    //     while(1) {
    //         sendPingToSlave();
    //         vTaskDelay(pdMS_TO_TICKS(3000)); // Ping every 5 seconds
    //     }
    // }, "pingDevices", 4096, NULL, 10, NULL);

    if (SPEAKER_ENABLED) {
        xTaskCreateUniversal([](void *param){
            while(1) {
                if (isSpeakerPlaying()) {
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    continue;
                }

                if (playSpeakerRandomMP3()){
                    logger->info("success play a random mp3");
                }
                
                vTaskDelay(pdMS_TO_TICKS(10000)); 
		        taskYIELD();
            }
        }, "autoSound", 4 * 1024, NULL, 5, NULL, 0);
    }

    delay(1000);
    logger->info("Tasks initialized");
}