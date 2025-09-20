#include "Cube3D.h"

namespace Display {

// Cube edges definition (vertex index pairs)
const int Cube3D::_edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Vertical edges
};

Cube3D::Cube3D(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display, int width, int height)
    : _display(display), _width(width), _height(height),
      _rotX(0), _rotY(0), _rotZ(0), _cubeSize(15), _distance(50) {

    _centerX = _width / 2;
    _centerY = _height / 2;

    // Initialize complementary filter parameters
    _alpha = 0.96f;           // 96% gyro, 4% accelerometer
    _lastUpdateTime = 0;
    _stationaryTime = 0;
    _gyroThreshold = 0.5f;    // degrees/second threshold for "stationary"
    initVertices();
}

Cube3D::~Cube3D() {
    // Destructor
}

void Cube3D::initVertices() {
    float half = _cubeSize / 2.0f;

    // Define cube vertices (8 corners of a cube)
    _vertices[0] = Point3D(-half, -half, -half); // Bottom-left-back
    _vertices[1] = Point3D( half, -half, -half); // Bottom-right-back
    _vertices[2] = Point3D( half,  half, -half); // Bottom-right-front
    _vertices[3] = Point3D(-half,  half, -half); // Bottom-left-front
    _vertices[4] = Point3D(-half, -half,  half); // Top-left-back
    _vertices[5] = Point3D( half, -half,  half); // Top-right-back
    _vertices[6] = Point3D( half,  half,  half); // Top-right-front
    _vertices[7] = Point3D(-half,  half,  half); // Top-left-front
}

void Cube3D::updateRotation(Sensors::OrientationSensor* orientation) {
    if (!orientation) return;

    unsigned long currentTime = millis();

    // Initialize timing on first call
    if (_lastUpdateTime == 0) {
        _lastUpdateTime = currentTime;
        return;
    }

    // Calculate time delta in seconds
    float deltaTime = (currentTime - _lastUpdateTime) / 1000.0f;
    _lastUpdateTime = currentTime;

    // Skip if time delta is too large (probably first call or long pause)
    if (deltaTime > 0.1f) {
        return;
    }

    // Get gyroscope data (degrees per second)
    // Try swapped axis mapping to match expected behavior:
    // When tilting forward/backward → pitch rotation (X-axis)
    // When turning left/right → yaw rotation (Y-axis)
    // When rolling left/right → roll rotation (Z-axis)
    float gyroX = -orientation->getY(); // Pitch rate (forward/backward tilt) - negated and swapped
    float gyroY = -orientation->getZ();  // Yaw rate (left/right turn) - swapped
    float gyroZ = orientation->getX();  // Roll rate (left/right tilt) - swapped

    // Get accelerometer data for gravity reference - match the gyro axis swap
    float accelX = -orientation->getAccelY(); // For pitch calculation 
    float accelY = orientation->getAccelZ();  // For yaw (not used)
    float accelZ = orientation->getAccelX();  // For roll calculation

    // Calculate tilt angles from accelerometer (absolute reference)
    // Pitch: rotation around X-axis (forward/backward tilt)
    float accelPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ));
    // Roll: rotation around Z-axis (left/right tilt)
    float accelRoll = atan2(accelZ, accelY);

    // Integrate gyroscope data
    float gyroPitchDelta = gyroX * deltaTime * PI / 180.0f;
    float gyroYawDelta = gyroY * deltaTime * PI / 180.0f;
    float gyroRollDelta = gyroZ * deltaTime * PI / 180.0f;

    // Complementary filter: combine gyro (short-term) with accel (long-term)
    // Pitch: forward/backward tilt (X-axis rotation)
    _rotX = _alpha * (_rotX + gyroPitchDelta) + (1.0f - _alpha) * accelPitch;
    // Roll: left/right tilt (Z-axis rotation)
    _rotZ = _alpha * (_rotZ + gyroRollDelta) + (1.0f - _alpha) * accelRoll;

    // Yaw has no gravity reference, use pure gyro integration with drift correction
    _rotY += gyroYawDelta;

    // Auto-drift correction for yaw when robot is stationary
    float gyroMagnitude = sqrt(gyroX*gyroX + gyroY*gyroY + gyroZ*gyroZ);

    if (gyroMagnitude < _gyroThreshold) {
        _stationaryTime += deltaTime;

        // After 3 seconds of being stationary, slowly drift yaw back to zero
        if (_stationaryTime > 3.0f) {
            _rotY *= 0.995f; // Slow drift correction
        }
    } else {
        _stationaryTime = 0;
    }

    // Wrap angles to prevent overflow
    while (_rotX > PI) _rotX -= 2 * PI;
    while (_rotX < -PI) _rotX += 2 * PI;
    while (_rotY > PI) _rotY -= 2 * PI;
    while (_rotY < -PI) _rotY += 2 * PI;
    while (_rotZ > PI) _rotZ -= 2 * PI;
    while (_rotZ < -PI) _rotZ += 2 * PI;
}

