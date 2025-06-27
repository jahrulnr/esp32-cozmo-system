#pragma once

#include <Arduino.h>
#include "lib/Utils/IOExtern.h"

namespace Sensors {

/**
 * Cliff detector class for detecting edges/cliffs
 */
class CliffDetector {
public:
    CliffDetector();
    ~CliffDetector();

    /**
     * Initialize the cliff detector with a direct GPIO pin
     * @param pin The GPIO pin for the sensor
     * @return true if initialization was successful, false otherwise
     */
    bool init(int pin = 13);
    
    /**
     * Initialize the cliff detector with an I/O extender
     * @param ioExtender Pointer to PCF8575 I/O extender
     * @param pin The pin number on the I/O extender
     * @return true if initialization was successful, false otherwise
     */
    bool initWithExtender(Utils::IOExtern* ioExtender, int pin = 6);

    /**
     * Update sensor readings
     */
    void update();

    /**
     * Check if a cliff is detected on the side
     * @return true if a cliff is detected, false otherwise
     */
    bool isCliffDetected();

    /**
     * Calibrate the cliff detector
     * @return true if calibration was successful, false otherwise
     */
    bool calibrate();

private:
    int _pin;
    bool _cliffDetected;
    int _threshold;
    bool _initialized;
    bool _useIoExtender;
    Utils::IOExtern* _ioExtender;
    
    // Helper method to read from either GPIO or I/O extender
    int readPin();
};

} // namespace Sensors
