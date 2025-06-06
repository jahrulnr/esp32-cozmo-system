#include "DistanceSensor.h"

namespace Sensors {

DistanceSensor::DistanceSensor() : _triggerPin(-1), _echoPin(-1),
                                 _maxDistance(400), _threshold(20.0),
																 _timeout(0),
                                 _initialized(false) {
}

DistanceSensor::~DistanceSensor() {
    // Release resources if needed
    if (_initialized) {
        // Nothing to clean up specifically for HC-SR04
    }
}

bool DistanceSensor::init(int triggerPin, int echoPin, int maxDistance) {
    _triggerPin = triggerPin;
    _echoPin = echoPin;
    _maxDistance = maxDistance;
    
    // Calculate the timeout based on the maximum distance
    // Sound travels at ~343m/s or ~34.3cm/ms in air at 20°C
    // For round trip (echo), we need to wait for double the time
    // Plus a small buffer for sensor response time
    _timeout = (unsigned long)((_maxDistance * 2.0) / 0.0343) + 1000; // microseconds
    
    // Configure pins
    pinMode(_triggerPin, OUTPUT);
    pinMode(_echoPin, INPUT);
    
    // Initialize TRIG pin to LOW
    digitalWrite(_triggerPin, LOW);
    delay(50); // Allow sensor to stabilize
    
    _initialized = true;
    return true;
}

void DistanceSensor::setThresHold(float threshold) {
	_threshold = threshold;
}

float DistanceSensor::measureDistance() {
    if (!_initialized) {
        return -1.0;
    }
    
    // Send a 10μs pulse on the TRIG pin
    digitalWrite(_triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_triggerPin, LOW);
    
    // Measure the duration of the pulse on the ECHO pin
    unsigned long duration = pulseIn(_echoPin, HIGH, _timeout);
    
    // Calculate the distance in centimeters
    // Speed of sound is ~343m/s or ~0.0343cm/μs
    // Distance = (Duration / 2) * 0.0343
    float distance = (duration / 2.0) * 0.0343;
    
    // Check if the measurement is valid
    if (duration == 0 || distance > _maxDistance) {
        return -1.0; // Timeout or out of range
    }
    
    return distance;
}

bool DistanceSensor::isObstacleDetected() {
    float distance = measureDistance();
    return (distance > 0 && distance < _threshold);
}

} // namespace Sensors
