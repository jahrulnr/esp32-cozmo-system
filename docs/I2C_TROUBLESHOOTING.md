# I2C Connectivity Troubleshooting Guide

This document provides a comprehensive guide for troubleshooting I2C connectivity issues with the Cozmo-System platform.

## Table of Contents
- [Common I2C Issues](#common-i2c-issues)
- [Diagnostic Procedures](#diagnostic-procedures)
- [Using the I2CScanner Utility](#using-the-i2cscanner-utility)
- [Bus Health Analysis](#bus-health-analysis)
- [Hardware-Specific Troubleshooting](#hardware-specific-troubleshooting)
- [Advanced Solutions](#advanced-solutions)

## Common I2C Issues

### 1. Device Not Responding

**Symptoms:**
- No devices found during I2C scan
- Error code 2 (NACK on address) returned during transmission
- `0x00` or `0xFF` values when reading from registers

**Possible Causes:**
- Incorrect wiring (SDA/SCL swapped or not connected)
- Missing pull-up resistors
- Incorrect device address
- Device not powered or insufficient power
- Voltage level mismatch
- Damaged device

**Solutions:**
- Verify connections with a multimeter (continuity test)
- Check device power (measure voltage at VCC pin)
- Add appropriate pull-up resistors (typically 4.7kΩ)
- Review device datasheet for correct address
- Use a logic analyzer to verify signal integrity

### 2. Intermittent Communication

**Symptoms:**
- Occasional timeouts or errors
- Works sometimes but fails randomly
- Error code 3 (NACK on data) returned during transmission

**Possible Causes:**
- Loose connections
- Electrical noise/interference
- Too long wires
- Insufficient pull-up resistors
- Multiple devices with the same address

**Solutions:**
- Secure all connections
- Use twisted pair wires and/or shielding
- Keep I2C lines short (<30cm recommended)
- Add capacitors near devices for better power stability
- Use I2C multiplexer for devices with address conflicts

### 3. Slow Communication

**Symptoms:**
- Very long response times (>1ms)
- System appears sluggish
- Works but unreliably

**Possible Causes:**
- Excessive capacitance on the bus
- Pull-up resistors too weak
- Clock stretching issues
- Low I2C frequency setting

**Solutions:**
- Optimize pull-up resistor values (try 2.2kΩ or 1.8kΩ)
- Reduce bus capacitance (shorter wires, fewer devices)
- Increase I2C clock frequency (if devices support it)
- Check for firmware issues related to clock stretching

## Using I2CScanner for Diagnosis

The Utils::I2CScanner class provides several tools for diagnosing I2C issues:

### Basic Scanning

```cpp
// Scan the bus for any responding devices
int deviceCount = Utils::I2CScanner::scan();
```

### Connection Testing

```cpp
// Test connection quality to a specific device
Utils::I2CScanner::testDeviceConnection(0x68);
```

### Comprehensive Diagnosis

```cpp
// Get detailed information about connection issues
Utils::I2CScanner::diagnoseConnectionIssues(0x68);
```

### Addressing Multiple Buses

```cpp
// Test a different I2C bus (using Wire1)
Utils::I2CScanner::initAndScan(SDA2_PIN, SCL2_PIN, 100000, Wire1);
Utils::I2CScanner::diagnoseConnectionIssues(0x68, Wire1);
```

## Hardware Troubleshooting

### 1. Verifying Pull-Up Resistors

1. Disconnect all devices from the I2C bus
2. Power off the system
3. Measure resistance between:
   - SDA and VCC (should be your pull-up value)
   - SCL and VCC (should be your pull-up value)
4. If resistance is much lower, check for shorts
5. If resistance is much higher, pull-ups are missing

### 2. Signal Quality Check

Use a logic analyzer or oscilloscope to check:
1. SDA and SCL signal rise times (should be <1μs for 100kHz I2C)
2. Signal levels (should reach >70% of VCC)
3. Clock stretching (look for held-low SCL periods)
4. Start and stop conditions (proper timing)

### 3. Bus Capacitance Test

Excessive capacitance is a common issue with I2C:
1. Disconnect all devices except one reference device
2. Measure signal rise time with oscilloscope
3. Calculate rough capacitance: C = t / (R * 0.8) where:
   - t = rise time in seconds
   - R = pull-up resistor value in ohms
4. Total capacitance should be <400pF for standard I2C

## ESP32-CAM Specific Issues

### 1. GPIO Pin Selection

Not all pins on the ESP32-CAM are suitable for I2C:
- **Preferred I2C pins:** GPIO 14 (SCL), GPIO 15 (SDA)
- **Avoid:** GPIO 0 (affects boot mode), GPIO 1 & 3 (UART), GPIO 16 (PSRAM)

### 2. Power Management

The ESP32-CAM can experience power issues affecting I2C:
- Use a stable 5V supply capable of >500mA
- Add a 100μF capacitor between VCC and GND near the ESP32
- Power off the camera when not in use (high current draw)

### 3. ESP32 Pull-Up Configuration

The ESP32 has built-in programmable pull-ups that can be enabled:
```cpp
Wire.begin(SDA_PIN, SCL_PIN);
pinMode(SDA_PIN, INPUT_PULLUP);  // Enable internal pull-up
pinMode(SCL_PIN, INPUT_PULLUP);  // Enable internal pull-up
```

Note: These internal pull-ups are weak (~47kΩ) and often insufficient alone for reliable I2C communication. External pull-ups are still recommended.

## Advanced Debugging Techniques

### 1. I2C Protocol Analyzer

Use a logic analyzer with I2C protocol decoding to:
- Capture and analyze full I2C transactions
- Identify timing violations
- Detect addressing or arbitration issues
- Verify data integrity

### 2. Isolation Testing

Test components individually:
1. Disconnect all devices except one
2. Test communication with the isolated device
3. Add devices one by one, testing after each addition
4. Identify which device causes issues when added

### 3. Frequency Adjustment

Try different I2C frequencies:
```cpp
// Try lower frequency for problematic devices
Wire.setClock(50000);  // 50kHz

// Or higher for better performance with good connections
Wire.setClock(400000); // 400kHz (Fast mode)
```

## Resources

- [I2C Specification](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)
- [Adafruit I2C Tutorial](https://learn.adafruit.com/i2c-addresses/overview)
- [ESP32 I2C Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)
