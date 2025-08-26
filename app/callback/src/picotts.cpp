#include "../register.h"
#include <esp_task_wdt.h>

#if PICOTTS_ENABLED

// PicoTTS output callback - called by the TTS engine with synthesized audio
void picotts_output_callback(int16_t *samples, unsigned count) {
    // Create stereo buffer (double the size for left/right channels)
    int16_t* stereo = new int16_t[count * 2];
    
    // Volume amplification factor (adjust as needed)
    const float volume_multiplier = 2.0f;
    
    // Convert mono to stereo by duplicating each sample with volume boost
    for (unsigned i = 0; i < count; i++) {
        // Amplify sample with clipping protection
        int32_t amplified = (int32_t)(samples[i] * volume_multiplier);
        
        // Clamp to int16_t range to prevent distortion
        if (amplified > 32767) amplified = 32767;
        if (amplified < -32768) amplified = -32768;
        
        int16_t boosted_sample = (int16_t)amplified;
        
        stereo[i * 2] = boosted_sample;     // Left channel
        stereo[i * 2 + 1] = boosted_sample; // Right channel
    }
    
    i2sSpeaker->writeSamples(stereo, count * 2);
    delete[] stereo;
}

// PicoTTS error callback - called when TTS encounters an error
void picotts_error_callback(void) {
    logger->error("PicoTTS engine encountered an error and stopped");
    picotts_initialized = false;
}

// PicoTTS idle callback - called when TTS engine becomes idle
void picotts_idle_callback(void) {
    logger->debug("PicoTTS engine is now idle");
    
    // Notify the management task that TTS is idle
    if (notification) {
        notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_TTS_COMPLETE);
    }
}

#endif
