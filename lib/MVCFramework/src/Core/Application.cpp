#include "Application.h"
#include "Config.h"
#include "ServiceContainer.h"
#include "../Routing/Router.h"
#include "../Http/Request.h"
#include "../Http/Response.h"
#include <SPIFFS.h>
#include <memory>
#include <ArduinoJson.h>

Application* Application::instance = nullptr;

Application* Application::getInstance() {
    if (instance == nullptr) {
        instance = new Application();
    }
    return instance;
}

void Application::boot() {
    if (booted) return;
    
    Serial.println("Booting ESP32 MVC Framework...");
    
    // Initialize SPIFFS for file storage
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    
    // Initialize core services
    config = std::make_unique<Config>();
    container = std::make_unique<ServiceContainer>();
    
    // Load configuration
    config->load();
    
    // Create web server
    AsyncWebServer* server = new AsyncWebServer(config->getServerPort());
    router = std::make_unique<Router>(server);
    
    // Register core services
    registerProviders();
    registerMiddleware();
    
    booted = true;
    Serial.println("Framework booted successfully!");
}

void Application::run() {
    if (!booted) {
        boot();
    }
    
    Serial.println("Starting web server...");
    
    // Register routes
    registerRoutes();
    
    // Initialize router
    router->init();
    
    Serial.printf("Server started on port %d\n", config->getServerPort());
    Serial.printf("Environment: %s\n", config->getAppEnv().c_str());
    Serial.printf("Debug mode: %s\n", config->isDebug() ? "enabled" : "disabled");
}

void Application::registerProviders() {
    // Register config as singleton
    container->singleton<Config>("config", [this]() -> Config* {
        return config.get();
    });
    
    // Register router as singleton
    container->singleton<Router>("router", [this]() -> Router* {
        return router.get();
    });
}

void Application::registerMiddleware() {
    // Register core middleware
    // Note: These middleware classes need to be implemented
    // router->registerMiddleware("cors", std::make_shared<CorsMiddleware>());
    // router->registerMiddleware("auth", std::make_shared<AuthMiddleware>());
    // router->registerMiddleware("logging", std::make_shared<LoggingMiddleware>());
    // router->registerMiddleware("json", std::make_shared<JsonMiddleware>());
    // router->registerMiddleware("ratelimit", std::make_shared<RateLimitMiddleware>());
    
    // For now, just register a basic middleware
    Serial.println("Middleware registration placeholder - implement custom middleware classes as needed");
}

void Application::registerRoutes() {
    // Routes will be registered in the main app file
    // This is just a placeholder for framework routes
    
    // Health check route
    router->get("/health", [](Request& request) -> Response {
        JsonDocument doc;
        doc["status"] = "ok";
        doc["framework"] = "ESP32 MVC";
        doc["version"] = "1.0.0";
        doc["uptime"] = millis();
        
        return Response(request.getServerRequest()).json(doc);
    });
}
