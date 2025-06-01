#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <functional>
#include <map>

namespace Communication {

class WebServer {
public:
    WebServer();
    ~WebServer();

    /**
     * Initialize web server
     * @param port The port to listen on
     * @return true if initialization was successful, false otherwise
     */
    bool init(uint16_t port = 80);

    /**
     * Start the web server
     */
    void begin();

    /**
     * Add a route handler for a specific HTTP method and path
     * @param method The HTTP method (GET, POST, etc.)
     * @param path The URL path
     * @param handler The handler function
     */
    void on(const char* path, WebRequestMethod method, ArRequestHandlerFunction handler);

    /**
     * Add a route handler for any HTTP method at a specific path
     * @param path The URL path
     * @param handler The handler function
     */
    void on(const char* path, ArRequestHandlerFunction handler);

    /**
     * Serve static files from SPIFFS
     * @param uri The URL path
     * @param contentType The content type
     * @param download Whether to download the file (optional)
     */
    void serveStatic(const char* uri, const char* contentType);

    /**
     * Add handler for when no route matches
     * @param handler The handler function
     */
    void onNotFound(ArRequestHandlerFunction handler);
    
    /**
     * Get the underlying AsyncWebServer instance
     * @return Pointer to AsyncWebServer instance
     */
    AsyncWebServer* getServer() { return _server; }

private:
    AsyncWebServer* _server;
    bool _initialized;
    
    // Helper function to determine content type from file extension
    String getContentType(const String& filename);
};

} // namespace Communication
