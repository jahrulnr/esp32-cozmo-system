#include <Arduino.h>
#include "app.h"

// Microphone thresholds and timing
const int SOUND_DETECTION_THRESHOLD = MICROPHONE_SOUND_THRESHOLD;
const int LOUD_SOUND_THRESHOLD = 3000;
const int QUIET_SOUND_THRESHOLD = 1500;

// Timing for sound detection
unsigned long lastSoundCheck = 0;
const unsigned long SOUND_CHECK_INTERVAL = 100; // Check every 100ms

// Timing for sound behaviors
unsigned long lastSoundBehavior = 0;
const unsigned long SOUND_BEHAVIOR_COOLDOWN = 5000; // 5 seconds between sound reactions

// Sound detection states
bool soundDetected = false;
bool loudSoundDetected = false;
int soundLevel = 0;
int peakSoundLevel = 0;

void setupMicrophone() {
  logger->info("Setting up MAX9814 microphone sensor...");
  
  #if MICROPHONE_ENABLED
  microphoneSensor = new Sensors::MicrophoneSensor(
    MICROPHONE_ANALOG_PIN, 
    MICROPHONE_GAIN_PIN, 
    MICROPHONE_AR_PIN
  );
  
  if (microphoneSensor->init()) {
    logger->info("MAX9814 microphone sensor initialized successfully");
    
    // Set gain to middle level (50dB)
    microphoneSensor->setGain(HIGH);
    
    // Set fast attack/release for better responsiveness
    microphoneSensor->setAttackRelease(false);
    
    logger->info("Microphone baseline calibration completed");
  } else {
    logger->error("MAX9814 microphone sensor initialization failed");
    delete microphoneSensor;
    microphoneSensor = nullptr;
  }
  #else
  logger->info("Microphone sensor disabled in configuration");
  microphoneSensor = nullptr;
  #endif
}

// Check microphone levels and detect sound events
void checkMicrophone() {
  #if MICROPHONE_ENABLED
  if (!microphoneSensor || !microphoneSensor->isInitialized()) {
    return;
  }
  
  // Only check microphone at certain intervals
  unsigned long currentTime = millis();
  if (currentTime - lastSoundCheck < SOUND_CHECK_INTERVAL) {
    return;
  }
  lastSoundCheck = currentTime;
  
  // Read current sound level
  soundLevel = microphoneSensor->readLevel();
  
  // Read peak level over a short duration for better detection
  peakSoundLevel = microphoneSensor->readPeakLevel(50);
  
  // Check for sound detection
  soundDetected = microphoneSensor->isSoundDetected(SOUND_DETECTION_THRESHOLD);
  loudSoundDetected = microphoneSensor->isSoundDetected(LOUD_SOUND_THRESHOLD);
  
  // Log sound levels periodically for debugging
  static unsigned long lastLogTime = 0;
  if (currentTime - lastLogTime > 5000) { // Log every 5 seconds
    logger->debug("Sound levels - Current: " + String(soundLevel) + 
                  ", Peak: " + String(peakSoundLevel) + 
                  ", Detected: " + String(soundDetected ? "Yes" : "No"));
    lastLogTime = currentTime;
  }
  
  // React to sound if automation is enabled and cooldown has passed
  if (soundDetected && automation && _enableAutomation && 
      (currentTime - lastSoundBehavior > SOUND_BEHAVIOR_COOLDOWN)) {
    
    String soundBehavior = "";
    
    if (loudSoundDetected) {
      logger->info("Loud sound detected! Level: " + String(peakSoundLevel));
      soundBehavior = "look around nervously, show surprised expression";
    } else if (peakSoundLevel > QUIET_SOUND_THRESHOLD) {
      logger->info("Moderate sound detected! Level: " + String(peakSoundLevel));
      soundBehavior = "turn head towards sound, show curious expression";
    } else {
      logger->info("Quiet sound detected! Level: " + String(peakSoundLevel));
      soundBehavior = "perk up slightly, show alert expression";
    }
    
    // if (!soundBehavior.isEmpty()) {
    //   automation->addBehavior(soundBehavior);
    //   lastSoundBehavior = currentTime;
      
    //   // Update manual control time to prevent immediate automation override
    //   updateManualControlTime();
    // }
  }
  #endif
}

// Get current sound level for WebSocket updates
int getCurrentSoundLevel() {
  #if MICROPHONE_ENABLED
  return soundLevel;
  #else
  return 0;
  #endif
}

// Get peak sound level for WebSocket updates
int getPeakSoundLevel() {
  #if MICROPHONE_ENABLED
  return peakSoundLevel;
  #else
  return 0;
  #endif
}

// Check if sound is currently detected
bool isSoundDetected() {
  #if MICROPHONE_ENABLED
  return soundDetected;
  #else
  return false;
  #endif
}

// Calibrate microphone baseline (useful for different environments)
void calibrateMicrophone() {
  #if MICROPHONE_ENABLED
  if (!microphoneSensor || !microphoneSensor->isInitialized()) {
    logger->warning("Cannot calibrate microphone - sensor not initialized");
    return;
  }
  
  logger->info("Calibrating microphone baseline...");
  int baseline = microphoneSensor->calibrateBaseline(MICROPHONE_BASELINE_CALIBRATION_TIME);
  logger->info("Microphone baseline calibrated to: " + String(baseline));
  #endif
}

// Set microphone gain level
void setMicrophoneGain(int gainLevel) {
  #if MICROPHONE_ENABLED
  if (!microphoneSensor || !microphoneSensor->isInitialized()) {
    logger->warning("Cannot set microphone gain - sensor not initialized");
    return;
  }
  
  microphoneSensor->setGain(gainLevel);
  String gainStr = (gainLevel == LOW) ? "40dB" : 
                   (gainLevel == HIGH) ? "50dB" : "60dB";
  logger->info("Microphone gain set to: " + gainStr);
  #endif
}
