# I2C Compatible Devices for Cozmo-System

This document lists I2C devices that are compatible with the Cozmo-System platform along with their addresses, basic usage information, and implementation tips. These devices are supported by the I2CManager and I2CScanner utilities and have been tested with the platform.

## Table of Contents
- [Motion Sensors](#motion-sensors)
- [Display Modules](#display-modules)
- [Environmental Sensors](#environmental-sensors)
- [ADC and DAC Modules](#adc-and-dac-modules)
- [Motor Controllers](#motor-controllers)
- [Input Devices](#input-devices)
- [Storage and RTC Modules](#storage-and-rtc-modules)
- [Communication Bridges](#communication-bridges)

## Device Compatibility Matrix

| Device Category | Device Name | Address | Cozmo-System Support Level | Power Requirement | I2C Speed Support |
|----------------|-------------|---------|---------------------|-----------------|-----------------|
| Motion Sensors | MPU6050 | 0x68/0x69 | Full | 3.3V | 400kHz |
| Motion Sensors | MPU9250 | 0x68/0x69 | Full | 3.3V | 400kHz |
| Display | SSD1306 | 0x3C/0x3D | Full | 3.3V/5V | 400kHz |
| Environmental | BME280 | 0x76/0x77 | Full | 3.3V | 400kHz |
| Environmental | BMP280 | 0x76/0x77 | Full | 3.3V | 400kHz |
| ADC | ADS1115 | 0x48-0x4B | Full | 3.3V/5V | 400kHz |
| Motor | PCA9685 | 0x40 | Full | 3.3V/5V | 100kHz |
| Input | MPR121 | 0x5A | Partial | 3.3V | 100kHz |
| Storage | AT24C32 | 0x50 | Full | 3.3V/5V | 400kHz |
| RTC | DS3231 | 0x68 | Partial | 3.3V | 100kHz |
| Bridge | MCP2221 | 0x00 | Experimental | 5V | 100kHz |

_Support Levels: Full = Fully tested and supported; Partial = Basic functionality tested; Experimental = Under development_

## Motion Sensors

### MPU6050 - 6-Axis Accelerometer and Gyroscope
- **Address**: 0x68 (alternate: 0x69 when AD0 pin is HIGH)
- **Library**: [Adafruit MPU6050](https://github.com/adafruit/Adafruit_MPU6050)
- **Usage**:
  ```cpp
  #include <Adafruit_MPU6050.h>
  
  Adafruit_MPU6050 mpu;
  mpu.begin();  // Uses the default address 0x68
  
  // Or with I2CManager
  auto& i2cManager = Utils::I2CManager::getInstance();
  TwoWire* wire = i2cManager.getBus("main");
  mpu.begin(0x68, wire);
  ```

### MPU9250 - 9-Axis Motion Sensor
- **Address**: 0x68 (alternate: 0x69 when AD0 pin is HIGH)
- **Library**: [MPU9250 by hideakitai](https://github.com/hideakitai/MPU9250)
- **Magnetometer Address**: 0x0C (accessed through MPU9250)

### BNO055 - 9-Axis Orientation Sensor
- **Address**: 0x28 (alternate: 0x29)
- **Library**: [Adafruit BNO055](https://github.com/adafruit/Adafruit_BNO055)
- **Features**: Fusion sensor with accelerometer, gyroscope, and magnetometer with built-in sensor fusion

## Environmental Sensors

### BME280 - Temperature, Humidity, Pressure Sensor
- **Address**: 0x76 (alternate: 0x77)
- **Library**: [Adafruit BME280](https://github.com/adafruit/Adafruit_BME280_Library)
- **Usage**:
  ```cpp
  #include <Adafruit_BME280.h>
  
  Adafruit_BME280 bme;
  bme.begin();
  
  float temp = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;  // hPa
  float humidity = bme.readHumidity();
  ```

### BMP280 - Barometric Pressure & Temperature Sensor
- **Address**: 0x76 (alternate: 0x77)
- **Library**: [Adafruit BMP280](https://github.com/adafruit/Adafruit_BMP280_Library)

### SHT31 - Temperature & Humidity Sensor
- **Address**: 0x44 (alternate: 0x45)
- **Library**: [Adafruit SHT31](https://github.com/adafruit/Adafruit_SHT31)

### BH1750 - Light Intensity Sensor
- **Address**: 0x23 (alternate: 0x5C)
- **Library**: [BH1750 by claws](https://github.com/claws/BH1750)

## Displays

### SSD1306 - OLED Display
- **Address**: 0x3C (for 128x32) or 0x3D (for alternate address)
- **Library**: [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- **Usage**:
  ```cpp
  #include <Adafruit_SSD1306.h>
  
  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 64
  
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Cozmo System"));
  display.display();
  ```

### SH1106 - OLED Display
- **Address**: 0x3C
- **Library**: [U8g2](https://github.com/olikraus/u8g2)

## Motor & Servo Controllers

### PCA9685 - 16-Channel PWM/Servo Controller
- **Address**: 0x40 (configurable up to 0x7F)
- **Library**: [Adafruit PWM Servo Driver](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)
- **Usage**:
  ```cpp
  #include <Adafruit_PWMServoDriver.h>
  
  Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
  pwm.begin();
  pwm.setPWMFreq(50);  // For servos, typically 50Hz
  
  // Move servo on channel 0 to position
  pwm.setPWM(0, 0, 300);  // ~90 degrees
  ```

### DRV8830 - DC Motor Controller
- **Address**: 0x60 (configurable up to 0x67)
- **Library**: [Adafruit DRV8830](https://github.com/adafruit/Adafruit_DRV8830)

## ADC & DAC

### ADS1115 - 16-bit ADC
- **Address**: 0x48 (configurable to 0x49, 0x4A, or 0x4B)
- **Library**: [Adafruit ADS1X15](https://github.com/adafruit/Adafruit_ADS1X15)
- **Usage**:
  ```cpp
  #include <Adafruit_ADS1X15.h>
  
  Adafruit_ADS1115 ads;
  ads.begin();
  ads.setGain(GAIN_ONE);  // 1x gain (±4.096V range)
  
  int16_t adc0 = ads.readADC_SingleEnded(0);
  float volts = ads.computeVolts(adc0);
  ```

### MCP4725 - 12-bit DAC
- **Address**: 0x60 (alternate: 0x61)
- **Library**: [Adafruit MCP4725](https://github.com/adafruit/Adafruit_MCP4725)

## Storage & RTC

### AT24C32 - EEPROM
- **Address**: 0x50 (configurable up to 0x57)
- **Library**: [AT24Cx](https://github.com/cyberp/AT24Cx)

### DS3231 - Real-Time Clock
- **Address**: 0x68
- **Library**: [RTClib by Adafruit](https://github.com/adafruit/RTClib)
- **Usage**:
  ```cpp
  #include <RTClib.h>
  
  RTC_DS3231 rtc;
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  ```

## Input Devices

### MPR121 - 12-Key Capacitive Touch Sensor
- **Address**: 0x5A (configurable up to 0x5D)
- **Library**: [Adafruit MPR121](https://github.com/adafruit/Adafruit_MPR121)

### TCA8418 - Keypad Decoder
- **Address**: 0x34
- **Library**: [SparkFun TCA8418](https://github.com/sparkfun/SparkFun_TCA8418_Arduino_Library)

## Interface Adapters

### TCA9548A - 8-Channel I2C Multiplexer
- **Address**: 0x70 (configurable up to 0x77)
- **Library**: [Adafruit TCA9548](https://github.com/adafruit/Adafruit_TCA9548)
- **Usage**:
  ```cpp
  #include <Adafruit_TCA9548.h>
  
  Adafruit_TCA9548 tca;
  
  void setup() {
    if (!tca.begin()) {
      Serial.println("Couldn't find TCA9548!");
    }
    
    // Select I2C bus 0
    tca.openChannel(0);
    // Now talk to a device on that bus...
    
    // When done, you can switch to another bus
    tca.openChannel(2);
    
    // Or close all channels
    tca.openChannel(TCA_CHANNEL_NONE);
  }
  ```

### PCF8574 - 8-bit I/O Expander
- **Address**: 0x20 (configurable up to 0x27)
- **Library**: [PCF8574 by xreef](https://github.com/xreef/PCF8574_library)

## Multi-Sensor Modules

### GY-87 - 10DOF Module (MPU6050 + HMC5883L + BMP180)
- **MPU6050 Address**: 0x68
- **HMC5883L Address**: 0x1E (connected through MPU6050 auxiliary bus)
- **BMP180 Address**: 0x77

## Integrated with Cozmo-System

### Default I2C Bus Configuration
```cpp
// In Config.h
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define I2C_FREQUENCY 400000

// In init.h
void initI2C() {
  Utils::I2CManager& i2cManager = Utils::I2CManager::getInstance();
  i2cManager.initBus("main", I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
  
  // Scan for connected devices
  Utils::I2CScanner::scan();
}
```

### Example: Adding MPU6050 to Cozmo-System

```cpp
// In app.ino or sensors.cpp
#include <Adafruit_MPU6050.h>

Adafruit_MPU6050 mpu;

bool initMPU6050() {
  Utils::I2CManager& i2cManager = Utils::I2CManager::getInstance();
  TwoWire* wire = i2cManager.getBus("main");
  
  // Check if device is present
  if (!i2cManager.devicePresent("main", 0x68)) {
    logger.error("MPU6050 not found!");
    return false;
  }
  
  // Initialize using the wire instance from I2CManager
  if (!mpu.begin(0x68, wire)) {
    logger.error("Failed to initialize MPU6050");
    return false;
  }
  
  // Configure the sensor
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  logger.info("MPU6050 initialized successfully");
  return true;
}

void readMPU6050() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Now use the values as needed
  // a.acceleration.x, a.acceleration.y, a.acceleration.z
  // g.gyro.x, g.gyro.y, g.gyro.z
  // temp.temperature
}
```

## Troubleshooting I2C Devices

For detailed I2C troubleshooting information, see [I2C_TROUBLESHOOTING.md](/docs/I2C_TROUBLESHOOTING.md)
