#include "routes.h"

void registerWebSocketRoutes(Router* router) {
	router->websocket("/ws")
		.onConnect([](WebSocketRequest& request) {
		  uint32_t clientId = request.clientId();
			Serial.printf("[WebSocket] client %u connected\n", request.clientId());
			Utils::Sstring ip = request.clientIP();
			logger->info("WebSocket client #%d connected from %s", clientId, ip.c_str());
			sessions[clientId % 5].authenticated = false;

			// Send welcome message
			Utils::SpiJsonDocument welcome;
			welcome["type"] = "welcome";
			welcome["message"] = "Connected websocket";

			String welcomeMsg;
			serializeJson(welcome, welcomeMsg);
			request.send(welcomeMsg.c_str());
		})
		.onDisconnect([](WebSocketRequest& request) {
		  uint32_t clientId = request.clientId();
      logger->info("WebSocket client #%d disconnected", clientId);
      // Clean up session data
      sessions[clientId % 5].authenticated = false;
		})
		.onMessage([](WebSocketRequest& request, const String& message) {
			Utils::SpiJsonDocument doc;
			DeserializationError error = deserializeJson(doc, message);
		  uint32_t clientId = request.clientId();

			if (error) {
				Serial.println("[WebSocket] Invalid JSON received");
				return;
			}

			Utils::Sstring type = doc["type"].as<String>();
			JsonVariant data = doc["data"];
			Utils::Sstring version = doc["version"] | "0.0";

			if (type == "login") {

			}
			else if (sessions[clientId % 5].authenticated) {

			}
		});
}
