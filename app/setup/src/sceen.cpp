#include <Arduino.h>
#include "setup/setup.h"

Display::Display* display = nullptr;

void setupDisplay() {
  display = new Display::Display();
#if SCREEN_ENABLED
  logger->info("Setting up display...");
  if (display->init(SCREEN_SDA_PIN, SCREEN_SCL_PIN, SCREEN_WIDTH, SCREEN_HEIGHT)) {
    display->clear();
    display->drawCenteredText(20, "Cozmo System");
    display->drawCenteredText(40, "Starting...");
    display->update();
    logger->info("Screen initialized successfully");
  } else {
    logger->error("Screen initialization failed");
  }
#endif
}