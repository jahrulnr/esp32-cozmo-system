#include <setup/setup.h>

Communication::WeatherService* weatherService = nullptr;

void setupWeather(){
	if (weatherService) return;

	Communication::WeatherService::WeatherConfig cfg;
	cfg.adm4Code = "31.71.03.1001"; // Kemayoran, Jakarta Pusat
	cfg.cacheExpiryMinutes = 60;

	weatherService = new Communication::WeatherService(fileManager);
	weatherService->init(cfg);

	if (WiFi.status() == WL_CONNECTED) {
		weatherService->getCurrentWeather(weatherCallback, true);
	}
}