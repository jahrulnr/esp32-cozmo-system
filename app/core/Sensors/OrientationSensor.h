#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "I2CManager.h"

namespace Sensors {

// Gyroscope sensitivity range options
enum GyroRange {
    GYRO_RANGE_250_DEG = 0x00,  // ±250°/s (default)
    GYRO_RANGE_500_DEG = 0x08,  // ±500°/s
    GYRO_RANGE_1000_DEG = 0x10, // ±1000°/s
    GYRO_RANGE_2000_DEG = 0x18  // ±2000°/s
};

// Accelerometer sensitivity range options
enum AccelRange {
    ACCEL_RANGE_2G = 0x00,  // ±2g (default)
    ACCEL_RANGE_4G = 0x08,  // ±4g
    ACCEL_RANGE_8G = 0x10,  // ±8g
    ACCEL_RANGE_16G = 0x18  // ±16g
};

/**
 * Gyroscope and accelerometer sensor class for motion detection and orientation
 */
class OrientationSensor {
public:
    OrientationSensor();
    ~OrientationSensor();

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

    /**
     * Set gyroscope sensitivity range
     * @param range The desired gyroscope range (GYRO_RANGE_250_DEG, GYRO_RANGE_500_DEG, GYRO_RANGE_1000_DEG, or GYRO_RANGE_2000_DEG)
     * @return true if setting the range was successful, false otherwise
     */
    bool setGyroRange(GyroRange range);

    /**
     * Set accelerometer sensitivity range
     * @param range The desired accelerometer range (ACCEL_RANGE_2G, ACCEL_RANGE_4G, ACCEL_RANGE_8G, or ACCEL_RANGE_16G)
     * @return true if setting the range was successful, false otherwise
     */
    bool setAccelRange(AccelRange range);

    /**
     * Get current gyroscope range setting
     * @return Current gyroscope range
     */
    GyroRange getGyroRange() const;

    /**
     * Get current accelerometer range setting
     * @return Current accelerometer range
     */
    AccelRange getAccelRange() const;

private:
    const char* TAG;
    float _x, _y, _z;  // Current gyroscope readings (degrees per second)
    float _accelX, _accelY, _accelZ;  // Current accelerometer readings (g)
    float _offsetX, _offsetY, _offsetZ;  // Gyro calibration offsets
    float _accelOffsetX, _accelOffsetY, _accelOffsetZ;  // Accel calibration offsets
    bool _initialized;
    TwoWire* _wire;
    GyroRange _gyroRange;  // Current gyroscope range
    AccelRange _accelRange;  // Current accelerometer range
    float _gyroScale;  // Current gyroscope scaling factor
    float _accelScale;  // Current accelerometer scaling factor

    /**
     * Update scaling factors based on current range settings
     */
    void updateScalingFactors();
};

} // namespace Sensors
