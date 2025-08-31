#include "OrientationSensor.h"
#include <math.h>

namespace Sensors {

// adjustable based on position implementation
const int BUFFER_X1 = 2; // original is 0
const int BUFFER_Y1 = 0; // original is 2
const int BUFFER_Z1 = 4;
const int BUFFER_X2 = 3; // original is 1
const int BUFFER_Y2 = 1; // original is 3
const int BUFFER_Z2 = 5;

OrientationSensor::OrientationSensor() : _x(0), _y(0), _z(0), 
               _accelX(0), _accelY(0), _accelZ(0),
               _offsetX(0), _offsetY(0), _offsetZ(0),
               _accelOffsetX(0), _accelOffsetY(0), _accelOffsetZ(0),
               _initialized(false), _wire(nullptr),
               _gyroRange(GYRO_RANGE_250_DEG), 
               _accelRange(ACCEL_RANGE_2G),
               _gyroScale(131.0), // Default scale for ±250°/s
               _accelScale(16384.0) // Default scale for ±2g
{
}

OrientationSensor::~OrientationSensor() {
    // Clean up resources if needed
}

bool OrientationSensor::init(int sda, int scl) {
    if (!Utils::I2CManager::getInstance().initBus("base", sda, scl)) {
        Serial.println("Failed to initialize I2C bus for gyroscope");
        return false;
    }
    
    _wire = Utils::I2CManager::getInstance().getBus("base");
    if (!_wire) {
        Serial.println("Failed to get I2C bus for gyroscope");
        return false;
    }
    
    // MPU6050 register addresses
    const uint8_t MPU6050_ADDR = 0x68;
    const uint8_t MPU6050_REG_PWR_MGMT_1 = 0x6B;
    const uint8_t MPU6050_REG_CONFIG = 0x1A;
    const uint8_t MPU6050_REG_GYRO_CONFIG = 0x1B;
    const uint8_t MPU6050_REG_ACCEL_CONFIG = 0x1C;
    
    // Wake up the MPU6050
    if (!Utils::I2CManager::getInstance().devicePresent("base", MPU6050_ADDR)) {
        Serial.println("MPU6050 not detected");
        return false;
    }
    
    // Initialize the MPU6050
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1, 0)) {
        Serial.println("Failed to wake up MPU6050");
        return false;
    }
    
    // Configure the gyroscope with default range (±250°/s)
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_GYRO_CONFIG, _gyroRange)) {
        Serial.println("Failed to configure gyroscope");
        return false;
    }
    
    // Configure the accelerometer with default range (±2g)
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_ACCEL_CONFIG, _accelRange)) {
        Serial.println("Failed to configure accelerometer");
        return false;
    }
    
    // Initialize scaling factors
    updateScalingFactors();
    
    // Configure digital low-pass filter
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_CONFIG, 0x03)) {
        Serial.println("Failed to configure DLPF");
        return false;
    }
    
    Serial.println("MPU6050 initialized successfully");
    _initialized = true;
    return true;
}

