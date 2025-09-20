#include <callback/register.h>

void weatherCallback(const Services::WeatherService::WeatherData &data, bool success){
	static const char* TAG = "weatherCallback";

	if (success) {
		ESP_LOGI(TAG, "Location=%s, Temperature=%d°C", data.location.c_str(), data.temperature);

		// Update display with weather data
		if (display) {
			display->updateWeatherData(data);
		}
	} else {
		ESP_LOGE(TAG, "Failed to retrieve weather data");
	}
}