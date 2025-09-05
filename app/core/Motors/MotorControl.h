#pragma once

#include <Arduino.h>
#include "display/Display.h"
#include "IOExtern.h"

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
     * Initialize motor control with direct GPIO pins
     * @param leftMotorPin1 First pin for the left motor
     * @param leftMotorPin2 Second pin for the left motor
     * @param rightMotorPin1 First pin for the right motor
     * @param rightMotorPin2 Second pin for the right motor
     * @return true if initialization was successful, false otherwise
     */
    bool init(int leftMotorPin1 = 2, int leftMotorPin2 = 4, 
              int rightMotorPin1 = 13, int rightMotorPin2 = 12);
              
    /**
     * Initialize motor control with I/O extender
     * @param ioExtender Pointer to PCF8575 I/O extender
     * @param leftMotorPin1 First pin for the left motor on I/O extender
     * @param leftMotorPin2 Second pin for the left motor on I/O extender
     * @param rightMotorPin1 First pin for the right motor on I/O extender
     * @param rightMotorPin2 Second pin for the right motor on I/O extender
     * @return true if initialization was successful, false otherwise
     */
    bool initWithExtender(Utils::IOExtern* ioExtender, int leftMotorPin1 = 0, int leftMotorPin2 = 1, 
                         int rightMotorPin1 = 2, int rightMotorPin2 = 3);

    /**
     * Move in a specified direction
     * @param direction Direction to move
     * @param duration Duration of movement in milliseconds (0 = continuous)
     */
    void move(Direction direction, unsigned long duration = 0);

    void disable();
    void enable();

    /**
     * Stop all motors
     */
    void stop();

    /**
     * Get the current direction of movement
     * @return The current direction
     */
    Direction getCurrentDirection() const;

    void setDisplay(Display::Display *display);
    void interuptMotor();

private:
    int _leftMotorPin1, _leftMotorPin2;
    int _rightMotorPin1, _rightMotorPin2;
    Direction _currentDirection;
    bool _interrupt;
    bool _initialized;
    bool _useIoExtender;
    Utils::IOExtern* _ioExtender;
    Display::Display *_display;
    bool _enable;
    
    void moveLook(Direction direction);
    bool isInterrupt();
    void setMotorPin(int pin, int value);  // Helper for unified pin control
};

} // namespace Motors
