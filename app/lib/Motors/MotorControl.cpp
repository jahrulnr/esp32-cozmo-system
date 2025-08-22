#include "MotorControl.h"
#include "Logger.h"

namespace Motors {

MotorControl::MotorControl() : _leftMotorPin1(-1), _leftMotorPin2(-1),
                               _rightMotorPin1(-1), _rightMotorPin2(-1),
                               _currentDirection(STOP), _initialized(false),
                               _useIoExtender(false), _ioExtender(nullptr),
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
    
    // Use direct GPIO pins
    _useIoExtender = false;
    _ioExtender = nullptr;

    pinMode(_leftMotorPin1, OUTPUT);
    pinMode(_leftMotorPin2, OUTPUT);
    pinMode(_rightMotorPin1, OUTPUT);
    pinMode(_rightMotorPin2, OUTPUT);

    // Start with motors stopped
    stop();

    _initialized = true;
    Utils::Logger::getInstance().info("MotorControl: Initialized with direct GPIO pins");
    return true;
}

bool MotorControl::initWithExtender(Utils::IOExtern* ioExtender, int leftMotorPin1, int leftMotorPin2,
                                int rightMotorPin1, int rightMotorPin2) {
    if (!ioExtender) {
        Utils::Logger::getInstance().error("MotorControl: Invalid I/O extender provided");
        return false;
    }
    
    _ioExtender = ioExtender;
    _useIoExtender = true;
    _leftMotorPin1 = leftMotorPin1;
    _leftMotorPin2 = leftMotorPin2;
    _rightMotorPin1 = rightMotorPin1;
    _rightMotorPin2 = rightMotorPin2;

    // No need to call pinMode for I/O extender pins
    
    // Start with motors stopped
    stop();

    _initialized = true;
    Utils::Logger::getInstance().info("MotorControl: Initialized with I/O extender");
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
            setMotorPin(_leftMotorPin1, HIGH);
            setMotorPin(_leftMotorPin2, LOW);
            setMotorPin(_rightMotorPin1, HIGH);
            setMotorPin(_rightMotorPin2, LOW);
            break;
            
        case BACKWARD:
            setMotorPin(_leftMotorPin1, LOW);
            setMotorPin(_leftMotorPin2, HIGH);
            setMotorPin(_rightMotorPin1, LOW);
            setMotorPin(_rightMotorPin2, HIGH);
            break;
            
        case LEFT:
            setMotorPin(_leftMotorPin1, LOW);
            setMotorPin(_leftMotorPin2, HIGH);
            setMotorPin(_rightMotorPin1, HIGH);
            setMotorPin(_rightMotorPin2, LOW);
            break;
            
        case RIGHT:
            setMotorPin(_leftMotorPin1, HIGH);
            setMotorPin(_leftMotorPin2, LOW);
            setMotorPin(_rightMotorPin1, LOW);
            setMotorPin(_rightMotorPin2, HIGH);
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
    setMotorPin(_leftMotorPin1, LOW);
    setMotorPin(_leftMotorPin2, LOW);
    setMotorPin(_rightMotorPin1, LOW);
    setMotorPin(_rightMotorPin2, LOW);
    
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

void MotorControl::setMotorPin(int pin, int value) {
    if (!_initialized) {
        return;
    }
    
    if (_useIoExtender && _ioExtender) {
        _ioExtender->digitalWrite(pin, value);
    } else {
        digitalWrite(pin, value);
    }
}

} // namespace Motors
