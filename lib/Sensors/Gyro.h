#pragma once

#include <Arduino.h>
#include <Wire.h>

namespace Sensors {

/**
 * Gyroscope sensor class for motion detection and orientation
 */
class Gyro {
public:
    Gyro();
    ~Gyro();

    /**
     * Initialize the gyroscope
     * @param sda The SDA pin for I2C communication
     * @param scl The SCL pin for I2C communication
     * @return true if initialization was successful, false otherwise
     */
    bool init(int sda = 14, int scl = 15);

    /**
     * Update gyroscope readings
     */
    void update();

    /**
     * Get the X-axis rotation in degrees
     * @return X-axis rotation in degrees
     */
    float getX() const;

    /**
     * Get the Y-axis rotation in degrees
     * @return Y-axis rotation in degrees
     */
    float getY() const;

    /**
     * Get the Z-axis rotation in degrees
     * @return Z-axis rotation in degrees
     */
    float getZ() const;

    /**
     * Calibrate the gyroscope
     * @return true if calibration was successful, false otherwise
     */
    bool calibrate();

private:
    float _x, _y, _z;  // Current gyroscope readings
    float _offsetX, _offsetY, _offsetZ;  // Calibration offsets
    bool _initialized;
    TwoWire* _wire;
};

} // namespace Sensors