void OrientationSensor::update() {
    if (!_initialized) {
        return;
    }
    
    // MPU6050 register addresses
    const uint8_t MPU6050_ADDR = 0x68;
    const uint8_t MPU6050_REG_ACCEL_XOUT_H = 0x3B;  // Accelerometer data starts at this register
    const uint8_t MPU6050_REG_GYRO_XOUT_H = 0x43;   // Gyroscope data starts at this register
    
    // Read accelerometer data - 6 bytes total (2 bytes per axis x, y, z)
    uint8_t accelBuffer[6];
    if (Utils::I2CManager::getInstance().readRegisters("base", MPU6050_ADDR, 
                                                     MPU6050_REG_ACCEL_XOUT_H, 
                                                     accelBuffer, sizeof(accelBuffer))) {
        // Convert raw values to g using current scaling factor
        _accelX = ((int16_t)((accelBuffer[BUFFER_X1] << 8) | accelBuffer[BUFFER_X2])) / _accelScale;
        _accelY = ((int16_t)((accelBuffer[BUFFER_Y1] << 8) | accelBuffer[BUFFER_Y2])) / _accelScale;
        _accelZ = ((int16_t)((accelBuffer[BUFFER_Z1] << 8) | accelBuffer[BUFFER_Z2])) / _accelScale;
        
        // Apply calibration offsets
        _accelX -= _accelOffsetX;
        _accelY -= _accelOffsetY;
        _accelZ -= _accelOffsetZ;
    } else {
        Serial.println("Failed to read accelerometer data");
    }
    
    // Read gyroscope data - 6 bytes total (2 bytes per axis x, y, z)
    uint8_t gyroBuffer[6];
    if (Utils::I2CManager::getInstance().readRegisters("base", MPU6050_ADDR, 
                                                     MPU6050_REG_GYRO_XOUT_H, 
                                                     gyroBuffer, sizeof(gyroBuffer))) {
        // Convert raw values to degrees per second using current scaling factor
        _x = ((int16_t)((gyroBuffer[BUFFER_X1] << 8) | gyroBuffer[BUFFER_X2])) / _gyroScale;
        _y = ((int16_t)((gyroBuffer[BUFFER_Y1] << 8) | gyroBuffer[BUFFER_Y2])) / _gyroScale;
        _z = ((int16_t)((gyroBuffer[BUFFER_Z1] << 8) | gyroBuffer[BUFFER_Z2])) / _gyroScale;
        
        // Apply calibration offsets
        _x -= _offsetX;
        _y -= _offsetY;
        _z -= _offsetZ;
    } else {
        Serial.println("Failed to read gyroscope data");
    }
}

float OrientationSensor::getX() const {
    return _x;
}

float OrientationSensor::getY() const {
    return _y;
}

float OrientationSensor::getZ() const {
    return _z;
}

float OrientationSensor::getAccelX() const {
    return _accelX;
}

float OrientationSensor::getAccelY() const {
    return _accelY;
}

float OrientationSensor::getAccelZ() const {
    return _accelZ;
}

float OrientationSensor::getAccelMagnitude() const {
    // Calculate the magnitude of the acceleration vector using Euclidean distance
    return sqrt(_accelX * _accelX + _accelY * _accelY + _accelZ * _accelZ);
}

bool OrientationSensor::calibrate() {
    if (!_initialized) {
        return false;
    }
    
    Serial.println("Starting gyroscope and accelerometer calibration...");
    
    // Calibration procedure
    const int samples = 100;
    float sumGyroX = 0, sumGyroY = 0, sumGyroZ = 0;
    float sumAccelX = 0, sumAccelY = 0, sumAccelZ = 0;
    
    // MPU6050 register addresses
    const uint8_t MPU6050_ADDR = 0x68;
    const uint8_t MPU6050_REG_ACCEL_XOUT_H = 0x3B;
    const uint8_t MPU6050_REG_GYRO_XOUT_H = 0x43;
    
    // Take multiple readings and average them
    for (int i = 0; i < samples; i++) {
        uint8_t accelBuffer[6];
        if (Utils::I2CManager::getInstance().readRegisters("base", MPU6050_ADDR, 
                                                         MPU6050_REG_ACCEL_XOUT_H, 
                                                         accelBuffer, sizeof(accelBuffer))) {
            // Get raw accelerometer values
            float rawAccelX = ((int16_t)((accelBuffer[BUFFER_X1] << 8) | accelBuffer[BUFFER_X2])) / _accelScale;
            float rawAccelY = ((int16_t)((accelBuffer[BUFFER_Y1] << 8) | accelBuffer[BUFFER_Y2])) / _accelScale;
            float rawAccelZ = ((int16_t)((accelBuffer[BUFFER_Z1] << 8) | accelBuffer[BUFFER_Z2])) / _accelScale;
            
            sumAccelX += rawAccelX;
            sumAccelY += rawAccelY;
            sumAccelZ += rawAccelZ;
        }
        
        uint8_t gyroBuffer[6];
        if (Utils::I2CManager::getInstance().readRegisters("base", MPU6050_ADDR, 
                                                         MPU6050_REG_GYRO_XOUT_H, 
                                                         gyroBuffer, sizeof(gyroBuffer))) {
            // Get raw gyro values
            float rawGyroX = ((int16_t)((gyroBuffer[BUFFER_X1] << 8) | gyroBuffer[BUFFER_X2])) / _gyroScale;
            float rawGyroY = ((int16_t)((gyroBuffer[BUFFER_Y1] << 8) | gyroBuffer[BUFFER_Y2])) / _gyroScale;
            float rawGyroZ = ((int16_t)((gyroBuffer[BUFFER_Z1] << 8) | gyroBuffer[BUFFER_Z2])) / _gyroScale;
            
            sumGyroX += rawGyroX;
            sumGyroY += rawGyroY;
            sumGyroZ += rawGyroZ;
        }
        
        delay(10);
    }
    
    // Calculate new offsets
    _offsetX = sumGyroX / samples;
    _offsetY = sumGyroY / samples;
    _offsetZ = sumGyroZ / samples;
    
    // For accelerometer, we only want to zero X and Y, but leave Z with gravity of ~1g
    _accelOffsetX = sumAccelX / samples;
    _accelOffsetY = sumAccelY / samples;
    _accelOffsetZ = sumAccelZ / samples;
    
    Serial.printf("Calibration complete.\n");
    Serial.printf("Gyro offsets: X=%.4f, Y=%.4f, Z=%.4f\n", _offsetX, _offsetY, _offsetZ);
    Serial.printf("Accel offsets: X=%.4f, Y=%.4f, Z=%.4f\n", _accelOffsetX, _accelOffsetY, _accelOffsetZ);
    
    return true;
}

