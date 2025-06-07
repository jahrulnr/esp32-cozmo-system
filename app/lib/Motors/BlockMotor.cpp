#include "BlockMotor.h"

namespace Motors {

BlockMotor::BlockMotor() : _currentPosition(DOWN), _liftPin(-1), _initialized(false) {
}

BlockMotor::~BlockMotor() {
    if (_initialized) {
        _liftServo.detach();
    }
}

bool BlockMotor::init(int liftPin) {
    _liftPin = liftPin;
    
    // Initialize ESP32 servo library if not already done
    ESP32PWM::allocateTimer(0);
    
    // Attach servo to pin
    _liftServo.setPeriodHertz(50);  // Standard 50hz servo
    _liftServo.attach(_liftPin, 500, 2400);
    
    // Start in down position
    moveToPosition(DOWN);
    
    _initialized = true;
    return true;
}

void BlockMotor::moveToPosition(BlockPosition position) {
    if (!_initialized) {
        return;
    }
    
    _currentPosition = position;
    
    switch (position) {
        case DOWN:
            _liftServo.write(DOWN_ANGLE);
            break;
            
        case HOLD:
            _liftServo.write(HOLD_ANGLE);
            break;
            
        case UP:
            _liftServo.write(UP_ANGLE);
            break;
    }
}

BlockMotor::BlockPosition BlockMotor::getCurrentPosition() const {
    return _currentPosition;
}

} // namespace Motors
