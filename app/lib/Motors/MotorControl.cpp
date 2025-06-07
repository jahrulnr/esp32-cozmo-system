#include "MotorControl.h"

namespace Motors {

MotorControl::MotorControl() : _leftMotorPin1(-1), _leftMotorPin2(-1),
                               _rightMotorPin1(-1), _rightMotorPin2(-1),
                               _speed(200), _currentDirection(STOP), _initialized(false) {
}

MotorControl::~MotorControl() {
    stop();
}

bool MotorControl::init(int leftMotorPin1, int leftMotorPin2, int rightMotorPin1, int rightMotorPin2) {
    _leftMotorPin1 = leftMotorPin1;
    _leftMotorPin2 = leftMotorPin2;
    _rightMotorPin1 = rightMotorPin1;
    _rightMotorPin2 = rightMotorPin2;

    pinMode(_leftMotorPin1, OUTPUT);
    pinMode(_leftMotorPin2, OUTPUT);
    pinMode(_rightMotorPin1, OUTPUT);
    pinMode(_rightMotorPin2, OUTPUT);

    // Start with motors stopped
    stop();

    _initialized = true;
    return true;
}

void MotorControl::setSpeed(uint8_t speed) {
    _speed = speed;
}

void MotorControl::move(Direction direction, unsigned long duration) {
    if (!_initialized) {
        return;
    }

    _currentDirection = direction;

    // switch (direction) {
    //     case FORWARD:
    //         analogWrite(_leftMotorPin1, _speed);
    //         analogWrite(_leftMotorPin2, 0);
    //         analogWrite(_rightMotorPin1, _speed);
    //         analogWrite(_rightMotorPin2, 0);
    //         break;
            
    //     case BACKWARD:
    //         analogWrite(_leftMotorPin1, 0);
    //         analogWrite(_leftMotorPin2, _speed);
    //         analogWrite(_rightMotorPin1, 0);
    //         analogWrite(_rightMotorPin2, _speed);
    //         break;
            
    //     case LEFT:
    //         analogWrite(_leftMotorPin1, 0);
    //         analogWrite(_leftMotorPin2, _speed);
    //         analogWrite(_rightMotorPin1, _speed);
    //         analogWrite(_rightMotorPin2, 0);
    //         break;
            
    //     case RIGHT:
    //         analogWrite(_leftMotorPin1, _speed);
    //         analogWrite(_leftMotorPin2, 0);
    //         analogWrite(_rightMotorPin1, 0);
    //         analogWrite(_rightMotorPin2, _speed);
    //         break;
            
    //     case STOP:
    //     default:
    //         stop();
    //         break;
    // }

    switch (direction) {
        case FORWARD:
            digitalWrite(_leftMotorPin1, HIGH);
            digitalWrite(_leftMotorPin2, LOW);
            digitalWrite(_rightMotorPin1, HIGH);
            digitalWrite(_rightMotorPin2, LOW);
            break;
            
        case BACKWARD:
            digitalWrite(_leftMotorPin1, LOW);
            digitalWrite(_leftMotorPin2, HIGH);
            digitalWrite(_rightMotorPin1, LOW);
            digitalWrite(_rightMotorPin2, HIGH);
            break;
            
        case LEFT:
            digitalWrite(_leftMotorPin1, LOW);
            digitalWrite(_leftMotorPin2, HIGH);
            digitalWrite(_rightMotorPin1, HIGH);
            digitalWrite(_rightMotorPin2, LOW);
            break;
            
        case RIGHT:
            digitalWrite(_leftMotorPin1, HIGH);
            digitalWrite(_leftMotorPin2, LOW);
            digitalWrite(_rightMotorPin1, LOW);
            digitalWrite(_rightMotorPin2, HIGH);
            break;
            
        case STOP:
        default:
            stop();
            break;
    }

    // If duration is specified, stop after the given time
    if (duration > 0) {
        delay(duration);
        stop();
    }
}

void MotorControl::stop() {
    if (!_initialized) {
        return;
    }
    
    digitalWrite(_leftMotorPin1, LOW);
    digitalWrite(_leftMotorPin2, LOW);
    digitalWrite(_rightMotorPin1, LOW);
    digitalWrite(_rightMotorPin2, LOW);
    
    _currentDirection = STOP;
}

MotorControl::Direction MotorControl::getCurrentDirection() const {
    return _currentDirection;
}

} // namespace Motors
