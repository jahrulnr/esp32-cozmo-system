#include <Arduino.h>
#include <sdkconfig.h>
#include "esp_log.h"
#include <soc/soc.h>
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "setup/setup.h"
#include "tasks/register.h"
#include <esp_task_wdt.h>

void setup() {
  setCpuFrequencyMhz(240);
  heap_caps_malloc_extmem_enable(128);

  // Initialize Serial
  Serial.begin(SERIAL_BAUD_RATE);
  if (Serial){
    Serial.println("Cozmo System Starting...");
  }

  esp_err_t esp_task_wdt_reconfigure(const esp_task_wdt_config_t *config);
  esp_task_wdt_config_t config = {
    .timeout_ms = 120 * 1000,
    .trigger_panic = true,
  };
  esp_task_wdt_reconfigure(&config);

  LittleFS.begin(false);
  setupApp();
  setupTasksCpu0();
  setupTasksCpu1();
}

void loop() {
  vTaskDelete(NULL);

  // static long lastTime = millis() + 10000;
  // static int i = 0;
  // if (lastTime <= millis()) {
  //   lastTime = millis() + 10000;

  //   switch (i) {
  //     case 0:
  //       commandMapper->executeCommandString("[TEXT=Time for a twist!][FACE_GLEE=5s][HAND_POSITION=135][TURN_LEFT=3s][HAND_CENTER][TURN_RIGHT=3s]");
  //     break;
  //     case 1:
  //       commandMapper->executeCommandString("[TEXT=Big finish spin!][FACE_HAPPY=6s][LOOK_TOP][HEAD_POSITION=130][HAND_POSITION=60][TURN_LEFT=3s][TURN_RIGHT=3s][HEAD_CENTER][HAND_CENTER]");
  //     break;
  //     case 2:
  //       commandMapper->executeCommandString("[TEXT=Let’s start the fun!][FACE_HAPPY=5s][LOOK_FRONT][HAND_UP][HEAD_POSITION=70][MOVE_FORWARD=5s][HAND_DOWN][MOVE_BACKWARD=5s]");
  //     break;
  //     case 3:
  //       commandMapper->executeCommandString("[TEXT=Back and forth we go!][FACE_HAPPY=5s][BLINK][HEAD_CENTER][HAND_POSITION=180][MOVE_FORWARD=5s][MOVE_BACKWARD=5s]");
  //     break;
  //   }
  //   i++;

  //   if (i > 3) i = 0;
  //   delay(1000);
  // }
}
