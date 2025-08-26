#include "../register.h"
#include <esp_task_wdt.h>

#if PICOTTS_ENABLED

// Speed control factor (1.0 = normal, >1.0 = faster, <1.0 = slower)
static float playback_speed = 1.5f;
// Volume amplification factor (adjust as needed)
const float volume_multiplier = 1.5f;

// Buffer to collect audio samples
static std::vector<int16_t> collected_audio;

// Function to set playback speed
void picotts_set_speed(float speed) {
    playback_speed = speed;
}

// Function to apply speed adjustment to audio samples
std::vector<int16_t> apply_speed_adjustment(const std::vector<int16_t>& samples) {
    if (playback_speed == 1.0f) {
        return samples; // No speed change needed
    }
    
    std::vector<int16_t> adjusted_samples;
    size_t output_size = (size_t)(samples.size() / playback_speed);
    adjusted_samples.reserve(output_size);
    
    for (size_t i = 0; i < output_size; i += 2) { // Process stereo pairs
        float source_index = i * playback_speed;
        size_t index = (size_t)source_index;
        
        if (index + 1 < samples.size()) {
            adjusted_samples.push_back(samples[index]);     // Left channel
            adjusted_samples.push_back(samples[index + 1]); // Right channel
        }
    }
    
    return adjusted_samples;
}

// PicoTTS output callback - called by the TTS engine with synthesized audio
void picotts_output_callback(int16_t *samples, unsigned count) {
    
    // Convert mono to stereo and collect samples
    for (unsigned i = 0; i < count; i++) {
        // Amplify sample with clipping protection
        int32_t amplified = (int32_t)(samples[i] * volume_multiplier);
        
        // Clamp to int16_t range to prevent distortion
        if (amplified > 32767) amplified = 32767;
        if (amplified < -32768) amplified = -32768;
        
        int16_t boosted_sample = (int16_t)amplified;
        
        // Add stereo samples to collection buffer
        collected_audio.push_back(boosted_sample);     // Left channel
        collected_audio.push_back(boosted_sample);     // Right channel
    }
}

// PicoTTS error callback - called when TTS encounters an error
void picotts_error_callback(void) {
    logger->error("PicoTTS engine encountered an error and stopped");
    picotts_initialized = false;
}

// PicoTTS idle callback - called when TTS engine becomes idle
void picotts_idle_callback(void) {
    logger->debug("PicoTTS engine is now idle");
    
    // Play collected audio samples to speaker with speed adjustment
    if (!collected_audio.empty() && i2sSpeaker) {
        std::vector<int16_t> speed_adjusted_audio = apply_speed_adjustment(collected_audio);
        i2sSpeaker->writeSamples(speed_adjusted_audio.data(), speed_adjusted_audio.size());
        logger->debug("Played speed-adjusted audio samples to speaker (speed: %.2f)", playback_speed);
        
        // Clear the buffer for next use
        collected_audio.clear();
    }
    
    // Notify the management task that TTS is idle
    if (notification) {
        notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_TTS_COMPLETE);
    }
}

#endif
