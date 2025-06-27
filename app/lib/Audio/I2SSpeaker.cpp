#include "I2SSpeaker.h"
#include "MP3Decoder.h"
#include <math.h>

namespace Audio {

I2SSpeaker::I2SSpeaker(int bclkPin, int wclkPin, int dataPin, i2s_port_t i2sPort) 
    : _bclkPin(bclkPin), _wclkPin(wclkPin), _dataPin(dataPin), _i2sPort(i2sPort),
      _initialized(false), _defaultVolume(50), _sampleRate(16000), _bitsPerSample(16), _playing(false) {
}

I2SSpeaker::~I2SSpeaker() {
    if (_initialized) {
        stop();
        i2s_driver_uninstall(_i2sPort);
    }
}

bool I2SSpeaker::init(uint32_t sampleRate, int bitsPerSample) {
    if (_initialized) {
        return true; // Already initialized
    }

    _sampleRate = sampleRate;
    _bitsPerSample = bitsPerSample;

    if (!configureI2S()) {
        return false;
    }

    _initialized = true;
    return true;
}

bool I2SSpeaker::configureI2S() {
    // I2S configuration
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = _sampleRate,
        .bits_per_sample = (i2s_bits_per_sample_t)_bitsPerSample,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    // Install I2S driver
    esp_err_t err = i2s_driver_install(_i2sPort, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        return false;
    }

    // I2S pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = _bclkPin,
        .ws_io_num = _wclkPin,
        .data_out_num = _dataPin,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    err = i2s_set_pin(_i2sPort, &pin_config);
    if (err != ESP_OK) {
        i2s_driver_uninstall(_i2sPort);
        return false;
    }

    return true;
}

void I2SSpeaker::playTone(int frequency, int duration, int volume) {
    if (!_initialized) {
        return;
    }

    // Constrain frequency to audible range
    frequency = constrain(frequency, 20, 20000);
    volume = constrain(volume, 0, 100);

    // Calculate number of samples needed
    size_t samplesNeeded = (_sampleRate * duration) / 1000;
    
    // Allocate buffer for samples
    int16_t* sampleBuffer = new int16_t[samplesNeeded * 2]; // Stereo
    if (!sampleBuffer) {
        return;
    }

    // Generate sine wave
    size_t samplesGenerated = generateSineWave(frequency, duration, 0.5f, sampleBuffer, samplesNeeded * 2);

    // Play the samples
    if (samplesGenerated > 0) {
        _playing = true;
        writeSamples(sampleBuffer, samplesGenerated, volume);
        _playing = false;
    }

    delete[] sampleBuffer;
}

size_t I2SSpeaker::generateSineWave(int frequency, int duration, float amplitude, int16_t* sampleBuffer, size_t bufferSize) {
    if (!sampleBuffer || bufferSize == 0) {
        return 0;
    }

    size_t samplesNeeded = (_sampleRate * duration) / 1000;
    size_t actualSamples = min(samplesNeeded, bufferSize / 2); // Stereo

    float angularFreq = 2.0f * PI * frequency / _sampleRate;

    for (size_t i = 0; i < actualSamples; i++) {
        float sineValue = sin(angularFreq * i);
        int16_t sample = (int16_t)(sineValue * amplitude * 32767);
        
        // Stereo output (same sample for both channels)
        sampleBuffer[i * 2] = sample;     // Left channel
        sampleBuffer[i * 2 + 1] = sample; // Right channel
    }

    return actualSamples * 2; // Return total samples (stereo)
}

void I2SSpeaker::writeSamples(const int16_t* samples, size_t sampleCount, int volume) {
    if (!_initialized || !samples || sampleCount == 0) {
        return;
    }

    // Apply volume scaling
    int16_t* volumeAdjustedSamples = new int16_t[sampleCount];
    if (!volumeAdjustedSamples) {
        return;
    }

    memcpy(volumeAdjustedSamples, samples, sampleCount * sizeof(int16_t));
    applyVolume(volumeAdjustedSamples, sampleCount, volume);

    // Write to I2S
    size_t bytesWritten;
    i2s_write(_i2sPort, volumeAdjustedSamples, sampleCount * sizeof(int16_t), &bytesWritten, portMAX_DELAY);

    delete[] volumeAdjustedSamples;
}

void I2SSpeaker::applyVolume(int16_t* samples, size_t sampleCount, int volume) {
    if (!samples || sampleCount == 0) {
        return;
    }

    float volumeScale = volume / 100.0f;
    
    for (size_t i = 0; i < sampleCount; i++) {
        samples[i] = (int16_t)(samples[i] * volumeScale);
    }
}

void I2SSpeaker::playAudioData(const uint8_t* data, size_t dataSize, int volume) {
    if (!_initialized || !data || dataSize == 0) {
        return;
    }

    _playing = true;
    
    // Assuming data is already in the correct format (16-bit samples)
    const int16_t* samples = (const int16_t*)data;
    size_t sampleCount = dataSize / sizeof(int16_t);
    
    writeSamples(samples, sampleCount, volume);
    
    _playing = false;
}

void I2SSpeaker::beep(int volume) {
    playTone(1000, 200, volume);
}

void I2SSpeaker::doubleBeep(int volume) {
    playTone(1000, 150, volume);
    delay(100);
    playTone(1000, 150, volume);
}

void I2SSpeaker::playConfirmation(int volume) {
    playTone(800, 150, volume);
    delay(50);
    playTone(1200, 200, volume);
}

void I2SSpeaker::playError(int volume) {
    playTone(400, 300, volume);
    delay(100);
    playTone(300, 300, volume);
}

void I2SSpeaker::playStartup(int volume) {
    // Play startup melody: C, E, G, C (octave higher)
    playTone(523, 200, volume);  // C5
    delay(50);
    playTone(659, 200, volume);  // E5
    delay(50);
    playTone(784, 200, volume);  // G5
    delay(50);
    playTone(1047, 400, volume); // C6
}

void I2SSpeaker::playNotification(int volume) {
    playTone(1000, 100, volume);
    delay(50);
    playTone(1500, 100, volume);
    delay(50);
    playTone(1000, 100, volume);
}

void I2SSpeaker::stop() {
    if (_initialized) {
        i2s_stop(_i2sPort);
        i2s_start(_i2sPort);
        _playing = false;
    }
}

void I2SSpeaker::setVolume(int volume) {
    _defaultVolume = constrain(volume, 0, 100);
}

bool I2SSpeaker::isPlaying() {
    return _playing;
}

bool I2SSpeaker::isInitialized() const {
    return _initialized;
}

bool I2SSpeaker::setSampleRate(uint32_t sampleRate) {
    if (!_initialized) {
        return false;
    }

    _sampleRate = sampleRate;
    
    // Reconfigure I2S with new sample rate
    i2s_stop(_i2sPort);
    
    esp_err_t err = i2s_set_sample_rates(_i2sPort, sampleRate);
    if (err == ESP_OK) {
        i2s_start(_i2sPort);
        return true;
    }
    
    return false;
}

bool I2SSpeaker::playAudioFile(const String& filePath, int volume) {
    // We need access to FileManager - this will be handled by the speaker.cpp wrapper
    // For now, return false to indicate this should be called through the main speaker interface
    return false;
}

bool I2SSpeaker::playMP3File(const String& filePath, int volume) {
    if (!_initialized) {
        return false;
    }
    
    // Create MP3 decoder
    MP3Decoder decoder;
    if (!decoder.init()) {
        
        return false;
    }
    
    // Decode MP3 file to PCM data
    int16_t* pcmBuffer = nullptr;
    size_t pcmSize = 0;
    MP3Decoder::MP3Info info;
    
    if (!decoder.decodeFile(filePath, &pcmBuffer, &pcmSize, &info)) {
        return false;
    }
    
    // Check if sample rate matches our I2S configuration
    if (info.sampleRate != _sampleRate) {
        // Automatically adjust I2S sample rate to match MP3 file
        if (setSampleRate(info.sampleRate)) {
            // Successfully changed sample rate
        } else {
            // Failed to change sample rate, but continue with current rate
            // Audio may play at wrong speed/pitch
        }
    }
    
    // Play the decoded PCM data
    playAudioData((uint8_t*)pcmBuffer, pcmSize * sizeof(int16_t), volume);
    
    // Clean up
    decoder.freePCMBuffer(pcmBuffer);
    
    return true;
}

} // namespace Audio
