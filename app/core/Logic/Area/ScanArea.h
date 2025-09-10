#pragma once
#include "core/Sensors/OrientationSensor.h"
#include "core/Sensors/DistanceSensor.h"

namespace Logic {

class ScanArea {
private:
	const char* _tag;
	Sensors::OrientationSensor *_orientationSensor;
	Sensors::DistanceSensor *_distanceSensor;

	// Current rotation angles (in radians)
	float _rotX, _rotY, _rotZ;
	
	// Complementary filter parameters
	const float _alpha = 0.96f; // 96% gyro, 4% accelerometer
	
	// Timing for gyroscope integration
	unsigned long _lastUpdateTime;
	
	// Auto-drift correction
	float _gyroThreshold;
	
	// Scan area parameters
	float _currentYawDegrees;  // Current yaw in degrees
	float _lastScanDistance;   // Last distance measurement

public:
	ScanArea(
		Sensors::OrientationSensor *orientationSensor,
		Sensors::DistanceSensor *distanceSensor
	): _tag("ScanArea"), 
		_orientationSensor(orientationSensor), 
		_distanceSensor(distanceSensor),
		_rotX(0), _rotY(0), _rotZ(0),
		_lastUpdateTime(0),
		_gyroThreshold(0.5f),
		_currentYawDegrees(0),
		_lastScanDistance(0)
	{ 
	}

	esp_err_t update() {
		if (!_orientationSensor || !_distanceSensor) return ESP_ERR_INVALID_STATE;
    
    unsigned long currentTime = millis();
    
    // Initialize timing on first call
    if (_lastUpdateTime == 0) {
        _lastUpdateTime = currentTime;
        return ESP_OK;
    }
    
    // Calculate time delta in seconds
    float deltaTime = (currentTime - _lastUpdateTime) / 1000.0f;
    _lastUpdateTime = currentTime;
    
    // Skip if time delta is too large (probably first call or long pause)
    if (deltaTime > 0.1f) {
        return ESP_OK;
    }
    
    // Get gyroscope data (degrees per second)
    // Try swapped axis mapping to match expected behavior:
    // When tilting forward/backward → pitch rotation (X-axis)
    // When turning left/right → yaw rotation (Y-axis) 
    // When rolling left/right → roll rotation (Z-axis)

		_orientationSensor->update();
    float gyroX = -_orientationSensor->getY(); // Pitch rate (forward/backward tilt) - negated and swapped
    float gyroY = _orientationSensor->getZ();  // Yaw rate (left/right turn) - swapped
    float gyroZ = _orientationSensor->getX();  // Roll rate (left/right tilt) - swapped
    
    // Get accelerometer data for gravity reference - match the gyro axis swap
    float accelX = -_orientationSensor->getAccelY(); // For pitch calculation - negated and swapped
    float accelY = _orientationSensor->getAccelZ();  // For yaw (not used) - swapped
    float accelZ = _orientationSensor->getAccelX();  // For roll calculation - swapped
    
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
    
    // Wrap angles to prevent overflow
    while (_rotX > PI) _rotX -= 2 * PI;
    while (_rotX < -PI) _rotX += 2 * PI;
    while (_rotY > PI) _rotY -= 2 * PI;
    while (_rotY < -PI) _rotY += 2 * PI;
    while (_rotZ > PI) _rotZ -= 2 * PI;
    while (_rotZ < -PI) _rotZ += 2 * PI;

    // Convert yaw to degrees for scan area mapping
    _currentYawDegrees = _rotY * 180.0f / PI;
    
    // Get distance measurement
    float distance = _distanceSensor->measureDistance();
		if (distance <= 1.) { // invalid if less than 1cm 
			ESP_LOGE(_tag, "Invalid distance sensor value: %.2f", distance);
			return ESP_ERR_INVALID_RESPONSE;
		}
		
    _lastScanDistance = distance;
    
    // Save scan area data to model
    return ESP_OK;
	}
	
	// Get current yaw in degrees
	float getCurrentYaw() const {
		return _currentYawDegrees;
	}
	
	// Get last scanned distance
	float getLastDistance() const {
		return _lastScanDistance;
	}
	
	// Calculate degrees with proper wrapping (-180 to +180)
	// Uses current yaw position + delta
	// Example: if current is 170° and delta is +10, result is -170°
	float calculateDegrees(float deltaDegrees) {
		float result = _currentYawDegrees + deltaDegrees;
		// Normalize to -180 to +180 range
		while (result > 180.0f) result -= 360.0f;
		while (result < -180.0f) result += 360.0f;
		return result;
	}
};

}; // end namespace
