#include "../register.h"

void notePlayerTask(void* param) {
    if (!notePlayer || !notification) {
        logger->error("Note task: Note system or notification not initialized");
        vTaskDelete(NULL);
        return;
    }

    logger->info("Note task started");
    
    const TickType_t checkFrequency = pdMS_TO_TICKS(50); // Check every 50ms

    while (true) {
        void* eventPtr = notification->consume(NOTIFICATION_NOTE, checkFrequency);
        
        if (eventPtr) {
            const char* event = (const char*)eventPtr;
            logger->info("Note task received event: %s", event);
            
            // Call the callback function to handle the event
            callbackNotePlayer(eventPtr);
        }
        
        // Small delay to prevent watchdog issues
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
