#include "setup/setup.h"
#include "callback/register.h"
#include "tasks/register.h"
#include "picotts.h"

#if PICOTTS_ENABLED

// Global state
bool picotts_initialized = false;

void setupPicoTTS() {
    logger->info("Setting up PicoTTS Text-to-Speech...");
    
    #if SPEAKER_ENABLED
    
    // Ensure speaker is initialized first
    if (!i2sSpeaker) {
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

        sayText("Hi, I am cozmo. Nice to meet you.");
    } else {
        logger->error("Failed to initialize PicoTTS engine");
        picotts_initialized = false;
    }
    
    #else
    logger->warning("Cannot setup PicoTTS: Speaker disabled in configuration");
    picotts_initialized = false;
    #endif
}

// Public function to check PicoTTS status
bool isPicoTTSInitialized() {
    return picotts_initialized;
}

bool sayText(const char* text) {
    // Validate text length
    if (strlen(text) > PICOTTS_MAX_TEXT_LENGTH) {
        logger->warning("Text too long (%d chars), truncating to %d", 
                strlen(text), PICOTTS_MAX_TEXT_LENGTH);
        // Create a truncated copy of the text
        char* truncated_text = (char*)malloc(PICOTTS_MAX_TEXT_LENGTH + 1);
        strncpy(truncated_text, text, PICOTTS_MAX_TEXT_LENGTH);
        truncated_text[PICOTTS_MAX_TEXT_LENGTH] = '\0';
        text = truncated_text;
    }

    logger->info("Task says: %s", text);  // Use the text
    const size_t length = strlen(text) + 1;  // +1 for null terminator
    char arr[length];  // Allocate array large enough to hold the string
    strcpy(arr, text);  // Copy the string to 

    // Send text to PicoTTS engine using the correct API
    picotts_add(arr, sizeof(arr));

    return true;
}

#else

void setupPicoTTS() {
    logger->info("PicoTTS disabled in configuration");
}

bool isPicoTTSInitialized() {
    return false;
}

bool sayText(const char* text) {
    return false;
};

#endif
