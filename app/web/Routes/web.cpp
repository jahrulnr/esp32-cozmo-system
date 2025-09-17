#include "routes.h"

void registerWebRoutes(Router* router) {
		AuthController* authController = new AuthController();

		// Single-page application route
		router->get("/", [](Request& request) -> Response {
				// Serve the app.html as the main entry point
				if (LittleFS.exists("/views/app.html")) {

						return Response(request.getServerRequest())
								.file("/views/app.html");
				}

				return Response(request.getServerRequest())
						.content("no content available")
						.status(404);
		}).name("app");

		// Authentication routes
		router->get("/login", [](Request& request) -> Response {
				// Redirect to the main app with login hash
				return Response(request.getServerRequest())
						.redirect("/#login");
		}).name("login.show");

		router->post("/login", [authController](Request& request) -> Response {
				return authController->login(request);
		}).name("login");

		router->post("/logout", [authController](Request& request) -> Response {
				return authController->logout(request);
		}).name("logout");

		// Protected routes (client-side auth check)
		router->get("/dashboard", [](Request& request) -> Response {
				// Redirect to the main app with dashboard hash
				return Response(request.getServerRequest())
						.redirect("/#dashboard");
		}).name("dashboard");

		// Static file serving for CSS, JS, and other assets
		router->get("/assets/{file}", [](Request& request) -> Response {
				String path = "/assets/" + request.route("file");

				return Response(request.getServerRequest())
						.file(path.c_str());
		}).name("assets");


		router->get("/favicon.ico", [](Request& request) -> Response {
				return Response(request.getServerRequest())
						.file("/favicon.ico");
		});
}