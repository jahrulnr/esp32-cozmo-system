#include <Arduino.h>
#include <soc/soc.h>
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "setup/setup.h"
#include "tasks/register.h"

void setup() {
  heap_caps_malloc_extmem_enable(1024 * 8);

  // Initialize Serial
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n\nCozmo System Starting...");

  setCpuFrequencyMhz(240);

  setupApp();
  setupTasks();
}

void loop() {
  vTaskDelete(NULL);
}
