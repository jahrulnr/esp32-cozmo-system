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
     * @param leftPin The pin for the left sensor
     * @param rightPin The pin for the right sensor
     * @return true if initialization was successful, false otherwise
     */
    bool init(int pin = 13, int rightPin = 12);

    /**
     * Update sensor readings
     */
    void update();

    /**
     * Check if a cliff is detected on the left side
     * @return true if a cliff is detected, false otherwise
     */
    bool isLeftCliffDetected() const;

    /**
     * Check if a cliff is detected on the right side
     * @return true if a cliff is detected, false otherwise
     */
    bool isRightCliffDetected() const;

    /**
     * Check if any cliff is detected
     * @return true if any cliff is detected, false otherwise
     */
    bool isAnyCliffDetected() const;

    /**
     * Calibrate the cliff detector
     * @return true if calibration was successful, false otherwise
     */
    bool calibrate();

private:
    int _leftPin, _rightPin;
    bool _leftCliffDetected, _rightCliffDetected;
    int _leftThreshold, _rightThreshold;
    bool _initialized;
};

} // namespace Sensors
