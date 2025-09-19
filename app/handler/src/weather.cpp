#include "../register.h"

void weatherHandler() {
	static long needUpdate = millis();
	const long updateFrequency = 60000;
	const char* TAG = "weatherTask";

	if (WiFi.status() != WL_CONNECTED)
		return;

	if(millis() > needUpdate) {
		weatherService->getCurrentWeather(weatherCallback, false);
		needUpdate = millis() + updateFrequency;
	}
}