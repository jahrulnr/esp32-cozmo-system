#include <callback/register.h>

void weatherCallback(const Communication::WeatherService::WeatherData &data, bool success){
	static const char* TAG = "weatherCallback";

	if (success) {
		ESP_LOGI(TAG, "Weather data received successfully:");
		ESP_LOGI(TAG, "Location: %s", data.location.c_str());
		ESP_LOGI(TAG, "Description: %s", data.description.c_str());
		ESP_LOGI(TAG, "Condition: %s", Communication::WeatherService::conditionToString(data.condition).c_str());
		ESP_LOGI(TAG, "Temperature: %d°C", data.temperature);
		ESP_LOGI(TAG, "Humidity: %d%%", data.humidity);
		ESP_LOGI(TAG, "Wind Speed: %d km/h", data.windSpeed);
		ESP_LOGI(TAG, "Wind Direction: %s", data.windDirection.c_str());
		ESP_LOGI(TAG, "Last Updated: %s", data.lastUpdated.c_str());
		ESP_LOGI(TAG, "Valid: %s", data.isValid ? "true" : "false");
	} else {
		ESP_LOGE(TAG, "Failed to retrieve weather data");
	}
}