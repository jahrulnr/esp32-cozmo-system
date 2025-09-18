#include "../register.h"

#if !MICROPHONE_ENABLED
esp_err_t mic_fill_callback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms) {
    return ESP_ERR_NOT_SUPPORTED;
}
#else

// fill callback for ESP-SR system
esp_err_t mic_fill_callback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms) {
    float mic_volume_multiplier = 1.f;
    if (arg) {
        float* volumeArg = (float*)arg;
        float volume = *volumeArg;
        // Validate the pointer and value before using
        if (volume >= 1.f && volume <= 3.f) {
            mic_volume_multiplier = volume;
        }
    }

    #if MICROPHONE_I2S
    if (!microphone)
        return ESP_ERR_INVALID_STATE;
    if (!microphone->isActive()) microphone->start();
    esp_err_t ret = microphone->readAudioData(out, len, bytes_read);
    if (ret != ESP_OK)
        return ret;

    if (mic_volume_multiplier == 1.0f) {
        return ret;
    }

    int16_t* samples = (int16_t*)out;
    int sample_count = len / sizeof(int16_t);
    for (int i = 0; i < sample_count; i++) {
        int32_t adjusted = (int32_t)(samples[i] * mic_volume_multiplier);
        if (adjusted > 32767) adjusted = 32767;
        else if (adjusted < -32768) adjusted = -32768;
        samples[i] = (int16_t)adjusted;
    }

    out = samples;
    return ret;
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