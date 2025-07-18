#include "ServoControl.h"
#include "lib/Utils/Logger.h"

namespace Motors {

ServoControl::ServoControl() : _headAngle(90), _handAngle(90),
                              _headServoPin(-1), _handServoPin(-1),
                              _initialized(false), _useIoExtender(false),
                              _ioExtender(nullptr), _lastHeadPosition(0), 
                              _lastHandPosition(0) {
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
    
    // Using direct GPIO pins
    _useIoExtender = false;
    _ioExtender = nullptr;

    // Initialize ESP32 servo library
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // Attach servos to pins
    _headServo.setPeriodHertz(50);    // Standard 50hz servo
    _handServo.setPeriodHertz(50);   // Standard 50hz servo
    
    _headServo.attach(_headServoPin, 500, 2500);
    _handServo.attach(_handServoPin, 500, 2500);
    
    _initialized = true;
    Utils::Logger::getInstance().info("ServoControl: Initialized with direct GPIO pins");
    return true;
}

bool ServoControl::initWithExtender(Utils::IOExtern* ioExtender, int headServoPin, int handServoPin) {
    if (!ioExtender) {
        Utils::Logger::getInstance().error("ServoControl: Invalid I/O extender provided");
        return false;
    }
    
    _ioExtender = ioExtender;
    _useIoExtender = true;
    _headServoPin = headServoPin;
    _handServoPin = handServoPin;

    // The PCF8575 I/O expander doesn't directly support PWM for servo control
    // We need to use software PWM or external PWM driver for servos
    
    // For now, we'll use direct pins for servo control
    // In a real implementation, you might connect servos through a dedicated PWM driver like PCA9685
    
    // Initialize ESP32 servo library
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // We'll simulate PWM with the I/O extender using software PWM
    // This is not ideal but could work for simple applications
    
    // Set initial pin state to LOW
    _ioExtender->digitalWrite(_headServoPin, LOW);
    _ioExtender->digitalWrite(_handServoPin, LOW);
    
    _initialized = true;
    Utils::Logger::getInstance().info("ServoControl: Initialized with I/O extender");
    Utils::Logger::getInstance().warning("ServoControl: Note - I/O extender based servos use software PWM which may not be precise");
    
    return true;
}

void ServoControl::setScreen(Screen::Screen *screen) {
    _screen = screen;
}

void ServoControl::moveLook(ServoType type, int angle) {
    if (!_screen || !_screen->getFace()) return;
    
    int lastPosition = 0;
    switch (type) {
        case HEAD:
            lastPosition = _lastHeadPosition;
            break;
        case HAND:
            lastPosition = _lastHandPosition;
        break;
    }

    if (angle > lastPosition) 
        _screen->getFace()->LookTop();
    else _screen->getFace()->LookFront();
}

void ServoControl::setHead(int angle) {
    if (!_initialized) {
        return;
    }

    moveLook(HEAD, angle);
    
    // Constrain angle to valid range
    angle = constrain(angle, 60, 110);
    
    if (_useIoExtender && _ioExtender) {
        // Software PWM implementation for I/O extender
        Utils::Logger::getInstance().debug("ServoControl: Moving head to %d degrees using I/O extender", angle);
        
        // Smooth movement implementation
        const int step = 2;  // smaller step for smoother movement
        const int delayMs = 15;  // delay between steps
        
        // Move servo gradually to target position
        if (_headAngle < angle) {
            for (int pos = _headAngle; pos <= angle; pos += step) {
                // Perform software PWM for servo control
                // Repeat multiple times to ensure servo gets the signal
                int pulseWidth = angleToPulseWidth(pos);
                for (int i = 0; i < 5; i++) {
                    softwarePwm(_headServoPin, pulseWidth, 20000); // 50Hz = 20ms period
                }
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        } else if (_headAngle > angle) {
            for (int pos = _headAngle; pos >= angle; pos -= step) {
                // Perform software PWM for servo control
                int pulseWidth = angleToPulseWidth(pos);
                for (int i = 0; i < 5; i++) {
                    softwarePwm(_headServoPin, pulseWidth, 20000); // 50Hz = 20ms period
                }
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        }
        
        // Ensure final position is reached
        int finalPulseWidth = angleToPulseWidth(angle);
        for (int i = 0; i < 10; i++) {
            softwarePwm(_headServoPin, finalPulseWidth, 20000);
        }
    } else {
        // Standard servo control using ESP32Servo
        // Smooth movement implementation
        const int step = 2;  // smaller step for smoother movement
        const int delayMs = 15;  // delay between steps
        
        // Move servo gradually to target position
        if (_headAngle < angle) {
            for (int pos = _headAngle; pos <= angle; pos += step) {
                _headServo.write(pos);
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        } else if (_headAngle > angle) {
            for (int pos = _headAngle; pos >= angle; pos -= step) {
                _headServo.write(pos);
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        }
        
        // Ensure final position is exact
        _headServo.write(angle);
    }
    
    _headAngle = angle;
    _lastHeadPosition = angle;
}

void ServoControl::setHand(int angle) {
    if (!_initialized) {
        return;
    }

    moveLook(HAND, angle);
    
    // Constrain angle to valid range
    angle = constrain(angle, 0, 180);
    // reverse
    angle = 180 - angle;
    int targetAngle = constrain(angle, 90, 133);
    
    if (_useIoExtender && _ioExtender) {
        // Software PWM implementation for I/O extender
        Utils::Logger::getInstance().debug("ServoControl: Moving hand to %d degrees using I/O extender", angle);
        
        // Smooth movement implementation
        const int step = 2;  // smaller step for smoother movement
        const int delayMs = 20;  // delay between steps
        
        // Move servo gradually to target position
        if (_handAngle < targetAngle) {
            for (int pos = _handAngle; pos <= targetAngle; pos += step) {
                // Perform software PWM for servo control
                int pulseWidth = angleToPulseWidth(pos);
                for (int i = 0; i < 5; i++) {
                    softwarePwm(_handServoPin, pulseWidth, 20000); // 50Hz = 20ms period
                }
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        } else if (_handAngle > targetAngle) {
            for (int pos = _handAngle; pos >= targetAngle; pos -= step) {
                // Perform software PWM for servo control
                int pulseWidth = angleToPulseWidth(pos);
                for (int i = 0; i < 5; i++) {
                    softwarePwm(_handServoPin, pulseWidth, 20000); // 50Hz = 20ms period
                }
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        }
        
        // Ensure final position is reached
        int finalPulseWidth = angleToPulseWidth(targetAngle);
        for (int i = 0; i < 10; i++) {
            softwarePwm(_handServoPin, finalPulseWidth, 20000);
        }
    } else {
        // Standard servo control using ESP32Servo
        // Smooth movement implementation
        const int step = 2;  // smaller step for smoother movement
        const int delayMs = 20;  // delay between steps
        
        // Move servo gradually to target position
        if (_handAngle < targetAngle) {
            for (int pos = _handAngle; pos <= targetAngle; pos += step) {
                _handServo.write(pos);
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        } else if (_handAngle > targetAngle) {
            for (int pos = _handAngle; pos >= targetAngle; pos -= step) {
                _handServo.write(pos);
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        }
        
        // Ensure final position is exact
        _handServo.write(targetAngle);
    }
    
    _handAngle = targetAngle;
    _lastHandPosition = angle;
}

int ServoControl::getHead() const {
    return _headAngle;
}

int ServoControl::getHand() const {
    // reverse
    return 180 - _handAngle;
}

void ServoControl::softwarePwm(int pin, int pulseWidth, int totalPeriod) {
    if (!_ioExtender) return;
    
    // Simple software PWM implementation
    // This is not ideal for servo control, but can work for basic applications
    _ioExtender->digitalWrite(pin, HIGH);
    delayMicroseconds(pulseWidth);
    _ioExtender->digitalWrite(pin, LOW);
    delayMicroseconds(totalPeriod - pulseWidth);
}

int ServoControl::angleToPulseWidth(int angle) {
    // Convert angle (0-180) to pulse width (500-2500 microseconds)
    // Standard servos typically use 1000-2000, but we use a wider range for better compatibility
    return map(angle, 0, 180, 500, 2500);
}

} // namespace Motors
