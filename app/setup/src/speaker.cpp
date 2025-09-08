#include <Arduino.h>
#include <vector>
#include "setup/setup.h"
#include "tasks/register.h"

I2SSpeaker *i2sSpeaker;
AudioSamples *audioSamples;

void setupSpeakers() {
  logger->info("Setting up speakers...");
  
  #if SPEAKER_ENABLED
  logger->info("Initializing I2S speaker (MAX98357)...");
  i2sSpeaker = new I2SSpeaker(
    I2S_SPEAKER_DATA_PIN,
    I2S_SPEAKER_BCLK_PIN, 
    I2S_SPEAKER_WCLK_PIN
  );
  
  if (i2sSpeaker->init(I2S_SPEAKER_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO) == ESP_OK) {
    logger->info("I2S speaker (MAX98357) initialized successfully");
    
    // Now initialize dependent components AFTER i2sSpeaker is created
    audioSamples = new AudioSamples(i2sSpeaker);
    if (!MP3Player::init(i2sSpeaker)) {
      logger->error("MP3Player initialization failed");
    } else {
      logger->info("MP3Player initialized successfully");
    }
    
    // Test with a simple beep first
    if (audioSamples) {
      audioSamples->playSample(AudioSamples::BEEP_SHORT);
    }
  } else {
    logger->error("I2S speaker (MAX98357) initialization failed");
    delete i2sSpeaker;
    i2sSpeaker = nullptr;
    audioSamples = nullptr;
  }
  
  #else
  logger->info("Speakers disabled in configuration");
  i2sSpeaker = nullptr;
  audioSamples = nullptr;
  #endif
}