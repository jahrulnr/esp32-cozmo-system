#include "tasks/register.h"

void ftpTask(void* param) {
	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(100);
	while(1){
		vTaskDelayUntil(&lastWakeTime, updateFrequency);
		ftpSrv.handleFTP();
	}
}