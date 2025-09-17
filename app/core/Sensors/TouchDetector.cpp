#include "TouchDetector.h"
#include "Logger.h"

namespace Sensors {

TouchDetector::TouchDetector() : _pin(-1),
                                 _detected(false),
                                 _initialized(false),
                                 _useIoExtender(false),
                                 _ioExtender(nullptr) {
}

bool TouchDetector::init(int pin) {
    _pin = pin;
    _useIoExtender = false;
    _ioExtender = nullptr;

    pinMode(_pin, INPUT);

    _initialized = true;
    Utils::Logger::getInstance().info("TouchDetector: Initialized with direct GPIO pin %d", _pin);
    return true;
}

bool TouchDetector::initWithExtender(Utils::IOExtern* ioExtender, int pin) {
    if (!ioExtender) {
        Utils::Logger::getInstance().error("TouchDetector: Invalid I/O extender provided");
        return false;
    }

    _ioExtender = ioExtender;
    _useIoExtender = true;
    _pin = pin;

    ioExtender->digitalWrite(pin, LOW);
    ioExtender->digitalRead(pin);

    _initialized = true;
    Utils::Logger::getInstance().info("TouchDetector: Initialized with I/O extender pin %d", _pin);
    return true;
}

void TouchDetector::update() {
    if (!_initialized) {
        return;
    }
    // Use digitalRead: 1 means cliff detected
    _detected = readPin();
}

bool TouchDetector::detected() {
    return _detected;
}

int TouchDetector::readPin() {
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
