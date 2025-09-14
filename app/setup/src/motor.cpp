#include <Arduino.h>
#include "setup/setup.h"

Motors::MotorControl *motors;

void setupMotors() {
  motors = new Motors::MotorControl();
  if (MOTOR_ENABLED) {
    logger->info("Setting up motors...");
    #if MOTOR_IO_EXTENDER
    if (motors->initWithExtender(&oExpander, LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2)) {
    #else
    if (motors->init(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2)) {
    #endif
      logger->info("Motors initialized successfully");
      motors->move(Motors::MotorControl::FORWARD, 100);
      motors->move(Motors::MotorControl::BACKWARD, 100);
      motors->move(Motors::MotorControl::LEFT, 100);
      motors->move(Motors::MotorControl::RIGHT, 100);
      motors->stop();
    } else {
      logger->error("Motors initialization failed");
    }
  }
}