#include <setup/setup.h>

Communication::WeatherService* weatherService = nullptr;

void setupWeather(){
	if (weatherService) return;

	Communication::WeatherService::WeatherConfig cfg;
	cfg.province = Communication::WeatherService::Province::DKI_JAKARTA;
	cfg.cityCode = static_cast<int>(Communication::WeatherService::JakartaCity::JAKARTA_BARAT);
	cfg.cacheExpiryMinutes = 60;

	weatherService = new Communication::WeatherService(fileManager);
	weatherService->init(cfg);

	if (WiFi.status() == WL_CONNECTED) {
		weatherService->getCurrentWeather(weatherCallback, true);
	}
}