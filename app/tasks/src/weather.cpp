#include <tasks/register.h>

TaskHandle_t weatherServiceTaskHandle;

void weatherServiceTask(void* param) {
	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(60000);

	do {
		weatherService->getCurrentWeather(weatherCallback, false);

		vTaskDelayUntil(&lastWakeTime, updateFrequency);
	}while(1);
}