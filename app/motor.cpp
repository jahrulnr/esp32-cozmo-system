#include <Arduino.h>
#include "init.h"

void setupMotors() {
  if (MOTOR_ENABLED) {
    logger->info("Setting up motors...");
    motors = new Motors::MotorControl();
    if (motors->init(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2)) {
      motors->setSpeed(MOTOR_SPEED_DEFAULT);
      logger->info("Motors initialized successfully");
    } else {
      logger->error("Motors initialization failed");
    }
  }
}