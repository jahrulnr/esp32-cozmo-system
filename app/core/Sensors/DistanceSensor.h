#pragma once

#include <Arduino.h>

namespace Sensors {

/**
 * HC-SR04 Ultrasonic Distance Sensor class
 *
 * This class provides an interface to the HC-SR04 ultrasonic sensor
 * for measuring distances.
 */
class DistanceSensor {
public:
    DistanceSensor();
    ~DistanceSensor();

    /**
     * Initialize the distance sensor
     * @param triggerPin GPIO pin connected to the TRIG pin of the sensor
     * @param echoPin GPIO pin connected to the ECHO pin of the sensor
     * @param maxDistance Maximum distance to measure in centimeters (default: 400cm)
     * @return true if initialization was successful, false otherwise
     */
    bool init(int triggerPin, int echoPin);

		void setThresHold(float threshold = 20.0);

    /**
     * Measure the distance
     * @return Distance in centimeters, or -1 if measurement failed
     */
    float measureDistance();

private:
    int _triggerPin;
    int _echoPin;
		float _threshold;
    int _maxDistance;
    float _lastValue;
    bool _inprogress;
    unsigned long _timeout; // Timeout in microseconds
    bool _initialized;
};

} // namespace Sensors
