#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

namespace Motors {

/**
 * Block motor class for lifting and manipulating blocks
 */
class BlockMotor {
public:
    enum BlockPosition {
        DOWN,       // Lowest position for picking up blocks
        HOLD,       // Middle position for holding blocks
        UP          // Highest position for lifting blocks
    };

    BlockMotor();
    ~BlockMotor();

    /**
     * Initialize block motor
     * @param liftPin Pin for the lifting servo
     * @return true if initialization was successful, false otherwise
     */
    bool init(int liftPin = 16);

    /**
     * Move block lifter to a specific position
     * @param position The desired position
     */
    void moveToPosition(BlockPosition position);

    /**
     * Get the current position of the block lifter
     * @return The current position
     */
    BlockPosition getCurrentPosition() const;

private:
    Servo _liftServo;
    BlockPosition _currentPosition;
    int _liftPin;
    bool _initialized;
    
    // Servo angles for different positions
    static constexpr int DOWN_ANGLE = 0;
    static constexpr int HOLD_ANGLE = 90;
    static constexpr int UP_ANGLE = 180;
};

} // namespace Motors
