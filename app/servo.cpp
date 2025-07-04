#include <Arduino.h>
#include "app.h"

void setupServos() {
  if (SERVO_ENABLED) {
    logger->info("Setting up servos...");
    servos = new Motors::ServoControl();
    if (servos->init(HEAD_SERVO_PIN, HAND_SERVO_PIN)) {
      servos->setScreen(screen);
      delay(500);
      servos->setHead(DEFAULT_HEAD_ANGLE);
      delay(50);
      servos->setHand(DEFAULT_HAND_ANGLE);
      logger->info("Servos initialized successfully");
    } else {
      logger->error("Servos initialization failed");
    }
  }
}