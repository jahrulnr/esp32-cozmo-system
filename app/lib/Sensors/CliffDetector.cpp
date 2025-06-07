#include "CliffDetector.h"

namespace Sensors {

CliffDetector::CliffDetector() : _pin(-1), 
                                 _cliffDetected(false), _threshold(500), 
                                 _initialized(false) {
}

CliffDetector::~CliffDetector() {
    // Clean up resources if needed
}

bool CliffDetector::init(int pin) {
    _pin = pin;
    
    pinMode(_pin, INPUT);
    
    _initialized = true;
    return true;
}

void CliffDetector::update() {
    if (!_initialized) {
        return;
    }
    
    // Read sensor values
    int value = analogRead(_pin);
    
    // Detect cliffs based on thresholds
    _cliffDetected = value < _threshold;
}

bool CliffDetector::isCliffDetected() const {
    return _cliffDetected;
}

bool CliffDetector::calibrate() {
    if (!_initialized) {
        return false;
    }
    
    // Calibration procedure
    const int samples = 100;
    long sumValue = 0;
    
    // Take multiple readings and average them
    for (int i = 0; i < samples; i++) {
        sumValue += analogRead(_pin);
        delay(10);
    }
    
    int avgValue = sumValue / samples;
    
    // Set thresholds based on average values (e.g., 80% of average)
    _threshold = avgValue * 0.8;
    
    return true;
}

} // namespace Sensors
