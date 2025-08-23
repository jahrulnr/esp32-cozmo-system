#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

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

AnalogMicrophone* amicrophone = nullptr;
I2SMicrophone* microphone;

void setupMicrophone() {
  logger->info("Setting up MAX9814 microphone sensor...");
  
  #if MICROPHONE_ENABLED
  #if MICROPHONE_I2S
  if (!microphone) {
    microphone = new I2SMicrophone(
        (gpio_num_t)MICROPHONE_DIN,    // Data pin
        (gpio_num_t)MICROPHONE_SCK,    // Clock pin  
        (gpio_num_t)MICROPHONE_WS,     // Word select pin
        I2S_NUM_1                      // Port 
    );
    esp_err_t ret = microphone->init(16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    if (ret != ESP_OK) {
        Serial.printf("[setupI2SMicrophone] ERROR: Failed to initialize I2S Standard driver: %s\n", esp_err_to_name(ret));
        return;
    }
    
    // Start the I2S channel
    ret = microphone->start();
    if (ret != ESP_OK) {
        Serial.printf("[setupI2SMicrophone] ERROR: Failed to start I2S Standard driver: %s\n", esp_err_to_name(ret));
        return;
    }
  }
  #elif MICROPHONE_ANALOG
  if (!amicrophone) {
      amicrophone = new AnalogMicrophone(MICROPHONE_ANALOG_PIN, MICROPHONE_GAIN_PIN, MICROPHONE_AR_PIN);
      
      esp_err_t ret = amicrophone->init();
      if (ret != ESP_OK) {
          logger->error("[setupAnalogMicrophone] ERROR: Failed to start analog microphone: %s\n", esp_err_to_name(ret));
          return;
      }
      amicrophone->setGain(INPUT);
      amicrophone->setAttackRelease(true);
  }
  #endif
  #else
  logger->info("Microphone sensor disabled in configuration");
  microphoneSensor = nullptr;
  #endif
  delay(1000);  
}