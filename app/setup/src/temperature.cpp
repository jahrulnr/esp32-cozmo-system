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

// Check temperature and execute appropriate template behaviors
void checkTemperature() {
  // Only check temperature at certain intervals
  unsigned long currentTime = millis();
  if (currentTime - lastTemperatureCheck < TEMPERATURE_CHECK_INTERVAL) {
    return;
  }
  lastTemperatureCheck = currentTime;
  
  // Skip if temperature sensor is not initialized
  if (!temperatureSensor) {
    return;
  }
  
  float temperature = temperatureSensor->readTemperature();
  
  // Skip invalid temperature readings
  if (isnan(temperature)) {
    logger->warning("Invalid temperature reading");
    return;
  }
  
  logger->debug("Current temperature: " + String(temperature, 1) + "°C");
  
  // Check if we should trigger a behavior (respect cooldown period)
  if (currentTime - lastTemperatureBehavior < TEMPERATURE_BEHAVIOR_COOLDOWN) {
    return;
  }
  
  // Execute behavior based on temperature
  bool behaviorTriggered = false;
  
  if (temperature > TEMP_HIGH_THRESHOLD) {
    logger->info("High temperature detected: " + String(temperature, 1) + "°C");
    behaviorTriggered = true;
  }
  else if (temperature < TEMP_LOW_THRESHOLD) {
    logger->info("Low temperature detected: " + String(temperature, 1) + "°C");
    behaviorTriggered = true;
  }
  
  // Update last behavior time if a behavior was triggered
  if (behaviorTriggered) {
    lastTemperatureBehavior = currentTime;
  }
}
