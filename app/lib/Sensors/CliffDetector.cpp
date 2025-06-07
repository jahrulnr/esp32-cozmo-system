#include "CliffDetector.h"

namespace Sensors {

CliffDetector::CliffDetector() : _pin(-1), 
                                 _cliffDetected(false), 
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
    // Use digitalRead: 1 means cliff detected
    int value = digitalRead(_pin);
    _cliffDetected = (value == HIGH);
}

bool CliffDetector::isCliffDetected() {
    update();
    return _cliffDetected;
}

bool CliffDetector::calibrate() {
    if (!_initialized) {
        return false;
    }
    
    return true;
}

} // namespace Sensors
