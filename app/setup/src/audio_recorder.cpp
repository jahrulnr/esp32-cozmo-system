#include <Arduino.h>
#include "setup/setup.h"

// Global variable for audio recorder
AudioRecorder* audioRecorder = nullptr;

// Initialize audio recorder
void setupAudioRecorder() {
    #if AUDIO_RECORDING_ENABLED
    if (audioRecorder == nullptr) {
        audioRecorder = new AudioRecorder(fileManager, logger, notification, mic_fill_callback);
        
        if (audioRecorder) {
            logger->info("AudioRecorder setup complete");
        } else {
            logger->error("AudioRecorder initialization failed");
        }
    }
    #endif
}
