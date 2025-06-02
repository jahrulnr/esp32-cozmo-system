#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "../Utils/I2CManager.h"

namespace Sensors {

/**
 * Gyroscope and accelerometer sensor class for motion detection and orientation
 */
class Gyro {
public:
    Gyro();
    ~Gyro();

    /**
     * Initialize the gyroscope and accelerometer
     * @param sda The SDA pin for I2C communication
     * @param scl The SCL pin for I2C communication
     * @return true if initialization was successful, false otherwise
     */
    bool init(int sda = 14, int scl = 15);

    /**
     * Update gyroscope and accelerometer readings
     */
    void update();

    /**
     * Get the X-axis rotation in degrees per second
     * @return X-axis rotation in degrees per second
     */
    float getX() const;

    /**
     * Get the Y-axis rotation in degrees per second
     * @return Y-axis rotation in degrees per second
     */
    float getY() const;

    /**
     * Get the Z-axis rotation in degrees per second
     * @return Z-axis rotation in degrees per second
     */
    float getZ() const;

    /**
     * Get the X-axis acceleration in g (9.81 m/s²)
     * @return X-axis acceleration in g
     */
    float getAccelX() const;

    /**
     * Get the Y-axis acceleration in g (9.81 m/s²)
     * @return Y-axis acceleration in g
     */
    float getAccelY() const;

    /**
     * Get the Z-axis acceleration in g (9.81 m/s²)
     * @return Z-axis acceleration in g
     */
    float getAccelZ() const;

    /**
     * Calculate acceleration magnitude (total acceleration vector)
     * @return Total acceleration magnitude in g
     */
    float getAccelMagnitude() const;

    /**
     * Calibrate the gyroscope and accelerometer
     * @return true if calibration was successful, false otherwise
     */
    bool calibrate();

private:
    float _x, _y, _z;  // Current gyroscope readings (degrees per second)
    float _accelX, _accelY, _accelZ;  // Current accelerometer readings (g)
    float _offsetX, _offsetY, _offsetZ;  // Gyro calibration offsets
    float _accelOffsetX, _accelOffsetY, _accelOffsetZ;  // Accel calibration offsets
    bool _initialized;
    TwoWire* _wire;
};

} // namespace Sensors
