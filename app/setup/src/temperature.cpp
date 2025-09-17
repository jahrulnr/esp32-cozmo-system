#include <Arduino.h>
#include "setup/setup.h"

// Temperature thresholds
const float TEMP_HIGH_THRESHOLD = 30.0;
const float TEMP_LOW_THRESHOLD = 10.0;

// Last time we checked the temperature
unsigned long lastTemperatureCheck = 0;
const unsigned long TEMPERATURE_CHECK_INTERVAL = 30000; // 30 seconds

// Last time we triggered a temperature behavior
unsigned long lastTemperatureBehavior = 0;
const unsigned long TEMPERATURE_BEHAVIOR_COOLDOWN = 300000; // 5 minutes

Sensors::TemperatureSensor *temperatureSensor;

void setupTemperatureSensor() {
  logger->info("Setting up temperature sensor...");

  temperatureSensor = new Sensors::TemperatureSensor();
  if (temperatureSensor->init()) {
    logger->info("Temperature sensor initialized successfully");
  } else {
    logger->warning("Temperature sensor initialization failed or not supported on this device");
  }
}
