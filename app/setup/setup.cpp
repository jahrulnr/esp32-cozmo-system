#include "setup.h"

void setupApp() {
	setupLogger();
	setupFilemanager();
  setupNotification();
  setupDisplay();

  // Initialize components
  setupExtender();
  setupCliffDetector();
  setupOrientation();
  setupMotors();
  setupServos();
  setupDistanceSensor();
  setupTouchDetector();
  setupTemperatureSensor();
  setupBatteryManager();
  setupMicrophone();
  setupSpeakers();
  setupCommandMapper();
  // setupAutomation();
  setupPicoTTS();
  setupSpeechRecognition();
  setupAudioRecorder();
  setupNotePlayer();
  setupCamera();
  setupPedestrian();

  delay(10);

  setupWiFi();
  setupGPT();
  setupFTPServer();
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