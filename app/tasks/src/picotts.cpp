#include "../register.h"

#if PICOTTS_ENABLED

TaskHandle_t picoTTSTaskHandle = nullptr;

// TTS request queue structure
struct TTSRequest {
    const char* text;
    int priority;
    bool urgent;
};

// Queue for TTS requests
static QueueHandle_t ttsQueue = nullptr;
static const int TTS_QUEUE_SIZE = PICOTTS_QUEUE_SIZE;

static const char* TAG = "PicoTTS_Task";

void picoTTSTask(void* param) {
    logger->info("PicoTTS management task started");
    
    // Create TTS request queue
    ttsQueue = xQueueCreate(TTS_QUEUE_SIZE, sizeof(TTSRequest));
    if (!ttsQueue) {
        logger->error("Failed to create TTS queue");
        vTaskDelete(nullptr);
        return;
    }
    
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    // Wait for PicoTTS to be initialized
    while (!picotts_initialized) {
        logger->info("Waiting for PicoTTS initialization...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    logger->info("PicoTTS system ready, management task started");
    
    while (1) {
        TTSRequest request;
        
        // Wait for TTS requests
        if (xQueueReceive(ttsQueue, &request, pdMS_TO_TICKS(100)) == pdTRUE) {
            logger->info("Processing TTS request: '%s' (priority: %d)", 
                    request.text, request.priority);
            
            // Validate text length
						if (strlen(request.text) > PICOTTS_MAX_TEXT_LENGTH) {
							logger->warning("Text too long (%d chars), truncating to %d", 
									strlen(request.text), PICOTTS_MAX_TEXT_LENGTH);
							// Create a truncated copy of the text
							char* truncated_text = (char*)malloc(PICOTTS_MAX_TEXT_LENGTH + 1);
							strncpy(truncated_text, request.text, PICOTTS_MAX_TEXT_LENGTH);
							truncated_text[PICOTTS_MAX_TEXT_LENGTH] = '\0';
							request.text = truncated_text;
						}
            
            // Send text to PicoTTS engine using the correct API
            picotts_add(request.text, strlen(request.text));
            logger->info("Text '%s' sent to PicoTTS engine", request.text);
            
            // Notify display that TTS is active
            if (notification) {
                notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_TTS_ACTIVE);
            }
        }
        
        // Monitor system health and handle errors
        static int health_counter = 0;
        if (++health_counter % 100 == 0) {  // Every 10 seconds (100 * 100ms)
            int free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
            logger->info("TTS Task Health - Free Heap: %d, Queue items: %d, Engine: %s", 
                    free_heap, uxQueueMessagesWaiting(ttsQueue),
                    picotts_initialized ? "OK" : "ERROR");
            
            // Handle engine errors by attempting restart
            if (!picotts_initialized) {
                logger->warning("PicoTTS engine error detected, attempting restart...");
                
                // Clear the queue
                TTSRequest dummy;
                while (xQueueReceive(ttsQueue, &dummy, 0) == pdTRUE) {
                    // Drain queue
                }
                
                // Attempt to restart PicoTTS
                picotts_shutdown();
                vTaskDelay(pdMS_TO_TICKS(1000));
                
                // Note: We can't restart here because we don't have access to the callback
                // The main setup function should monitor picotts_initialized and handle restart
                logger->error("PicoTTS restart requires setupPicoTTS() to be called again");
            }
        }
        
        taskYIELD(); // Allow other tasks to run
    }
}

// Public function to queue TTS requests
bool queueTTSRequest(const String& text, int priority, bool urgent) {
    if (!ttsQueue || text.isEmpty()) {
        logger->warning("Cannot queue TTS - queue not ready or empty text");
        return false;
    }
    
    TTSRequest request;
    request.text = text.c_str() + '\0';
    request.priority = priority;
    request.urgent = urgent;
    
    TickType_t wait_time = urgent ? 0 : pdMS_TO_TICKS(10);
    
    if (xQueueSend(ttsQueue, &request, wait_time) == pdTRUE) {
        logger->info("TTS request queued: '%s'", text.c_str());
        return true;
    } else {
        logger->warning("TTS queue full, request dropped: '%s'", text.c_str());
        return false;
    }
}

// Convenience functions
bool sayText(const String& text) {
    return queueTTSRequest(text, 19, false);
}

bool sayTextUrgent(const String& text) {
    return queueTTSRequest(text, 10, true);
}

// Get queue status
int getTTSQueueSize() {
    return ttsQueue ? uxQueueMessagesWaiting(ttsQueue) : 0;
}

bool isTTSQueueFull() {
    return ttsQueue ? (uxQueueSpacesAvailable(ttsQueue) == 0) : true;
}

#endif
