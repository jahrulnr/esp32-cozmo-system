#ifndef ROUTER_H
#define ROUTER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include <vector>
#include <functional>

// Forward declarations
class Request;
class Response;
class Middleware;
class WebSocketRequest;
class WebSocketResponse;

struct Route {
    String method;
    String path;
    std::function<Response(Request&)> handler;
    std::vector<String> middleware;
    String name;
    std::map<String, String> parameters;
};

struct WebSocketRoute {
    String path;
    std::function<void(WebSocketRequest&)> onConnect;
    std::function<void(WebSocketRequest&)> onDisconnect;
    std::function<void(WebSocketRequest&, const String&)> onMessage;
    std::function<void(WebSocketRequest&, uint8_t*, size_t)> onBinary;
    std::vector<String> middleware;
    String name;
};

class Router {
private:
    AsyncWebServer* server;
    std::vector<Route> routes;
    std::vector<WebSocketRoute> wsRoutes;
    std::map<String, AsyncWebSocket*> webSockets;
    std::map<String, std::shared_ptr<Middleware>> middlewares;
    String prefix;
    std::vector<String> middlewareStack;

public:
    Router(AsyncWebServer* webServer);
    
    // Route registration
    Router& get(const String& path, std::function<Response(Request&)> handler);
    Router& post(const String& path, std::function<Response(Request&)> handler);
    Router& put(const String& path, std::function<Response(Request&)> handler);
    Router& patch(const String& path, std::function<Response(Request&)> handler);
    Router& delete_(const String& path, std::function<Response(Request&)> handler);
    Router& any(const String& path, std::function<Response(Request&)> handler);
    
    // WebSocket registration
    Router& websocket(const String& path);
    Router& onConnect(std::function<void(WebSocketRequest&)> handler);
    Router& onDisconnect(std::function<void(WebSocketRequest&)> handler);
    Router& onMessage(std::function<void(WebSocketRequest&, const String&)> handler);
    Router& onBinary(std::function<void(WebSocketRequest&, uint8_t*, size_t)> handler);
    
    // Route groups
    Router& group(const String& prefix, std::function<void(Router&)> routes);
    Router& middleware(const String& name);
    Router& middleware(const std::vector<String>& names);
    
    // Named routes
    Router& name(const String& routeName);
    String route(const String& name, const std::map<String, String>& parameters = {});
    
    // Controller routes
    Router& controller(const String& path, const String& controller);
    Router& resource(const String& path, const String& controller);
    
    // Middleware management
    void registerMiddleware(const String& name, std::shared_ptr<Middleware> middleware);
    
    // Route matching and execution
    bool match(const String& method, const String& path, AsyncWebServerRequest* request);
    void handleRequest(AsyncWebServerRequest* request);
    void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
    
    // WebSocket utilities
    void broadcastText(const String& path, const String& message);
    void broadcastBinary(const String& path, uint8_t* data, size_t len);
    void sendToClient(const String& path, uint32_t clientId, const String& message);
    AsyncWebSocket* getWebSocket(const String& path);
    
    // Initialize routes on server
    void init();
    
private:
    Route& addRoute(const String& method, const String& path, std::function<Response(Request&)> handler);
    WebSocketRoute& addWebSocketRoute(const String& path);
    bool matchRoute(const Route& route, const String& method, const String& path, std::map<String, String>& params);
    String compilePath(const String& path);
    Response executeMiddleware(const std::vector<String>& middleware, Request& request, std::function<Response(Request&)> next);
    WebSocketRoute* currentWsRoute = nullptr; // For chaining WebSocket handlers
};

#endif
