#include "setup.h"

void setupApp() {
	setupLogger();
	setupFilemanager();

  // Initialize components
  setupScreen();
  setupExtender();
  setupWiFi();
  setupMotors();
  setupServos();
  setupOrientation();
  setupDistanceSensor();
  setupCliffDetector();
  setupTemperatureSensor();
  setupMicrophone();
  setupSpeakers();
  setupWebServer();
  setupWebSocket();
  setupGPT();
  setupCommandMapper();
  setupAutomation();
  setupCamera();

  if (motors && screen)
    motors->setScreen(screen);
  if (servos && screen)
    servos->setScreen(screen);
  
  logger->info("System initialization complete");
  
  if (screen) {
    screen->clear();
    screen->drawCenteredText(20, "Cozmo System");
    screen->drawCenteredText(40, "Ready!");
    screen->update();
  }
  
  if (!playSpeakerMP3File("/audio/boot.mp3")) {
    logger->error("Play boot mp3 failed");
  };
}