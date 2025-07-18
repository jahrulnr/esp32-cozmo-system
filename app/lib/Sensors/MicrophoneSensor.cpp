#include "MicrophoneSensor.h"

namespace Sensors {

MicrophoneSensor::MicrophoneSensor(int analogPin, int gainPin, int attackReleasePin) 
    : _analogPin(analogPin), _gainPin(gainPin), _attackReleasePin(attackReleasePin), 
      _initialized(false), _baselineLevel(0) {
    _mutex = xSemaphoreCreateMutex();
}

MicrophoneSensor::~MicrophoneSensor() {
    // No special cleanup needed for analog pins
}

bool MicrophoneSensor::init() {
    if (_initialized) {
        return true; // Already initialized
    }

    // Validate analog pin
    if (_analogPin < 0) {
        return false;
    }

    // Set up gain control pin if specified
    if (_gainPin >= 0) {
        pinMode(_gainPin, OUTPUT);
        digitalWrite(_gainPin, LOW); // Default to 40dB gain
    }

    // Set up attack/release control pin if specified
    if (_attackReleasePin >= 0) {
        pinMode(_attackReleasePin, OUTPUT);
        digitalWrite(_attackReleasePin, LOW); // Default to fast attack/release
    }

    // Set ADC resolution to 12 bits for better precision
    analogReadResolution(12);
    
    // Allow some time for the MAX9814 to stabilize
    delay(100);

    // Calibrate baseline noise level
    _baselineLevel = calibrateBaseline(500);

    _initialized = true;
    return true;
}

int MicrophoneSensor::readLevel() {
    if (!_initialized) {
        return -1;
    }
    
    xSemaphoreTake(_mutex, portMAX_DELAY);
    int value = analogRead(_analogPin);
    xSemaphoreGive(_mutex);
    return value;
}

int MicrophoneSensor::readPeakLevel(int durationMs) {
    if (!_initialized) {
        return -1;
    }

    int peakLevel = 0;
    unsigned long startTime = millis();
    
    while (millis() - startTime < durationMs) {
        int currentLevel = readLevel();
        if (currentLevel > peakLevel) {
            peakLevel = currentLevel;
        }
        delayMicroseconds(100); // Small delay to prevent overwhelming the ADC
    }

    return peakLevel;
}

int MicrophoneSensor::readAverageLevel(int durationMs) {
    if (!_initialized) {
        return -1;
    }

    long totalLevel = 0;
    int sampleCount = 0;
    unsigned long startTime = millis();
    
    while (millis() - startTime < durationMs) {
        totalLevel += readLevel();
        sampleCount++;
        delayMicroseconds(100); // Small delay to prevent overwhelming the ADC
    }

    return sampleCount > 0 ? (int)(totalLevel / sampleCount) : 0;
}

bool MicrophoneSensor::isSoundDetected(int threshold) {
    if (!_initialized) {
        return false;
    }

    int currentLevel = readLevel();
    return (currentLevel - _baselineLevel) > threshold;
}

void MicrophoneSensor::setGain(int gainLevel) {
    if (_gainPin >= 0) {
        if (gainLevel == LOW) {
            digitalWrite(_gainPin, LOW);  // 40dB gain
        } else if (gainLevel == HIGH) {
            digitalWrite(_gainPin, HIGH); // 50dB gain
        } else {
            // For 60dB gain, set pin as input (floating)
            pinMode(_gainPin, INPUT);
        }
    }
}

void MicrophoneSensor::setAttackRelease(bool attackRelease) {
    if (_attackReleasePin >= 0) {
        digitalWrite(_attackReleasePin, attackRelease ? HIGH : LOW);
    }
}

int MicrophoneSensor::calibrateBaseline(int samplingTime) {
    if (!_initialized && samplingTime <= 0) {
        return 0;
    }

    long totalLevel = 0;
    int sampleCount = 0;
    unsigned long startTime = millis();
    
    // Sample the environment noise for the specified time
    while (millis() - startTime < samplingTime) {
        totalLevel += analogRead(_analogPin);
        sampleCount++;
        delay(5); // 5ms delay between samples
    }

    int baseline = sampleCount > 0 ? (int)(totalLevel / sampleCount) : 0;
    
    if (_initialized) {
        _baselineLevel = baseline;
    }
    
    return baseline;
}

bool MicrophoneSensor::isInitialized() const {
    return _initialized;
}

int* MicrophoneSensor::readSamples(int samples, int delayMs) {
    if (!_initialized || samples <= 0) {
        return nullptr;
    }

    int* sampleArray = new int[samples];
    
    for (int i = 0; i < samples; i++) {
        sampleArray[i] = readLevel();
        if (delayMs > 0) {
            delay(delayMs);
        }
    }

    return sampleArray;
}

} // namespace Sensors
