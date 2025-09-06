#include "../register.h"

#if MICROPHONE_ENABLED

void srControlTask(void *param) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t checkFrequency = pdMS_TO_TICKS(10); // Check every 10ms for responsiveness
    
    logger->info("SR Control Task started");
    
    while(1) {
        vTaskDelayUntil(&lastWakeTime, checkFrequency);
        
        if (notification->has(NOTIFICATION_SR)) {
            void* eventPtr = notification->consume(NOTIFICATION_SR, checkFrequency);
            const char* event = (const char*)eventPtr;
            
            if (event) {
                if (strcmp(event, EVENT_SR::PAUSE) == 0) {
                    logger->info("Pausing ESP-SR system");
                    esp_err_t result = SR::sr_pause();
                    if (result == ESP_OK) {
                        logger->info("ESP-SR paused successfully");
                    } else {
                        logger->error("Failed to pause ESP-SR: %s", esp_err_to_name(result));
                    }
                }
                else if (strcmp(event, EVENT_SR::RESUME) == 0) {
                    logger->info("Resuming ESP-SR system");
                    esp_err_t result = SR::sr_resume();
                    if (result == ESP_OK) {
                        logger->info("ESP-SR resumed successfully");
                    } else {
                        logger->error("Failed to resume ESP-SR: %s", esp_err_to_name(result));
                    }
                }
                else {
                    logger->debug("Unknown SR event: %s", event);
                }
            }
        }
    }
}

#endif
