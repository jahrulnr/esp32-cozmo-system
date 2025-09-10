#include <tasks/register.h>

void weatherServiceTask(void* param) {
	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(60000);
	const char* TAG = "weatherTask";

	int check = 0;
	do {
		bool connected = false;
		if (WiFi.status() != WL_CONNECTED) {
			ESP_LOGW(TAG, "Waiting connection. attempt: %d", check +1);
			vTaskDelay(pdMS_TO_TICKS(1000));

			if (check == 3) {
				vTaskDelete(NULL);
			}
		}
		else {
			connected = true;
			break;
		}
	}while(check++ < 3);

	do {
		weatherService->getCurrentWeather(weatherCallback, false);

		vTaskDelayUntil(&lastWakeTime, updateFrequency);
	}while(1);
}