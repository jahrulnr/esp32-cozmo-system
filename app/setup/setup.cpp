#include "setup.h"

void setupApp() {
	setupLogger();
	setupFilemanager();
  setupNotification();
  setupDisplay();

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

  delay(10);
  setupWebServer();

  if (motors && display)
    motors->setDisplay(display);
  if (servos && display)
    servos->setDisplay(display);

  delay(10);
  setupWeather();
  
  logger->info("System initialization complete");
  
  if (display) {
    display->clear();
    display->drawCenteredText(20, "Cozmo System");
    display->drawCenteredText(40, "Ready!");
    display->update();
  }
}