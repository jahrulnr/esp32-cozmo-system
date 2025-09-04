#include <Arduino.h>
#include "setup/setup.h"

Screen::Screen* screen = nullptr;

void setupScreen() {
  screen = new Screen::Screen();
#if SCREEN_ENABLED
  logger->info("Setting up screen...");
  if (screen->init(SCREEN_SDA_PIN, SCREEN_SCL_PIN, SCREEN_WIDTH, SCREEN_HEIGHT)) {
    screen->clear();
    screen->drawCenteredText(20, "Cozmo System");
    screen->drawCenteredText(40, "Starting...");
    screen->update();
    logger->info("Screen initialized successfully");
  } else {
    logger->error("Screen initialization failed");
  }
#endif
}