#include "setup/setup.h"
#include "web/Routes/routes.h"

AsyncWebServer* webServer = nullptr;
Router* webRouter = nullptr;

void setupWebServer() {
	if (!webServer) {
		webServer = new AsyncWebServer(80);
	}

	if (!webRouter) {
		webRouter = new Router(webServer);
		registerWebRoutes(webRouter);
		registerApiRoutes(webRouter);
		registerWebSocketRoutes(webRouter);
	}
}