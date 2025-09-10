#include <setup/setup.h>

Services::WeatherService* weatherService = nullptr;

void setupWeather(){
	if (weatherService) return;

	Services::WeatherService::WeatherConfig cfg;
	cfg.adm4Code = "31.71.03.1001"; // Kemayoran, Jakarta Pusat
	cfg.cacheExpiryMinutes = 60;

	weatherService = new Services::WeatherService(fileManager);
	weatherService->init(cfg);

	if (WiFi.status() == WL_CONNECTED) {
		weatherService->getCurrentWeather(weatherCallback, true);
	}
}