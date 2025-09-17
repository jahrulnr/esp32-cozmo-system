#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

AnalogMicrophone* amicrophone = nullptr;
I2SMicrophone* microphone = nullptr;

void setupMicrophone() {
  logger->info("Setting up MAX9814 microphone sensor...");

  #if MICROPHONE_ENABLED
  #if MICROPHONE_I2S
  if (!microphone) {
    microphone = new I2SMicrophone(
        MICROPHONE_DIN,    // Data pin
        MICROPHONE_SCK,    // Clock pin
        MICROPHONE_WS,     // Word select pin
        I2S_NUM_1          // Port
    );
    esp_err_t ret = microphone->init(16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    if (ret != ESP_OK) {
        logger->error("[setupI2SMicrophone] ERROR: Failed to initialize I2S Standard driver: %s\n", esp_err_to_name(ret));
        return;
    }

    // Start the I2S channel
    ret = microphone->start();
    if (ret != ESP_OK) {
        logger->error("[setupI2SMicrophone] ERROR: Failed to start I2S Standard driver: %s\n", esp_err_to_name(ret));
        return;
    }
  }
  #elif MICROPHONE_ANALOG
  if (!amicrophone) {
      amicrophone = new AnalogMicrophone(MICROPHONE_ANALOG_PIN, MICROPHONE_GAIN_PIN, MICROPHONE_AR_PIN);

      bool ret = amicrophone->init();
      if (!ret) {
          logger->error("[setupAnalogMicrophone] ERROR: Failed to start analog microphone: %s\n", esp_err_to_name(ret));
          return;
      }
      amicrophone->setGain(LOW);
      amicrophone->setAttackRelease(true);
  }
  #endif
  #else
  logger->info("Microphone sensor disabled in configuration");
  #endif
  delay(1000);
}