#include "../register.h"

#if MICROPHONE_ENABLED

float mic_volume_multiplier = 1.0f;

// Analog fill callback for ESP-SR system
esp_err_t sr_fill_callback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms) {
    #if MICROPHONE_I2S
    if (microphone){
        if (!microphone->isActive()) microphone->start();
        return microphone->readAudioData(out, len, bytes_read);
    }
    #elif MICROPHONE_ANALOG
    // Calculate how many 16-bit samples we need
    int samples_needed = len / sizeof(int16_t);
    int samples_read = 0;
    if (amicrophone) {
        if (!amicrophone->isActive()) amicrophone->start();
        samples_read = amicrophone->readSamples((int16_t*)out, samples_needed, timeout_ms);
    }
    
    if (samples_read > 0) {
        if (mic_volume_multiplier != 1.0f) {
            int16_t* samples = (int16_t*)out;
            for (int i = 0; i < samples_read; i++) {
                int32_t adjusted = (int32_t)(samples[i] * mic_volume_multiplier);
                // Clamp to 16-bit range
                if (adjusted > 32767) adjusted = 32767;
                else if (adjusted < -32768) adjusted = -32768;
                samples[i] = (int16_t)adjusted;
            }
        }
        
        *bytes_read = samples_read * sizeof(int16_t);
        return ESP_OK;
    }
    
    *bytes_read = 0;
    #endif
    return ESP_FAIL;
}
#endif