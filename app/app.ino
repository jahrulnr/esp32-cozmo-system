#include <Arduino.h>
#include "esp_log.h"
#include <soc/soc.h>
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "setup/setup.h"
#include "tasks/register.h"
#include <esp_task_wdt.h>

void setup() {
  setCpuFrequencyMhz(240);
  heap_caps_malloc_extmem_enable(1024);

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

  setupApp();
  setupTasks();
}

void loop() {
  vTaskDelete(NULL);
}
