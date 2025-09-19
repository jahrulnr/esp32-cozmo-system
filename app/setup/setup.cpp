#include "setup.h"

void setupApp() {
  heap_caps_malloc_extmem_enable(0);
	setupLogger();
	setupFilemanager();
  setupNotification();
  setupDisplay();

  heap_caps_malloc_extmem_enable(128);
	setupCamera();
  setupSpeechRecognition();
  heap_caps_malloc_extmem_enable(0);

  // Initialize components
  setupExtender();
  setupCliffDetector();
  setupOrientation();
  setupMotors();
  setupServos();
  setupMicrophone();
  setupSpeakers();
  setupCommandMapper();
  // setupAutomation();
  setupPicoTTS();
  setupAudioRecorder();
  setupNotePlayer();

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