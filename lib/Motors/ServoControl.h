#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

namespace Motors {

/**
 * Servo control class for camera head/hand and arm control
 */
class ServoControl {
public:
    ServoControl();
    ~ServoControl();

    /**
     * Initialize servo control
     * @param headServoPin Pin for the head servo
     * @param handServoPin Pin for the hand servo
     * @return true if initialization was successful, false otherwise
     */
    bool init(int headServoPin = -1, int handServoPin = -1);

    /**
     * Set the head angle
     * @param angle Angle in degrees (0-180)
     */
    void setHead(int angle);

    /**
     * Set the hand angle
     * @param angle Angle in degrees (0-180)
     */
    void setHand(int angle);

    /**
     * Get the current head angle
     * @return Current head angle in degrees
     */
    int getHead() const;

    /**
     * Get the current hand angle
     * @return Current hand angle in degrees
     */
    int getHand() const;

private:
    Servo _headServo;
    Servo _handServo;
    int _headAngle, _handAngle, _armAngle;
    int _headServoPin, _handServoPin, _armServoPin;
    bool _initialized;
};

} // namespace Motors
