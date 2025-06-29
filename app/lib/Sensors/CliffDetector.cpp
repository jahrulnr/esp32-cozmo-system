#include "CliffDetector.h"
#include "lib/Utils/Logger.h"

namespace Sensors {

CliffDetector::CliffDetector() : _pin(-1), 
                                 _cliffDetected(false), 
                                 _initialized(false),
                                 _useIoExtender(false),
                                 _ioExtender(nullptr) {
}

CliffDetector::~CliffDetector() {
    // Clean up resources if needed
}

bool CliffDetector::init(int pin) {
    _pin = pin;
    _useIoExtender = false;
    _ioExtender = nullptr;
    
    pinMode(_pin, INPUT);
    
    _initialized = true;
    Utils::Logger::getInstance().info("CliffDetector: Initialized with direct GPIO pin %d", _pin);
    return true;
}

bool CliffDetector::initWithExtender(Utils::IOExtern* ioExtender, int pin) {
    if (!ioExtender) {
        Utils::Logger::getInstance().error("CliffDetector: Invalid I/O extender provided");
        return false;
    }
    
    _ioExtender = ioExtender;
    _useIoExtender = true;
    _pin = pin;
    
    // No need to call pinMode for I/O extender pins
    
    _initialized = true;
    Utils::Logger::getInstance().info("CliffDetector: Initialized with I/O extender pin %d", _pin);
    return true;
}

void CliffDetector::update() {
    if (!_initialized) {
        return;
    }
    // Use digitalRead: 1 means cliff detected
    int value = readPin();
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

int CliffDetector::readPin() {
    if (!_initialized) {
        return LOW;
    }
    
    if (_useIoExtender && _ioExtender) {
        return _ioExtender->digitalRead(_pin);
    } else {
        return digitalRead(_pin);
    }
}

} // namespace Sensors
