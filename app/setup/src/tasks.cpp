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
            4,                      // Priority
            &cameraStreamTaskHandle,  // Task handle
            1
        );
        
        logger->info("Camera streaming task initialized");
    } else {
        logger->warning("Camera not initialized, skipping camera stream task");
    }

    if (screen) {
        xTaskCreate(
            screenTask, 
            "screenTaskHandler", 
            4096, 
            NULL, 
            4, 
            NULL
        );
    }
    
    // Create sensor monitoring task
    xTaskCreate(
        sensorMonitorTask,         // Task function
        "SensorMonitor",           // Task name
        4096,                      // Stack size
        NULL,                      // Parameters
        0,                         // Priority
        &sensorMonitorTaskHandle   // Task handle
    );
    
    // Initialize automation variables
    _enableAutomation = AUTOMATION_ENABLED;
    _lastManualControlTime = millis();
    
    // Create automation task
    if (automation) {
        automation->start();
        automation->setRandomBehaviorOrder();
    }

    // if (SPEAKER_ENABLED) {
    //     xTaskCreate([](void *param){
    //         while(1) {
    //             if (isSpeakerPlaying()) {
    //                 vTaskDelay(pdMS_TO_TICKS(5000));
    //                 continue;
    //             }

    //             if (playSpeakerRandomMP3()){
    //                 logger->info("success play a random mp3");
    //             }
                
    //             vTaskDelay(pdMS_TO_TICKS(10000)); 
	// 	        taskYIELD();
    //         }
    //     }, "autoSound", 4 * 1024, NULL, 4, NULL);
    // }

    #if MICROPHONE_ENABLED
        xTaskCreate(
            speechRecognitionTask,
            "speechRecognitionTask", 
            4 * 1024, 
            NULL, 
            4, 
            NULL
        );
    #endif

    delay(1000);
    logger->info("Tasks initialized");
}