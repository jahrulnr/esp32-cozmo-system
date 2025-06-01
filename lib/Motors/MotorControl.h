#pragma once

#include <Arduino.h>

namespace Motors {

/**
 * Motor control class for robot movement
 */
class MotorControl {
public:
    enum Direction {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        STOP
    };

    MotorControl();
    ~MotorControl();

    /**
     * Initialize motor control
     * @param leftMotorPin1 First pin for the left motor
     * @param leftMotorPin2 Second pin for the left motor
     * @param rightMotorPin1 First pin for the right motor
     * @param rightMotorPin2 Second pin for the right motor
     * @return true if initialization was successful, false otherwise
     */
    bool init(int leftMotorPin1 = 2, int leftMotorPin2 = 4, 
              int rightMotorPin1 = 13, int rightMotorPin2 = 12);

    /**
     * Set the speed of the motors
     * @param speed Speed value (0-255)
     */
    void setSpeed(uint8_t speed);

    /**
     * Move in a specified direction
     * @param direction Direction to move
     * @param duration Duration of movement in milliseconds (0 = continuous)
     */
    void move(Direction direction, unsigned long duration = 0);

    /**
     * Stop all motors
     */
    void stop();

    /**
     * Get the current direction of movement
     * @return The current direction
     */
    Direction getCurrentDirection() const;

private:
    int _leftMotorPin1, _leftMotorPin2;
    int _rightMotorPin1, _rightMotorPin2;
    uint8_t _speed;
    Direction _currentDirection;
    bool _initialized;
};

} // namespace Motors
