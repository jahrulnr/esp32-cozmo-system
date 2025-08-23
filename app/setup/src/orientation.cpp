#include <Arduino.h>
#include "setup/setup.h"

void setupOrientation() {
  if (ORIENTATION_ENABLED) {
    logger->info("Setting up gyroscope...");
    orientation = new Sensors::OrientationSensor();
    if (orientation->init(ORIENTATION_SDA_PIN, ORIENTATION_SCL_PIN)) {
      orientation->calibrate();
      orientation->setGyroRange(Sensors::GYRO_RANGE_2000_DEG);
      orientation->setAccelRange(Sensors::ACCEL_RANGE_16G);
      logger->info("Gyroscope initialized successfully");
    } else {
      logger->error("Gyroscope initialization failed");
      orientation = nullptr;
    }
  }
}