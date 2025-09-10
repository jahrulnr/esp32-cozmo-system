#include "setup/setup.h"
#include "web/Routes/routes.h"

Application* app;
CsvDatabase* database;
Router* webRouter = nullptr;

void setupWebServer() {
	if (webRouter) {
		return;
	}

	// Initialize application
	app = Application::getInstance(LittleFS);
	app->setDeviceName(deviceName);
	
	// Boot the framework
	app->boot();
    
	// Set up mDNS responder for local name resolution
	if (MDNS.begin(deviceName)) {
			logger->info("mDNS responder started: %s.local", deviceName);
			// Add service to mDNS
			MDNS.addService("http", "tcp", 80);
	} else {
			logger->info("Error setting up mDNS responder");
	}
    
	// Initialize CSV database first (needed for Configuration model)
	database = new CsvDatabase(LittleFS);
	Model::setDatabase(database);

	webRouter = app->getRouter();
	registerWebRoutes(webRouter);
	registerApiRoutes(webRouter);
	registerWebSocketRoutes(webRouter);

	// Start the application
	app->run();
}