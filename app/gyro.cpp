#include <Arduino.h>
#include "init.h"

void setupGyro() {
  if (GYRO_ENABLED) {
    logger->info("Setting up gyroscope...");
    gyro = new Sensors::Gyro();
    if (gyro->init(GYRO_SDA_PIN, GYRO_SCL_PIN)) {
      gyro->calibrate();
      logger->info("Gyroscope initialized successfully");
    } else {
      logger->error("Gyroscope initialization failed");
      gyro = nullptr;
    }
  }
}