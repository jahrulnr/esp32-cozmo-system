#include "CliffDetector.h"

namespace Sensors {

CliffDetector::CliffDetector() : _leftPin(-1), _rightPin(-1), _leftCliffDetected(false), 
                                 _rightCliffDetected(false), _leftThreshold(500), _rightThreshold(500), 
                                 _initialized(false) {
}

CliffDetector::~CliffDetector() {
    // Clean up resources if needed
}

bool CliffDetector::init(int leftPin, int rightPin) {
    _leftPin = leftPin;
    _rightPin = rightPin;
    
    pinMode(_leftPin, INPUT);
    pinMode(_rightPin, INPUT);
    
    _initialized = true;
    return true;
}

void CliffDetector::update() {
    if (!_initialized) {
        return;
    }
    
    // Read sensor values
    int leftValue = analogRead(_leftPin);
    int rightValue = analogRead(_rightPin);
    
    // Detect cliffs based on thresholds
    _leftCliffDetected = leftValue < _leftThreshold;
    _rightCliffDetected = rightValue < _rightThreshold;
}

bool CliffDetector::isLeftCliffDetected() const {
    return _leftCliffDetected;
}

bool CliffDetector::isRightCliffDetected() const {
    return _rightCliffDetected;
}

bool CliffDetector::isAnyCliffDetected() const {
    return _leftCliffDetected || _rightCliffDetected;
}

bool CliffDetector::calibrate() {
    if (!_initialized) {
        return false;
    }
    
    // Calibration procedure
    const int samples = 100;
    long sumLeft = 0, sumRight = 0;
    
    // Take multiple readings and average them
    for (int i = 0; i < samples; i++) {
        sumLeft += analogRead(_leftPin);
        sumRight += analogRead(_rightPin);
        delay(10);
    }
    
    int avgLeft = sumLeft / samples;
    int avgRight = sumRight / samples;
    
    // Set thresholds based on average values (e.g., 80% of average)
    _leftThreshold = avgLeft * 0.8;
    _rightThreshold = avgRight * 0.8;
    
    return true;
}

} // namespace Sensors
