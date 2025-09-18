#include "routes.h"

void registerApiRoutes(Router* router) {
		// API routes with middleware
		router->group("/api/v1", [&](Router& api) {
				api.middleware({"cors", "json", "ratelimit"});

				// Auth routes for user info (register first to avoid conflicts)
				api.group("/auth", [&](Router& auth) {
						AuthController* authController = new AuthController();

						auth.get("/user", [authController](Request& request) -> Response {
								return authController->getUserInfo(request);
						}).name("api.auth.user");

						auth.post("/password", [authController](Request& request) -> Response {
								// Password update endpoint (not implemented yet)
								Utils::SpiJsonDocument response;
								response["success"] = false;
								response["message"] = "Password update not implemented yet";

								return Response(request.getServerRequest())
										.status(200)
										.json(response);
						}).name("api.auth.password");
				});

				// Admin routes
				api.group("/admin", [&](Router& admin) {
						admin.middleware({"auth", "admin", "json"}); // Middleware to check if user is admin

						AuthController* authController = new AuthController();

						admin.get("/users", [authController](Request& request) -> Response {
								// Get all users (not implemented yet)
								Utils::SpiJsonDocument response;
								response["success"] = true;
								response["users"] = JsonArray();

								// Return demo data
								JsonObject user1 = response["users"].add<JsonObject>();
								user1["id"] = 1;
								user1["name"] = "Admin User";
								user1["username"] = "admin";
								user1["role"] = "admin";
								user1["active"] = true;

								return Response(request.getServerRequest())
										.status(200)
										.json(response);
						}).name("api.admin.users");
				});

				// System routes
				api.group("/system", [&](Router& system) {
						system.middleware({"auth", "admin"}); // Require authentication

						// Get system statistics
						system.get("/stats", [](Request& request) -> Response {
								return SystemController::getStats(request);
						}).name("api.system.stats");

						// Get detailed memory information
						system.get("/memory", [](Request& request) -> Response {
								return SystemController::getMemoryInfo(request);
						}).name("api.system.memory");

						// Get network information
						system.get("/network", [](Request& request) -> Response {
								return SystemController::getNetworkInfo(request);
						}).name("api.system.network");

						// Get hostname information
						system.get("/hostname", [](Request& request) -> Response {
								return SystemController::getHostname(request);
						}).name("api.system.hostname.get");

						// Update hostname
						system.post("/hostname", [](Request& request) -> Response {
								return SystemController::updateHostname(request);
						}).name("api.system.hostname.update");

						// Get all configurations
						system.get("/configurations", [](Request& request) -> Response {
								return SystemController::getConfigurations(request);
						}).name("api.system.configs.get");

						// Update a configuration
						system.post("/configuration", [](Request& request) -> Response {
								return SystemController::updateConfiguration(request);
						}).name("api.system.configs.update");

						// Get battery status
						system.get("/battery", [](Request& request) -> Response {
								return SystemController::getBatteryStatus(request);
						}).name("api.system.battery");

						// System restart (admin only)
						system.post("/restart", [](Request& request) -> Response {
								return SystemController::restart(request);
						}).name("api.system.restart");
				});

				// Robot control routes
				api.group("/robot", [&](Router& robot) {
						robot.middleware("auth"); // Require authentication for robot control

						// Motor control
						robot.post("/motor/move", [](Request& request) -> Response {
								return RobotController::moveMotor(request);
						}).name("api.robot.motor.move");

						robot.post("/motor/stop", [](Request& request) -> Response {
								return RobotController::stopMotor(request);
						}).name("api.robot.motor.stop");

						robot.post("/motor/speed", [](Request& request) -> Response {
								return RobotController::setMotorSpeed(request);
						}).name("api.robot.motor.speed");

						// Servo control
						robot.post("/servo/position", [](Request& request) -> Response {
								return RobotController::setServoPosition(request);
						}).name("api.robot.servo.position");

						robot.get("/servo/position", [](Request& request) -> Response {
								return RobotController::getServoPosition(request);
						}).name("api.robot.servo.position.get");

						robot.post("/servo/center", [](Request& request) -> Response {
								return RobotController::centerServos(request);
						}).name("api.robot.servo.center");

						// Sensor data
						robot.get("/sensors", [](Request& request) -> Response {
								return RobotController::getSensorData(request);
						}).name("api.robot.sensors");

						// Emergency control
						robot.post("/emergency/stop", [](Request& request) -> Response {
								return RobotController::emergencyStop(request);
						}).name("api.robot.emergency.stop");

						// Voice control
						robot.post("/voice/toggle", [](Request& request) -> Response {
								return RobotController::toggleVoiceControl(request);
						}).name("api.robot.voice.toggle");

						// Chat/AI
						robot.post("/chat/message", [](Request& request) -> Response {
								return RobotController::sendChatMessage(request);
						}).name("api.robot.chat.message");
				});

				// WiFi management routes
				api.group("/wifi", [&](Router& wifi) {
						wifi.middleware("auth"); // Require authentication

						wifi.get("/status", [](Request& request) -> Response {
								return SystemController::getNetworkInfo(request);
						}).name("api.wifi.status");

						wifi.get("/scan", [](Request& request) -> Response {
								// WiFi network scanning - would need to implement
								Utils::SpiJsonDocument response;
								response["success"] = true;
								response["networks"] = JsonArray();
								
								// Demo networks
								JsonObject net1 = response["networks"].add<JsonObject>();
								net1["ssid"] = "Home_WiFi";
								net1["rssi"] = -45;
								net1["encryption"] = "WPA2";
								
								JsonObject net2 = response["networks"].add<JsonObject>();
								net2["ssid"] = "Guest_Network";
								net2["rssi"] = -65;
								net2["encryption"] = "Open";
								
								return Response(request.getServerRequest())
										.status(200)
										.json(response);
						}).name("api.wifi.scan");
				});
		});
}