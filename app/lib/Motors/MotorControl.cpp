#include "MotorControl.h"

namespace Motors {

MotorControl::MotorControl() : _leftMotorPin1(-1), _leftMotorPin2(-1),
                               _rightMotorPin1(-1), _rightMotorPin2(-1),
                               _currentDirection(STOP), _initialized(false),
                               _screen(nullptr), _interrupt(false) {
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

void MotorControl::setScreen(Screen::Screen *screen) {
    _screen = screen;
}

void MotorControl::moveLook(MotorControl::Direction direction) {
    if (_screen && _screen->getFace()) {
        switch (direction) {
            case FORWARD:
            case BACKWARD:
            case STOP:
            default:
                _screen->getFace()->LookFront();
                break;
                
            case LEFT:
                _screen->getFace()->LookLeft();
                break;
                
            case RIGHT:
                _screen->getFace()->LookRight();
                break;
        }
    }
}

void MotorControl::move(Direction direction, unsigned long duration) {
    if (!_initialized) {
        return;
    }

    _currentDirection = direction;
    _interrupt = false;

    moveLook(direction);
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
        for(int i = 0; i <= duration; i+=5) {
            if (isInterrupt()) {
                stop();
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        stop();
    }
}

void MotorControl::stop() {
    if (!_initialized) {
        return;
    }
    
    moveLook(STOP);
    digitalWrite(_leftMotorPin1, LOW);
    digitalWrite(_leftMotorPin2, LOW);
    digitalWrite(_rightMotorPin1, LOW);
    digitalWrite(_rightMotorPin2, LOW);
    
    _currentDirection = STOP;
}

MotorControl::Direction MotorControl::getCurrentDirection() const {
    return _currentDirection;
}

void MotorControl::interuptMotor() {
    if (_currentDirection != STOP) return;
    _interrupt = true;
}

bool MotorControl::isInterrupt() {
    return _interrupt;
}

} // namespace Motors
