#include "setup.h"

void setupApp() {
	setupLogger();
	setupFilemanager();
  setupNotification();
  setupScreen();

  // Initialize components
  setupWiFi();
  setupExtender();
  setupOrientation();
  setupMotors();
  setupServos();
  setupDistanceSensor();
  setupCliffDetector();
  setupTouchDetector();
  setupTemperatureSensor();
  setupMicrophone();
  setupSpeakers();
  setupGPT();
  setupCommandMapper();
  setupAutomation();
  setupFTPServer();
  setupPicoTTS();
  setupSpeechRecognition();
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
}