void Cube3D::updateRotation(float rotX, float rotY, float rotZ) {
    // Convert degrees to radians
    _rotX = rotX * PI / 180.0f;
    _rotY = rotY * PI / 180.0f;
    _rotZ = rotZ * PI / 180.0f;
}

void Cube3D::draw() {
    if (!_display) return;
		_display->clearBuffer();

    // Transform and project all vertices
    for (int i = 0; i < 8; i++) {
        Point3D rotated = _vertices[i];

        // Apply rotations in order: X, Y, Z
        rotated = rotateX(rotated, _rotX);
        rotated = rotateY(rotated, _rotY);
        rotated = rotateZ(rotated, _rotZ);

        // Project to 2D
        _projectedVertices[i] = project3Dto2D(rotated);
    }

    // Draw all edges
    for (int i = 0; i < 12; i++) {
        int v1 = _edges[i][0];
        int v2 = _edges[i][1];

        Point2D p1 = _projectedVertices[v1];
        Point2D p2 = _projectedVertices[v2];

        // Only draw if both points are visible
        if (isPointVisible(p1) && isPointVisible(p2)) {
            drawLine(p1, p2);
        }
    }

    // Draw orientation indicators
    _display->setFont(u8g2_font_4x6_tf);

    // Show rotation values as text (in degrees) with clear axis labels
    String rotText = "X:" + String((int)(_rotX * 180.0f / PI)) +
                    " Y:" + String((int)(_rotY * 180.0f / PI)) +
                    " Z:" + String((int)(_rotZ * 180.0f / PI));

    // Draw text at bottom of screen
    int textWidth = _display->getStrWidth(rotText.c_str());
    int textX = (_width - textWidth) / 2;
    _display->drawStr(textX, _height - 16, rotText.c_str());

    // Show complementary filter status
    String filterText = "CF:" + String((int)(_alpha * 100)) + "%";
    _display->drawStr(2, _height - 8, filterText.c_str());

    // Show drift correction status
    if (_stationaryTime > 3.0f) {
        _display->drawStr(_width - 20, _height - 8, "DC");
    } else {
        _display->drawStr(_width - 25, _height - 8, "GYRO");
    }
		_display->sendBuffer();
}

void Cube3D::setCubeSize(float size) {
    _cubeSize = size;
    initVertices();
}

void Cube3D::setFilterAlpha(float alpha) {
    // Clamp alpha between reasonable values
    if (alpha < 0.8f) alpha = 0.8f;
    if (alpha > 0.99f) alpha = 0.99f;
    _alpha = alpha;
}

Point3D Cube3D::rotateX(const Point3D& point, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);

    return Point3D(
        point.x,
        point.y * cosA - point.z * sinA,
        point.y * sinA + point.z * cosA
    );
}

Point3D Cube3D::rotateY(const Point3D& point, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);

    return Point3D(
        point.x * cosA + point.z * sinA,
        point.y,
        -point.x * sinA + point.z * cosA
    );
}

Point3D Cube3D::rotateZ(const Point3D& point, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);

    return Point3D(
        point.x * cosA - point.y * sinA,
        point.x * sinA + point.y * cosA,
        point.z
    );
}

Point2D Cube3D::project3Dto2D(const Point3D& point) {
    // Simple perspective projection
    float projectedX = (point.x * _distance) / (point.z + _distance);
    float projectedY = (point.y * _distance) / (point.z + _distance);

    // Convert to screen coordinates
    int screenX = _centerX + (int)projectedX;
    int screenY = _centerY - (int)projectedY; // Flip Y axis for screen coordinates

    return Point2D(screenX, screenY);
}

void Cube3D::drawLine(const Point2D& p1, const Point2D& p2) {
    _display->drawLine(p1.x, p1.y, p2.x, p2.y);
}

bool Cube3D::isPointVisible(const Point2D& point) {
    return (point.x >= 0 && point.x < _width &&
            point.y >= 0 && point.y < _height);
}

} // namespace Display
