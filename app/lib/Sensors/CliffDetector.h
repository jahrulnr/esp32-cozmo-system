#pragma once

#include <Arduino.h>

namespace Sensors {

/**
 * Cliff detector class for detecting edges/cliffs
 */
class CliffDetector {
public:
    CliffDetector();
    ~CliffDetector();

    /**
     * Initialize the cliff detector
     * @param pin The pin for the sensor
     * @return true if initialization was successful, false otherwise
     */
    bool init(int pin = 13);

    /**
     * Update sensor readings
     */
    void update();

    /**
     * Check if a cliff is detected on the side
     * @return true if a cliff is detected, false otherwise
     */
    bool isCliffDetected() const;

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
};

} // namespace Sensors
