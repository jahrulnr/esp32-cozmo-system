#pragma once

#include <Arduino.h>
#ifdef CONFIG_IDF_TARGET_ESP32
extern "C" {
#include "esp_system.h"
uint8_t temprature_sens_read(); // Note: original ESP32 function has typo in name
}
#elif SOC_TEMP_SENSOR_SUPPORTED
#include "driver/temperature_sensor.h"
#endif

namespace Sensors {

/**
 * TemperatureSensor class
 * 
 * Provides a unified interface for reading temperature from
 * different ESP32 boards (ESP32CAM and ESP32S3).
 */
class TemperatureSensor {
public:
    /**
     * Constructor
     */
    TemperatureSensor();

    /**
     * Destructor
     */
    ~TemperatureSensor();

    /**
     * Initialize the temperature sensor
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool init();

    /**
     * Read the temperature from the sensor
     * 
     * @return Temperature in Celsius or NAN if reading fails
     */
    float readTemperature();

    /**
     * Check if the temperature sensor is supported on this device
     * 
     * @return true if temperature sensor is supported, false otherwise
     */
    bool isSupported();

private:
    bool _initialized;
    
    #if SOC_TEMP_SENSOR_SUPPORTED
    // Handle for the ESP32S3 temperature sensor
    temperature_sensor_handle_t _tempSensor;
    #endif
};

} // namespace Sensors
