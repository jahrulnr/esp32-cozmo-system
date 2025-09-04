#include "setup/setup.h"
#include "web/Routes/routes.h"

Application* app;
CsvDatabase* database;
Router* webRouter = nullptr;
String deviceName = "pio-esp32-cam";

void setupWebServer() {
	if (webRouter) {
		return;
	}

	// Initialize application
	app = Application::getInstance(LittleFS);
	app->setDeviceName(deviceName.c_str());
	
	// Boot the framework
	app->boot();
    
	// Set up mDNS responder for local name resolution
	if (MDNS.begin(deviceName.c_str())) {
			Serial.println("mDNS responder started: " + deviceName + ".local");
			// Add service to mDNS
			MDNS.addService("http", "tcp", 80);
	} else {
			Serial.println("Error setting up mDNS responder");
	}
    
	// Initialize CSV database first (needed for Configuration model)
	database = new CsvDatabase(LittleFS);
	Model::setDatabase(database);

	// Initialize database tables
	Configuration::initTable();

	webRouter = app->getRouter();
	registerWebRoutes(webRouter);
	registerApiRoutes(webRouter);
	registerWebSocketRoutes(webRouter);

	// Start the application
	app->run();
}