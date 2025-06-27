#include "PWMSpeaker.h"
#include "MP3Decoder.h"
#include <Arduino.h>
#include <driver/ledc.h>

namespace Audio {

PWMSpeaker::PWMSpeaker(int pin, int channel) 
    : _pin(pin), _channel((ledc_channel_t)channel), _timer((ledc_timer_t)channel), _initialized(false), _defaultVolume(50), _playing(false), _playEndTime(0) {
}

PWMSpeaker::~PWMSpeaker() {
    if (_initialized) {
        stop();
        // ESP-IDF doesn't have a specific detach function, just stop the channel
        ledc_stop(LEDC_LOW_SPEED_MODE, _channel, 0);
    }
}

bool PWMSpeaker::init() {
    if (_initialized) {
        return true; // Already initialized
    }

    // Validate pin
    if (_pin < 0) {
        return false;
    }

    // Configure LEDC timer
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = _timer,
        .freq_hz = 1000, // Default frequency
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false
    };
    
    esp_err_t timer_err = ledc_timer_config(&timer_config);
    if (timer_err != ESP_OK) {
        return false;
    }

    // Configure LEDC channel
    ledc_channel_config_t channel_config = {
        .gpio_num = _pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = _channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = _timer,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = {0}
    };
    
    esp_err_t channel_err = ledc_channel_config(&channel_config);
    if (channel_err != ESP_OK) {
        return false;
    }

    _initialized = true;
    return true;
}

void PWMSpeaker::playTone(int frequency, int duration, int volume) {
    if (!_initialized) {
        return;
    }

    // Constrain frequency to audible range
    frequency = constrain(frequency, 20, 20000);
    volume = constrain(volume, 0, 100);

    setPWM(frequency, volume);
    _playing = true;
    _playEndTime = millis() + duration;

    // Block for the duration
    delay(duration);
    stop();
}

void PWMSpeaker::beep(int volume) {
    playTone(1000, 200, volume);
}

void PWMSpeaker::doubleBeep(int volume) {
    playTone(1000, 150, volume);
    delay(100);
    playTone(1000, 150, volume);
}

void PWMSpeaker::playConfirmation(int volume) {
    playTone(800, 150, volume);
    delay(50);
    playTone(1200, 200, volume);
}

void PWMSpeaker::playError(int volume) {
    playTone(400, 300, volume);
    delay(100);
    playTone(300, 300, volume);
}

void PWMSpeaker::playStartup(int volume) {
    const int frequencies[] = {523, 659, 784, 1047}; // C, E, G, C (octave higher)
    const int durations[] = {200, 200, 200, 400};
    playMelody(frequencies, durations, 4, volume);
}

void PWMSpeaker::playNotification(int volume) {
    playTone(1000, 100, volume);
    delay(50);
    playTone(1500, 100, volume);
    delay(50);
    playTone(1000, 100, volume);
}

void PWMSpeaker::stop() {
    if (_initialized) {
        stopPWM();
        _playing = false;
        _playEndTime = 0;
    }
}

void PWMSpeaker::setVolume(int volume) {
    _defaultVolume = constrain(volume, 0, 100);
}

bool PWMSpeaker::isPlaying() {
    if (_playing && millis() >= _playEndTime) {
        stop();
    }
    return _playing;
}

bool PWMSpeaker::isInitialized() const {
    return _initialized;
}

void PWMSpeaker::playMelody(const int* frequencies, const int* durations, int length, int volume) {
    if (!_initialized || !frequencies || !durations || length <= 0) {
        return;
    }

    for (int i = 0; i < length; i++) {
        if (frequencies[i] > 0) {
            playTone(frequencies[i], durations[i], volume);
        } else {
            // Rest note
            delay(durations[i]);
        }
        
        // Small pause between notes
        delay(50);
    }
}

void PWMSpeaker::setPWM(int frequency, int volume) {
    if (!_initialized) {
        return;
    }

    // Set the frequency
    esp_err_t freq_err = ledc_set_freq(LEDC_LOW_SPEED_MODE, _timer, frequency);
    if (freq_err != ESP_OK) {
        return;
    }
    
    // Set duty cycle based on volume (0-255 for 8-bit resolution)
    int dutyCycle = map(volume, 0, 100, 0, 127); // Use only half range to prevent distortion
    ledc_set_duty(LEDC_LOW_SPEED_MODE, _channel, dutyCycle);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, _channel);
}

void PWMSpeaker::stopPWM() {
    if (_initialized) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, _channel, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, _channel);
    }
}

void PWMSpeaker::playAudioData(const uint8_t* data, size_t dataSize, uint32_t sampleRate, int volume) {
    if (!_initialized || !data || dataSize == 0) {
        return;
    }

    // For PWM speaker, we'll interpret the data as frequency values
    // This is a simplified approach - real audio would need more complex processing
    const uint16_t* freqData = (const uint16_t*)data;
    size_t sampleCount = dataSize / sizeof(uint16_t);
    
    _playing = true;
    
    // Calculate time per sample based on sample rate
    float timePerSample = 1000.0f / sampleRate; // milliseconds per sample
    
    for (size_t i = 0; i < sampleCount && _playing; i++) {
        int frequency = freqData[i];
        if (frequency > 0 && frequency < 20000) {
            setPWM(frequency, volume);
            delay((int)timePerSample);
        }
    }
    
    stop();
}

bool PWMSpeaker::playAudioFile(const String& filePath, int volume) {
    // We need access to FileManager - this will be handled by the speaker.cpp wrapper
    // For now, return false to indicate this should be called through the main speaker interface
    return false;
}

bool PWMSpeaker::playMP3File(const String& filePath, int volume) {
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
    
    // For PWM, we'll use a lower sample rate (8kHz) regardless of MP3 sample rate
    uint32_t playbackSampleRate = 8000;
    
    // Play the decoded PCM data
    playAudioData((uint8_t*)pcmBuffer, pcmSize * sizeof(int16_t), playbackSampleRate, volume);
    
    // Clean up
    decoder.freePCMBuffer(pcmBuffer);
    
    return true;
}

} // namespace Audio
