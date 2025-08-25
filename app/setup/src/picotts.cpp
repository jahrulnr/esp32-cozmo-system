#include "setup/setup.h"
#include "callback/register.h"
#include "tasks/register.h"
#include "picotts.h"

#if PICOTTS_ENABLED

static const char* TAG = "PicoTTS_Setup";

// Global state
bool picotts_initialized = false;

// Forward declarations
static void picotts_output_callback(int16_t *samples, unsigned count);
static void picotts_error_callback(void);
static void picotts_idle_callback(void);

void setupPicoTTS() {
    logger->info("Setting up PicoTTS Text-to-Speech...");
    
    #if SPEAKER_ENABLED
    
    // Ensure speaker is initialized first
    if (!i2sSpeaker || !getSpeakerStatus()) {
        logger->error("Cannot setup PicoTTS: I2S speaker not initialized");
        return;
    }
    
    logger->info("Initializing PicoTTS engine...");
    
    // Initialize PicoTTS using the correct API signature
    if (picotts_init(PICOTTS_TASK_PRIORITY, picotts_output_callback, PICOTTS_CORE)) {
        picotts_initialized = true;
        logger->info("PicoTTS initialized successfully!");
        
        // Set error and idle callbacks
        picotts_set_error_notify(picotts_error_callback);
        picotts_set_idle_notify(picotts_idle_callback);
        
        // Create PicoTTS management task
        BaseType_t task_result = xTaskCreate(
            picoTTSTask,                    // Task function
            "PicoTTS Task",                 // Task name
            4096,                           // Stack size
            nullptr,                        // Parameters
            PICOTTS_TASK_PRIORITY + 1,      // Priority (higher than TTS engine)
            &picoTTSTaskHandle              // Task handle
            // &picoTTSTaskHandle,             // Task handle
            // PICOTTS_CORE                    // Core
        );
        
        if (task_result == pdPASS) {
            logger->info("PicoTTS management task created successfully on core %d", PICOTTS_CORE);
            
            // Test TTS with a greeting
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for task to initialize
            sayTextUrgent("PicoTTS system ready");
            
        } else {
            logger->error("Failed to create PicoTTS management task");
            picotts_shutdown();
            picotts_initialized = false;
        }
        
    } else {
        logger->error("Failed to initialize PicoTTS engine");
        picotts_initialized = false;
    }
    
    #else
    logger->warning("Cannot setup PicoTTS: Speaker disabled in configuration");
    picotts_initialized = false;
    #endif
}

// PicoTTS output callback - called by the TTS engine with synthesized audio
static void picotts_output_callback(int16_t *samples, unsigned count) {    
    // Send synthesized audio to the speaker
    i2sSpeaker->writeSamples(samples, count * sizeof(int16_t));
}

// PicoTTS error callback - called when TTS encounters an error
static void picotts_error_callback(void) {
    logger->error("PicoTTS engine encountered an error and stopped");
    picotts_initialized = false;
    
    // Note: This is called from the TTS task, so we can't do heavy operations here
    // The management task should monitor picotts_initialized and handle restart
}

// PicoTTS idle callback - called when TTS engine becomes idle
static void picotts_idle_callback(void) {
    ESP_LOGD(TAG, "PicoTTS engine is now idle");
    
    // Notify the management task that TTS is idle
    if (notification) {
        notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_TTS_COMPLETE);
    }
}

// Public function to check PicoTTS status
bool isPicoTTSInitialized() {
    return picotts_initialized;
}

// Public function to get PicoTTS info
String getPicoTTSInfo() {
    String info = "PicoTTS Status: ";
    info += picotts_initialized ? "Initialized" : "Not Initialized";
    info += ", Task: ";
    info += (picoTTSTaskHandle != nullptr) ? "Running" : "Stopped";
    info += ", Queue: " + String(getTTSQueueSize()) + " items";
    return info;
}

#else

void setupPicoTTS() {
    logger->info("PicoTTS disabled in configuration");
}

bool isPicoTTSInitialized() {
    return false;
}

String getPicoTTSInfo() {
    return "PicoTTS: Disabled";
}

#endif
