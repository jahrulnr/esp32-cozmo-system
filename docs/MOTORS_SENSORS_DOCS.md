# Motors and Sensors Components Documentation

## Overview

This document details the Motors and Sensors components of the Cozmo-System project. These components are essential for robot mobility, environmental sensing, and interaction capabilities.

## Table of Contents

1. [Motors Component](#motors-component)
   - [MotorControl](#motorcontrol)
   - [ServoControl](#servocontrol)
   
2. [Sensors Component](#sensors-component)
   - [Camera](#camera)
   - [Gyro](#gyro)
   - [CliffDetector](#cliffdetector)
   - [DistanceSensor](#distancesensor)

---

## Motors Component

The Motors component provides classes for controlling various types of motors in the robot, including DC motors for locomotion and servo motors for precise positioning of components like the camera or manipulator.

### MotorControl

The `MotorControl` class manages the robot's movement via DC motors, controlling direction and speed.

#### Features

- Bidirectional control of two DC motors (left and right)
- Five movement directions: FORWARD, BACKWARD, LEFT, RIGHT, STOP
- Speed control (0-255)
- Timed movement capability
- Current direction tracking

#### API Reference

```cpp
namespace Motors {

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

    bool init(int leftMotorPin1 = 2, int leftMotorPin2 = 4, 
              int rightMotorPin1 = 13, int rightMotorPin2 = 12);
    void move(Direction direction, unsigned long duration = 0);
    void stop();
    Direction getCurrentDirection() const;

private:
    int _leftMotorPin1, _leftMotorPin2;
    int _rightMotorPin1, _rightMotorPin2;
    uint8_t _speed;
    Direction _currentDirection;
    bool _initialized;
};

} // namespace Motors
```

#### Example Usage

```cpp
#include "Motors/MotorControl.h"

Motors::MotorControl motors;

void setup() {
    // Initialize motors with default pins
    motors.init();
}

void loop() {
    // Move forward for 2 seconds
    motors.move(Motors::MotorControl::FORWARD, 2000);
    
    // Turn left for 1 second
    motors.move(Motors::MotorControl::LEFT, 1000);
    
    // Move backward for 2 seconds
    motors.move(Motors::MotorControl::BACKWARD, 2000);
    
    // Turn right for 1 second
    motors.move(Motors::MotorControl::RIGHT, 1000);
    
    // Stop and wait
    motors.stop();
    delay(3000);
}
```

#### Implementation Details

The MotorControl class assumes a dual H-bridge motor driver configuration, where each motor requires two control pins for bidirectional control. The class uses PWM to control motor speed through the `analogWrite()` function.

For movement control, the combinations of pin states are:

| Direction | Left Motor | Right Motor |
|-----------|------------|-------------|
| FORWARD   | (HIGH, LOW) | (HIGH, LOW) |
| BACKWARD  | (LOW, HIGH) | (LOW, HIGH) |
| LEFT      | (LOW, HIGH) | (HIGH, LOW) |
| RIGHT     | (HIGH, LOW) | (LOW, HIGH) |
| STOP      | (LOW, LOW)  | (LOW, LOW)  |

The timed movement feature uses the `delay()` function, which blocks execution. For non-blocking operation, consider implementing timer-based control or using FreeRTOS tasks.

---

### ServoControl

The `ServoControl` class manages servo motors for precise positioning of the camera head and hand components.

#### Features

- Control of head and hand servo motors
- Angle positioning (0-180 degrees)
- Built-in angle constraints
- Integration with ESP32Servo library

#### API Reference

```cpp
namespace Motors {

class ServoControl {
public:
    ServoControl();
    ~ServoControl();

    bool init(int headServoPin = -1, int handServoPin = -1);
    void setHead(int angle);
    void setHand(int angle);
    int getHead() const;
    int getHand() const;

private:
    Servo _headServo;
    Servo _handServo;
    int _headAngle, _handAngle;
    int _headServoPin, _handServoPin;
    bool _initialized;
};

} // namespace Motors
```

#### Example Usage

```cpp
#include "Motors/ServoControl.h"

Motors::ServoControl servos;

void setup() {
    // Initialize servos with pins 12 and 13
    servos.init(12, 13);
}

void loop() {
    // Move head from left to right
    for (int angle = 0; angle <= 180; angle += 5) {
        servos.setHead(angle);
        delay(50);
    }
    
    // Move head from right to left
    for (int angle = 180; angle >= 0; angle -= 5) {
        servos.setHead(angle);
        delay(50);
    }
    
    // Move hand to various positions
    servos.setHand(0);    // Down position
    delay(1000);
    servos.setHand(90);   // Middle position
    delay(1000);
    servos.setHand(180);  // Up position
    delay(1000);
}
```

#### Implementation Details

The ServoControl class uses the ESP32Servo library, which is optimized for ESP32's hardware timer capabilities. The class initializes the servo system with:

```cpp
ESP32PWM::allocateTimer(0);
ESP32PWM::allocateTimer(1);
ESP32PWM::allocateTimer(2);
ESP32PWM::allocateTimer(3);
```

This allocates hardware timers for the servos, allowing precise PWM generation. The servos are configured with standard pulse widths:

```cpp
_headServo.attach(_headServoPin, 500, 2400);
_handServo.attach(_handServoPin, 500, 2400);
```

Where 500µs represents 0 degrees and 2400µs represents 180 degrees, providing a full range of motion.

---

## Sensors Component

The Sensors component provides interfaces to various sensors including the camera, gyroscope/accelerometer, and cliff detectors.

### Camera

The `Camera` class manages the ESP32-CAM module, handling initialization, configuration, and frame capture.

#### Features

- Camera initialization with various configuration options
- Frame capture and buffer management
- Resolution adjustment
- Camera settings control (brightness, contrast, saturation)
- Streaming interval management

#### API Reference

```cpp
namespace Sensors {

class Camera {
public:
    Camera();
    ~Camera();

    bool init();
    camera_fb_t* captureFrame();
    void returnFrame(camera_fb_t* fb);
    void setResolution(framesize_t resolution);
    framesize_t getResolution() const;
    uint32_t getStreamingInterval() const;
    void setStreamingInterval(uint32_t interval);
    void adjustSettings(int brightness, int contrast, int saturation);

private:
    framesize_t _resolution;
    bool _initialized;
    uint32_t _streamingInterval;
};

} // namespace Sensors
```

#### Example Usage

```cpp
#include "Sensors/Camera.h"

Sensors::Camera camera;

void setup() {
    Serial.begin(115200);
    
    // Initialize camera
    if (camera.init()) {
        Serial.println("Camera initialized successfully");
        
        // Set resolution to VGA (640x480)
        camera.setResolution(FRAMESIZE_VGA);
        
        // Set streaming interval to 100ms
        camera.setStreamingInterval(100);
        
        // Adjust camera settings
        camera.adjustSettings(1, 0, 1);  // Brightness+1, Normal contrast, Saturation+1
    } else {
        Serial.println("Camera initialization failed");
    }
}

void loop() {
    // Capture a frame
    camera_fb_t* frame = camera.captureFrame();
    
    if (frame) {
        // Process the frame
        Serial.printf("Frame captured: %dx%d, %d bytes\n", 
                     frame->width, frame->height, frame->len);
        
        // Return the frame buffer when done
        camera.returnFrame(frame);
    }
    
    delay(camera.getStreamingInterval());
}
```

#### Implementation Details

The Camera class initializes the ESP32-CAM module with the following configurations:

```cpp
camera_config_t config;
config.ledc_channel = LEDC_CHANNEL_0;
config.ledc_timer = LEDC_TIMER_0;
config.pin_d0 = Y2_GPIO_NUM;
config.pin_d1 = Y3_GPIO_NUM;
// ... other pin definitions

config.xclk_freq_hz = 20000000;  // 20MHz XCLK frequency
config.pixel_format = PIXFORMAT_JPEG;  // JPEG format for compression

// PSRAM configuration for better performance
if (psramFound()) {
    config.frame_size = _resolution;
    config.jpeg_quality = 10;  // 0-63, lower is better quality
    config.fb_count = 2;       // Double buffering
} else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;      // Single buffering
}
```

The class supports various resolutions through the `framesize_t` enum from the ESP32 camera library:
- FRAMESIZE_QVGA: 320x240
- FRAMESIZE_VGA: 640x480
- FRAMESIZE_SVGA: 800x600
- FRAMESIZE_XGA: 1024x768
- FRAMESIZE_UXGA: 1600x1200

Frame capture is handled by the ESP32 camera library's `esp_camera_fb_get()` function, which returns a `camera_fb_t` structure containing the image data, dimensions, and format information.

---

### Gyro

The `Gyro` class interfaces with the MPU6050 gyroscope and accelerometer sensor via I2C.

#### Features

- Rotation measurement in degrees per second (X, Y, Z axes)
- Acceleration measurement in g (X, Y, Z axes)
- Acceleration magnitude calculation
- Sensor calibration
- Integration with I2CManager for thread-safe I2C access

#### API Reference

```cpp
namespace Sensors {

class Gyro {
public:
    Gyro();
    ~Gyro();
    
    bool init(int sda = 14, int scl = 15);
    void update();
    float getX() const;
    float getY() const;
    float getZ() const;
    float getAccelX() const;
    float getAccelY() const;
    float getAccelZ() const;
    float getAccelMagnitude() const;
    bool calibrate();

private:
    float _x, _y, _z;  // Gyroscope values
    float _accelX, _accelY, _accelZ;  // Accelerometer values
    float _offsetX, _offsetY, _offsetZ;  // Gyro calibration offsets
    float _accelOffsetX, _accelOffsetY, _accelOffsetZ;  // Accel calibration offsets
    bool _initialized;
    TwoWire* _wire;
};

} // namespace Sensors
```

#### Example Usage

```cpp
#include "Sensors/Gyro.h"

Sensors::Gyro gyro;

void setup() {
    Serial.begin(115200);
    
    // Initialize gyroscope with SDA=14, SCL=15
    if (gyro.init(14, 15)) {
        Serial.println("Gyroscope initialized successfully");
        
        // Calibrate the sensor
        Serial.println("Calibrating gyroscope...");
        if (gyro.calibrate()) {
            Serial.println("Calibration successful");
        } else {
            Serial.println("Calibration failed");
        }
    } else {
        Serial.println("Gyroscope initialization failed");
    }
}

void loop() {
    // Update sensor readings
    gyro.update();
    
    // Print gyroscope values (degrees per second)
    Serial.printf("Gyro: X=%.2f Y=%.2f Z=%.2f deg/s\n",
                 gyro.getX(), gyro.getY(), gyro.getZ());
    
    // Print accelerometer values (g)
    Serial.printf("Accel: X=%.2f Y=%.2f Z=%.2f g\n",
                 gyro.getAccelX(), gyro.getAccelY(), gyro.getAccelZ());
    
    // Print acceleration magnitude
    Serial.printf("Magnitude: %.2f g\n", gyro.getAccelMagnitude());
    
    delay(100);
}
```

#### Implementation Details

The Gyro class uses the Utils::I2CManager to communicate with the MPU6050 sensor. Key initialization steps include:

1. Setting up the I2C bus:
```cpp
Utils::I2CManager::getInstance().initBus("base", sda, scl, 400000);
```

2. Waking up the MPU6050:
```cpp
Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1, 0);
```

3. Configuring the gyroscope (±250°/s range):
```cpp
Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_GYRO_CONFIG, 0x00);
```

4. Configuring the accelerometer (±2g range):
```cpp
Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_ACCEL_CONFIG, 0x00);
```

5. Configuring the digital low-pass filter:
```cpp
Utils::I2CManager::getInstance().writeRegister("base", MPU6050_ADDR, MPU6050_REG_CONFIG, 0x03);
```

The `update()` method reads raw sensor data and applies the following conversions:

- Gyroscope: Raw value / 131.0 to get degrees per second (±250°/s range)
- Accelerometer: Raw value / 16384.0 to get g (±2g range)

Acceleration magnitude is calculated as the Euclidean norm of the three acceleration components:
```cpp
sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ)
```

---

### CliffDetector

The `CliffDetector` class interfaces with infrared sensors to detect edges or cliffs, preventing falls.

#### Features

- Dual sensor support (left and right)
- Calibration capability
- Edge detection logic

#### API Reference

```cpp
namespace Sensors {

class CliffDetector {
public:
    CliffDetector();
    ~CliffDetector();

    bool init(int leftPin = 13, int rightPin = 12);
    void update();
    bool isLeftCliffDetected() const;
    bool isRightCliffDetected() const;
    bool isAnyCliffDetected() const;
    bool calibrate();

private:
    int _leftPin, _rightPin;
    bool _leftCliffDetected, _rightCliffDetected;
    int _leftThreshold, _rightThreshold;
    bool _initialized;
};

} // namespace Sensors
```

#### Example Usage

```cpp
#include "Sensors/CliffDetector.h"

Sensors::CliffDetector cliffDetector;

void setup() {
    Serial.begin(115200);
    
    // Initialize cliff detector with pins 13 and 12
    if (cliffDetector.init(13, 12)) {
        Serial.println("Cliff detector initialized successfully");
        
        // Calibrate the sensors
        Serial.println("Calibrating cliff sensors...");
        Serial.println("Place the robot on a flat surface");
        delay(2000);
        
        if (cliffDetector.calibrate()) {
            Serial.println("Calibration successful");
        } else {
            Serial.println("Calibration failed");
        }
    } else {
        Serial.println("Cliff detector initialization failed");
    }
}

void loop() {
    // Update sensor readings
    cliffDetector.update();
    
    // Check for cliffs
    if (cliffDetector.isAnyCliffDetected()) {
        Serial.println("Cliff detected!");
        
        if (cliffDetector.isLeftCliffDetected()) {
            Serial.println("Left cliff detected");
        }
        
        if (cliffDetector.isRightCliffDetected()) {
            Serial.println("Right cliff detected");
        }
        
        // Safety action
        // motors.move(Motors::MotorControl::BACKWARD, 500);
    }
    
    delay(50);
}
```

#### Implementation Details

The CliffDetector class uses analog infrared sensors to detect sudden changes in distance, which occur at edges. The class reads analog values from the sensors and compares them to calibrated thresholds to determine if an edge has been detected.

The calibration process involves:
1. Reading multiple samples from each sensor
2. Calculating an average value for each sensor
3. Setting thresholds based on these averages, typically with an offset (e.g., 80% of the average)

During normal operation, the `update()` method:
1. Reads the current sensor values
2. Compares them to the thresholds
3. Sets the cliff detection flags accordingly

### DistanceSensor

The `DistanceSensor` class interfaces with the HC-SR04 ultrasonic sensor to measure distances and detect obstacles.

#### Features

- Accurate distance measurement (2-400 cm)
- Obstacle detection with configurable threshold
- Error handling for out-of-range or invalid measurements

#### API Reference

```cpp
namespace Sensors {

class DistanceSensor {
public:
    DistanceSensor();
    ~DistanceSensor();

    /**
     * Initialize the distance sensor
     * @param triggerPin GPIO pin connected to the TRIG pin of the sensor
     * @param echoPin GPIO pin connected to the ECHO pin of the sensor
     * @param maxDistance Maximum distance to measure in centimeters (default: 400cm)
     * @return true if initialization was successful, false otherwise
     */
    bool init(int triggerPin, int echoPin, int maxDistance = 400);

    /**
     * Measure the distance
     * @return Distance in centimeters, or -1 if measurement failed
     */
    float measureDistance();

    /**
     * Check if an obstacle is detected within the specified range
     * @return true if an obstacle is detected within the threshold, false otherwise
     */
    bool isObstacleDetected();

private:
    int _triggerPin;
    int _echoPin;
    int _maxDistance;
    unsigned long _timeout; // Timeout in microseconds
    bool _initialized;
};

} // namespace Sensors
```

#### Example Usage

```cpp
#include "Sensors/DistanceSensor.h"

Sensors::DistanceSensor distanceSensor;

void setup() {
    Serial.begin(115200);
    
    // Initialize distance sensor with trigger pin 4 and echo pin 13
    if (distanceSensor.init(4, 13, 400)) {
        Serial.println("Distance sensor initialized successfully");
        
        // Test measurement
        float distance = distanceSensor.measureDistance();
        if (distance >= 0) {
            Serial.print("Initial distance: ");
            Serial.print(distance);
            Serial.println(" cm");
        } else {
            Serial.println("Initial measurement failed");
        }
    } else {
        Serial.println("Distance sensor initialization failed");
    }
}

void loop() {
    // Measure distance
    float distance = distanceSensor.measureDistance();
    
    if (distance >= 0) {
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
        
        // Check for obstacles
        if (distanceSensor.isObstacleDetected()) {
            Serial.println("Obstacle detected! Stopping robot");
            // motors.move(Motors::MotorControl::STOP);
        }
    } else {
        Serial.println("Measurement failed");
    }
    
    delay(500);
}
```

#### WebSocket API

The distance sensor can be accessed through the WebSocket API using the following commands:

1. Request distance measurement:
```json
{
  "type": "distance_request"
}
```

2. Response format:
```json
{
  "type": "sensor_data",
  "data": {
    "distance": {
      "value": 45.2,
      "unit": "cm",
      "valid": true,
      "obstacle": false
    }
  }
}
```

3. Periodic sensor updates (part of sensor monitoring task):
```json
{
  "type": "sensor_update",
  "data": {
    "distance": {
      "value": 45.2,
      "unit": "cm",
      "valid": true,
      "obstacle": false
    },
    // other sensor data...
  }
}
```

#### Implementation Details

The DistanceSensor class uses the HC-SR04 ultrasonic sensor to measure distances using sound waves. The class manages the timing of trigger pulses and echo signals to calculate distances.

The measurement process involves:
1. Sending a 10μs pulse to the trigger pin
2. Measuring the duration of the echo signal
3. Converting the time to distance using the speed of sound
4. Validating the measurement against minimum and maximum ranges

The obstacle detection feature provides a simplified interface for detecting objects within a specific distance threshold, which can be used for collision avoidance algorithms.

---
