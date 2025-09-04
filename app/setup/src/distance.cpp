#include <Arduino.h>
#include "setup/setup.h"

Sensors::DistanceSensor *distanceSensor;

void setupDistanceSensor() {
  if (ULTRASONIC_ENABLED) {
    logger->info("Setting up HC-SR04 ultrasonic distance sensor...");
    distanceSensor = new Sensors::DistanceSensor();
    
    if (distanceSensor->init(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MAX_DISTANCE)) {
    	distanceSensor->setThresHold(ULTRASONIC_OBSTACLE_TRESHOLD);
      logger->info("HC-SR04 initialized successfully");
      
      // Perform a test measurement
      delay(1000);
      float distance = distanceSensor->measureDistance();
      if (distance >= 0) {
        logger->info("Initial distance measurement: " + String(distance, 2) + " cm");
      } else {
        logger->warning("Initial distance measurement failed");
      }
    } else {
      logger->error("HC-SR04 initialization failed");
    }
  } else {
    logger->info("HC-SR04 distance sensor disabled in config");
  }
}