void OrientationSensor::updateScalingFactors() {
    // Update gyroscope scaling factor based on range
    switch (_gyroRange) {
        case GYRO_RANGE_250_DEG:
            _gyroScale = 131.0f;
            break;
        case GYRO_RANGE_500_DEG:
            _gyroScale = 65.5f;
            break;
        case GYRO_RANGE_1000_DEG:
            _gyroScale = 32.8f;
            break;
        case GYRO_RANGE_2000_DEG:
            _gyroScale = 16.4f;
            break;
        default:
            _gyroScale = 131.0f; // Default to most sensitive
    }
    
    // Update accelerometer scaling factor based on range
    switch (_accelRange) {
        case ACCEL_RANGE_2G:
            _accelScale = 16384.0f;
            break;
        case ACCEL_RANGE_4G:
            _accelScale = 8192.0f;
            break;
        case ACCEL_RANGE_8G:
            _accelScale = 4096.0f;
            break;
        case ACCEL_RANGE_16G:
            _accelScale = 2048.0f;
            break;
        default:
            _accelScale = 16384.0f; // Default to most sensitive
    }
}

bool OrientationSensor::setGyroRange(GyroRange range) {
    if (!_initialized) {
        return false;
    }
    
    const uint8_t MPU6050_ADDR = 0x68;
    const uint8_t MPU6050_REG_GYRO_CONFIG = 0x1B;
    
    // Configure the gyroscope with new range
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_GYRO_CONFIG, range)) {
        Serial.println("Failed to change gyroscope range");
        return false;
    }
    
    _gyroRange = range;
    updateScalingFactors();
    
    Serial.printf("Gyroscope range changed to: %d\n", range);
    return true;
}

bool OrientationSensor::setAccelRange(AccelRange range) {
    if (!_initialized) {
        return false;
    }
    
    const uint8_t MPU6050_ADDR = 0x68;
    const uint8_t MPU6050_REG_ACCEL_CONFIG = 0x1C;
    
    // Configure the accelerometer with new range
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_ACCEL_CONFIG, range)) {
        Serial.println("Failed to change accelerometer range");
        return false;
    }
    
    _accelRange = range;
    updateScalingFactors();
    
    Serial.printf("Accelerometer range changed to: %d\n", range);
    return true;
}

GyroRange OrientationSensor::getGyroRange() const {
    return _gyroRange;
}

AccelRange OrientationSensor::getAccelRange() const {
    return _accelRange;
}

} // namespace Sensors
