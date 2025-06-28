#include <Arduino.h>
#include "app.h"

// Microphone thresholds and timing
const int SOUND_DETECTION_THRESHOLD = MICROPHONE_SOUND_THRESHOLD;
const int LOUD_SOUND_THRESHOLD = 3000;
const int QUIET_SOUND_THRESHOLD = 1500;

// Voice recording thresholds
const int VOICE_START_THRESHOLD = 2000;  // Level to start recording
const int VOICE_STOP_THRESHOLD = 1000;   // Level to stop recording (lower than start for hysteresis)
const unsigned long VOICE_START_DURATION = 200;  // ms of continuous sound to start recording
const unsigned long VOICE_STOP_DURATION = 1000;  // ms of silence to stop recording
const unsigned long MAX_RECORDING_TIME = 10000;  // Maximum recording time (10 seconds)

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

// Voice recording states
bool isRecording = false;
bool voiceDetected = false;
unsigned long voiceStartTime = 0;
unsigned long voiceSilenceStartTime = 0;
unsigned long recordingStartTime = 0;
unsigned long lastVoiceActivityTime = 0;

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
  
  // Voice Activity Detection (VAD) for recording
  checkVoiceActivity(currentTime);
  
  // Log sound levels periodically for debugging
  static unsigned long lastLogTime = 0;
  if (currentTime - lastLogTime > 5000) { // Log every 5 seconds
    logger->debug("Sound levels - Current: " + String(soundLevel) + 
                  ", Peak: " + String(peakSoundLevel) + 
                  ", Detected: " + String(soundDetected ? "Yes" : "No") +
                  ", Recording: " + String(isRecording ? "Yes" : "No"));
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

// Voice Activity Detection (VAD) for recording trigger
void checkVoiceActivity(unsigned long currentTime) {
  #if MICROPHONE_ENABLED
  if (!microphoneSensor || !microphoneSensor->isInitialized()) {
    return;
  }
  
  bool currentVoiceActivity = (soundLevel > VOICE_START_THRESHOLD);
  
  // State machine for voice activity detection
  if (!isRecording) {
    // Not currently recording - check if we should start
    if (currentVoiceActivity) {
      if (!voiceDetected) {
        // Start of potential voice activity
        voiceDetected = true;
        voiceStartTime = currentTime;
        logger->debug("Voice activity started, level: " + String(soundLevel));
      } else {
        // Continuous voice activity - check duration
        if (currentTime - voiceStartTime >= VOICE_START_DURATION) {
          // Start recording!
          startVoiceRecording(currentTime);
        }
      }
    } else {
      // No voice activity
      voiceDetected = false;
    }
  } else {
    // Currently recording - check if we should stop
    if (currentVoiceActivity) {
      // Voice activity continues
      lastVoiceActivityTime = currentTime;
      voiceSilenceStartTime = 0; // Reset silence timer
    } else {
      // No voice activity - start silence timer
      if (voiceSilenceStartTime == 0) {
        voiceSilenceStartTime = currentTime;
      } else {
        // Check if silence duration exceeded threshold
        if (currentTime - voiceSilenceStartTime >= VOICE_STOP_DURATION) {
          stopVoiceRecording(currentTime);
        }
      }
    }
    
    // Safety: Stop recording if maximum time exceeded
    if (currentTime - recordingStartTime >= MAX_RECORDING_TIME) {
      logger->warning("Voice recording stopped - maximum time exceeded");
      stopVoiceRecording(currentTime);
    }
  }
  #endif
}

// Start voice recording
void startVoiceRecording(unsigned long currentTime) {
  #if MICROPHONE_ENABLED
  if (isRecording) {
    return; // Already recording
  }
  
  isRecording = true;
  recordingStartTime = currentTime;
  lastVoiceActivityTime = currentTime;
  voiceSilenceStartTime = 0;
  
  logger->info("Voice recording started - Sound level: " + String(soundLevel));
  
  // Trigger automation response for voice detection
  if (automation && _enableAutomation) {
    // automation->addBehavior("voice detected, show listening expression");
  }
  
  // Play a short beep to indicate recording started
  if (SPEAKER_ENABLED) {
    playSpeakerBeep(30); // Low volume beep
  }
  
  // TODO: Start actual audio recording/buffering here
  // This is where you would implement:
  // - Audio buffer allocation
  // - Start capturing audio samples
  // - Prepare for speech recognition
  
  #endif
}

// Stop voice recording
void stopVoiceRecording(unsigned long currentTime) {
  #if MICROPHONE_ENABLED
  if (!isRecording) {
    return; // Not recording
  }
  
  unsigned long recordingDuration = currentTime - recordingStartTime;
  isRecording = false;
  voiceDetected = false;
  
  logger->info("Voice recording stopped - Duration: " + String(recordingDuration) + "ms");
  
  // Play a different beep to indicate recording stopped
  if (SPEAKER_ENABLED) {
    playSpeakerTone(800, 100, 25); // Higher pitch, short beep
  }
  
  // Only process if recording was long enough to be meaningful
  if (recordingDuration >= 500) { // At least 500ms
    processVoiceRecording(recordingDuration);
  } else {
    logger->debug("Voice recording too short, ignoring");
  }
  
  // TODO: Stop actual audio recording here
  // This is where you would implement:
  // - Stop capturing audio samples
  // - Process the recorded audio
  // - Send to speech recognition
  // - Clean up audio buffers
  
  #endif
}

// Process the recorded voice
void processVoiceRecording(unsigned long duration) {
  #if MICROPHONE_ENABLED
  logger->info("Processing voice recording (" + String(duration) + "ms)");
  
  // Trigger automation response
  if (automation && _enableAutomation) {
    // automation->addBehavior("processing voice command, show thinking expression");
  }
  
  // TODO: Implement actual voice processing here
  // This could include:
  // - Send audio to cloud speech recognition (Google Speech-to-Text, etc.)
  // - Local keyword detection for "hey cozmo"
  // - Pattern matching for simple commands
  // - Integration with GPT for natural language processing
  
  // For now, just log that we detected voice activity
  logger->info("Voice activity detected - ready for speech recognition integration");
  
  #endif
}

// Get voice recording status for WebSocket updates
bool isVoiceRecording() {
  #if MICROPHONE_ENABLED
  return isRecording;
  #else
  return false;
  #endif
}

// Get voice activity status for WebSocket updates
bool isVoiceDetected() {
  #if MICROPHONE_ENABLED
  return voiceDetected;
  #else
  return false;
  #endif
}

// Manually trigger voice recording (for testing or remote activation)
void triggerVoiceRecording() {
  #if MICROPHONE_ENABLED
  if (!isRecording) {
    startVoiceRecording(millis());
    logger->info("Voice recording triggered manually");
  }
  #endif
}

// Stop voice recording manually
void stopVoiceRecordingManual() {
  #if MICROPHONE_ENABLED
  if (isRecording) {
    stopVoiceRecording(millis());
    logger->info("Voice recording stopped manually");
  }
  #endif
}
