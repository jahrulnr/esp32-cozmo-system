#include "Gyro.h"
#include <math.h>

namespace Sensors {

Gyro::Gyro() : _x(0), _y(0), _z(0), 
               _accelX(0), _accelY(0), _accelZ(0),
               _offsetX(0), _offsetY(0), _offsetZ(0),
               _accelOffsetX(0), _accelOffsetY(0), _accelOffsetZ(0),
               _initialized(false), _wire(nullptr) {
}

Gyro::~Gyro() {
    // Clean up resources if needed
}

bool Gyro::init(int sda, int scl) {
    if (!Utils::I2CManager::getInstance().initBus("base", sda, scl, 400000)) {
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
    
    // Configure the gyroscope (±250°/s range) - 0x00
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_GYRO_CONFIG, 0x00)) {
        Serial.println("Failed to configure gyroscope");
        return false;
    }
    
    // Configure the accelerometer (±2g range) - 0x00
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_ACCEL_CONFIG, 0x00)) {
        Serial.println("Failed to configure accelerometer");
        return false;
    }
    
    // Configure digital low-pass filter
    if (!Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_CONFIG, 0x03)) {
        Serial.println("Failed to configure DLPF");
        return false;
    }
    
    Serial.println("MPU6050 initialized successfully");
    _initialized = true;
    return true;
}

void Gyro::update() {
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
        // Convert raw values to g (±2g range)
        _accelX = ((int16_t)((accelBuffer[0] << 8) | accelBuffer[1])) / 16384.0;
        _accelY = ((int16_t)((accelBuffer[2] << 8) | accelBuffer[3])) / 16384.0;
        _accelZ = ((int16_t)((accelBuffer[4] << 8) | accelBuffer[5])) / 16384.0;
        
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
        // Convert raw values to degrees per second (±250°/s range)
        _x = ((int16_t)((gyroBuffer[0] << 8) | gyroBuffer[1])) / 131.0;
        _y = ((int16_t)((gyroBuffer[2] << 8) | gyroBuffer[3])) / 131.0;
        _z = ((int16_t)((gyroBuffer[4] << 8) | gyroBuffer[5])) / 131.0;
        
        // Apply calibration offsets
        _x -= _offsetX;
        _y -= _offsetY;
        _z -= _offsetZ;
    } else {
        Serial.println("Failed to read gyroscope data");
    }
}

float Gyro::getX() const {
    return _x;
}

float Gyro::getY() const {
    return _y;
}

float Gyro::getZ() const {
    return _z;
}

float Gyro::getAccelX() const {
    return _accelX;
}

float Gyro::getAccelY() const {
    return _accelY;
}

float Gyro::getAccelZ() const {
    return _accelZ;
}

float Gyro::getAccelMagnitude() const {
    // Calculate the magnitude of the acceleration vector using Euclidean distance
    return sqrt(_accelX * _accelX + _accelY * _accelY + _accelZ * _accelZ);
}

bool Gyro::calibrate() {
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
            float rawAccelX = ((int16_t)((accelBuffer[0] << 8) | accelBuffer[1])) / 16384.0;
            float rawAccelY = ((int16_t)((accelBuffer[2] << 8) | accelBuffer[3])) / 16384.0;
            float rawAccelZ = ((int16_t)((accelBuffer[4] << 8) | accelBuffer[5])) / 16384.0;
            
            sumAccelX += rawAccelX;
            sumAccelY += rawAccelY;
            sumAccelZ += rawAccelZ;
        }
        
        uint8_t gyroBuffer[6];
        if (Utils::I2CManager::getInstance().readRegisters("base", MPU6050_ADDR, 
                                                         MPU6050_REG_GYRO_XOUT_H, 
                                                         gyroBuffer, sizeof(gyroBuffer))) {
            // Get raw gyro values
            float rawGyroX = ((int16_t)((gyroBuffer[0] << 8) | gyroBuffer[1])) / 131.0;
            float rawGyroY = ((int16_t)((gyroBuffer[2] << 8) | gyroBuffer[3])) / 131.0;
            float rawGyroZ = ((int16_t)((gyroBuffer[4] << 8) | gyroBuffer[5])) / 131.0;
            
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
    _accelOffsetZ = (sumAccelZ / samples) - 1.0; // Subtract 1g to account for gravity
    
    Serial.printf("Calibration complete.\n");
    Serial.printf("Gyro offsets: X=%.4f, Y=%.4f, Z=%.4f\n", _offsetX, _offsetY, _offsetZ);
    Serial.printf("Accel offsets: X=%.4f, Y=%.4f, Z=%.4f\n", _accelOffsetX, _accelOffsetY, _accelOffsetZ);
    
    return true;
}

} // namespace Sensors
