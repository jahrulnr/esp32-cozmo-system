#include <Arduino.h>
#include "setup/setup.h"

void setupScreen() {
  screen = new Screen::Screen(logger);
  if (SCREEN_ENABLED) {
    logger->info("Setting up screen...");
    if (screen->init(SCREEN_SDA_PIN, SCREEN_SCL_PIN)) {
      screen->clear();
      screen->drawCenteredText(20, "Cozmo System");
      screen->drawCenteredText(40, "Starting...");
      screen->update();
      logger->info("Screen initialized successfully");
    } else {
      logger->error("Screen initialization failed");
    }
  }
}