#ifndef CUBE3D_H
#define CUBE3D_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include "core/Sensors/OrientationSensor.h"

namespace Display {

struct Point3D {
    float x, y, z;

    Point3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Point2D {
    int x, y;

    Point2D(int x = 0, int y = 0) : x(x), y(y) {}
};

class Cube3D {
private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* _display;
    int _width;
    int _height;
    int _centerX;
    int _centerY;

    // Cube vertices (in 3D space)
    Point3D _vertices[8];
    Point2D _projectedVertices[8];

    // Current rotation angles (in radians)
    float _rotX, _rotY, _rotZ;

    // Complementary filter parameters
    float _alpha; // Filter coefficient (0.95 = 95% gyro, 5% accel)

    // Timing for gyroscope integration
    unsigned long _lastUpdateTime;

    // Auto-drift correction
    float _stationaryTime;
    float _gyroThreshold;

    // Last gyro readings for debugging
    float _lastGyroX, _lastGyroY, _lastGyroZ;

    // Cube size
    float _cubeSize;

    // Projection distance
    float _distance;

    // Cube edges (vertex index pairs)
    static const int _edges[12][2];

public:
    Cube3D(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display, int width = 128, int height = 64);
    ~Cube3D();

    /**
     * Update cube rotation based on orientation sensor data
     * @param orientation The orientation sensor instance
     */
    void updateRotation(Sensors::OrientationSensor* orientation);

    /**
     * Update cube rotation with explicit angles
     * @param rotX Rotation around X axis (pitch) in degrees
     * @param rotY Rotation around Y axis (yaw) in degrees
     * @param rotZ Rotation around Z axis (roll) in degrees
     */
    void updateRotation(float rotX, float rotY, float rotZ);

    /**
     * Draw the 3D cube
     */
    void draw();

    /**
     * Set cube size
     * @param size Cube size (default: 20)
     */
    void setCubeSize(float size);

    /**
     * Set complementary filter coefficient
     * @param alpha Filter coefficient (0.9-0.99, higher = more gyro influence)
     */
    void setFilterAlpha(float alpha);

private:
    /**
     * Initialize cube vertices
     */
    void initVertices();

    /**
     * Rotate a point around X axis
     * @param point The point to rotate
     * @param angle Rotation angle in radians
     * @return Rotated point
     */
    Point3D rotateX(const Point3D& point, float angle);

    /**
     * Rotate a point around Y axis
     * @param point The point to rotate
     * @param angle Rotation angle in radians
     * @return Rotated point
     */
    Point3D rotateY(const Point3D& point, float angle);

    /**
     * Rotate a point around Z axis
     * @param point The point to rotate
     * @param angle Rotation angle in radians
     * @return Rotated point
     */
    Point3D rotateZ(const Point3D& point, float angle);

    /**
     * Project 3D point to 2D screen coordinates
     * @param point 3D point to project
     * @return 2D screen coordinates
     */
    Point2D project3Dto2D(const Point3D& point);

    /**
     * Draw a line between two 2D points
     * @param p1 First point
     * @param p2 Second point
     */
    void drawLine(const Point2D& p1, const Point2D& p2);

    /**
     * Check if a point is within screen bounds
     * @param point Point to check
     * @return true if point is visible
     */
    bool isPointVisible(const Point2D& point);
};

} // namespace Display

#endif // CUBE3D_H
