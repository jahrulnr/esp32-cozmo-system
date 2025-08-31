#include "setup.h"

void setupApp() {
	setupLogger();
	setupFilemanager();
  setupNotification();

  // Initialize components
  setupCamera();
  setupExtender();
  setupScreen();
  setupOrientation();
  setupWiFi();
  setupMotors();
  setupServos();
  setupDistanceSensor();
  setupCliffDetector();
  setupTemperatureSensor();
  setupSpeakers();
  setupMicrophone();
  setupGPT();
  setupCommandMapper();
  setupAutomation();
  setupSpeechRecognition();
  setupPicoTTS();

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
}