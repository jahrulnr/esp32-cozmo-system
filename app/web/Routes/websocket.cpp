#include "routes.h"

void registerWebSocketRoutes(Router* router) {
		router->websocket("/ws")
				.onConnect([](WebSocketRequest& request) {
						Serial.printf("[WebSocket] client %u connected\n", request.clientId());
						
						// Send welcome message
						JsonDocument welcome;
						welcome["type"] = "welcome";
						welcome["message"] = "Connected websocket";
						
						String welcomeMsg;
						serializeJson(welcome, welcomeMsg);
						request.send(welcomeMsg);
				})
				.onDisconnect([](WebSocketRequest& request) {
						Serial.printf("[WebSocket] client %u disconnected\n", request.clientId());
				})
				.onMessage([](WebSocketRequest& request, const String& message) {
						JsonDocument doc;
						DeserializationError error = deserializeJson(doc, message);
						
						if (error) {
								Serial.println("[WebSocket] Invalid JSON received");
								return;
						}
						
						String command = doc["command"].as<String>();
						if (command == "ping") {
								// Respond with pong
								JsonDocument pongResponse;
								pongResponse["type"] = "pong";
								pongResponse["timestamp"] = millis();
								
								String pongMsg;
								serializeJson(pongResponse, pongMsg);
								request.send(pongMsg);
						}
				});
}
