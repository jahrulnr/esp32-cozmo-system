#include "ServoControl.h"

namespace Motors {

ServoControl::ServoControl() : _headAngle(90), _handAngle(90),
                              _headServoPin(-1), _handServoPin(-1),
                              _initialized(false) {
}

ServoControl::~ServoControl() {
    // Clean up resources if needed
    if (_initialized) {
        _headServo.detach();
        _handServo.detach();
    }
}

bool ServoControl::init(int headServoPin, int handServoPin) {
    _headServoPin = headServoPin;
    _handServoPin = handServoPin;

    // Initialize ESP32 servo library
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // Attach servos to pins
    _headServo.setPeriodHertz(50);    // Standard 50hz servo
    _handServo.setPeriodHertz(50);   // Standard 50hz servo
    
    _headServo.attach(_headServoPin, 500, 2400);
    _handServo.attach(_handServoPin, 500, 2400);
    
    // Set initial positions
    setHead(_headAngle);
    setHand(_handAngle);
    
    _initialized = true;
    return true;
}

void ServoControl::setHead(int angle) {
    if (!_initialized) {
        return;
    }
    
    // Constrain angle to valid range
    _headAngle = constrain(angle, 0, 180);
    _headServo.write(_headAngle);
}

void ServoControl::setHand(int angle) {
    if (!_initialized) {
        return;
    }
    
    // Constrain angle to valid range
    _handAngle = constrain(angle, 0, 180);
    _handServo.write(_handAngle);
}

int ServoControl::getHead() const {
    return _headAngle;
}

int ServoControl::getHand() const {
    return _handAngle;
}

} // namespace Motors
