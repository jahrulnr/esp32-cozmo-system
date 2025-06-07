#include <Arduino.h>
#include "app.h"

void setupMotors() {
  if (MOTOR_ENABLED) {
    logger->info("Setting up motors...");
    motors = new Motors::MotorControl();
    if (motors->init(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2)) {
      motors->setSpeed(MOTOR_SPEED_DEFAULT);
      logger->info("Motors initialized successfully");
      motors->move(Motors::MotorControl::FORWARD);
      delay(500);
      motors->move(Motors::MotorControl::BACKWARD);
      delay(500);
      motors->move(Motors::MotorControl::LEFT);
      delay(500);
      motors->move(Motors::MotorControl::RIGHT);
      delay(500);
      motors->stop();
    } else {
      logger->error("Motors initialization failed");
    }
  }
}