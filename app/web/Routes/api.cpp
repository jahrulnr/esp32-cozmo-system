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
								JsonDocument response;
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
								JsonDocument response;
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
						
						// System restart (admin only)
						system.post("/restart", [](Request& request) -> Response {
								return SystemController::restart(request);
						}).name("api.system.restart");
				});
		});
}