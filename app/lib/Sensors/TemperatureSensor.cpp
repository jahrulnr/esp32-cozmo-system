#include "TemperatureSensor.h"

namespace Sensors {

TemperatureSensor::TemperatureSensor() : _initialized(false) {
#if SOC_TEMP_SENSOR_SUPPORTED
    _tempSensor = NULL;
#endif
}

TemperatureSensor::~TemperatureSensor() {
#if SOC_TEMP_SENSOR_SUPPORTED
    if (_initialized && _tempSensor != NULL) {
        temperature_sensor_disable(_tempSensor);
        temperature_sensor_uninstall(_tempSensor);
    }
#endif
}

bool TemperatureSensor::init() {
    if (_initialized) {
        return true; // Already initialized
    }

#if SOC_TEMP_SENSOR_SUPPORTED
    // For ESP32-S3 and other chips with SOC_TEMP_SENSOR_SUPPORTED
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    
    if (temperature_sensor_install(&temp_sensor_config, &_tempSensor) != ESP_OK) {
        return false;
    }
    
    if (temperature_sensor_enable(_tempSensor) != ESP_OK) {
        temperature_sensor_uninstall(_tempSensor);
        _tempSensor = NULL;
        return false;
    }
    
    _initialized = true;
    return true;
#elif CONFIG_IDF_TARGET_ESP32
    // For original ESP32, the sensor is always available
    _initialized = true;
    return true;
#else
    // Temperature sensor not supported on this device
    return false;
#endif
}

float TemperatureSensor::readTemperature() {
    if (!_initialized && !init()) {
        return NAN; // Return NaN if initialization failed
    }

#if CONFIG_IDF_TARGET_ESP32
    // ESP32 specific implementation
    return (temprature_sens_read() - 32) / 1.8;
#elif SOC_TEMP_SENSOR_SUPPORTED
    // ESP32-S3 and other supported chips
    float temp = NAN;
    if (_tempSensor != NULL) {
        if (temperature_sensor_get_celsius(_tempSensor, &temp) != ESP_OK) {
            return NAN;
        }
    }
    return temp;
#else
    return NAN; // Not supported
#endif
}

bool TemperatureSensor::isSupported() {
#if CONFIG_IDF_TARGET_ESP32 || SOC_TEMP_SENSOR_SUPPORTED
    return true;
#else
    return false;
#endif
}

} // namespace Sensors
