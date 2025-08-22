#ifndef REQUEST_H
#define REQUEST_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESPAsyncWebServer.h"
#include <map>

class Request {
private:
    AsyncWebServerRequest* serverRequest;
    std::map<String, String> parameters;
    std::map<String, String> headers;
    String body;

public:
    Request(AsyncWebServerRequest* request);
    
    // Static method to set up body handling
    static void setupBodyHandling(AsyncWebServer* server);
    
    // HTTP Methods
    String method() const;
    bool isGet() const { return method() == "GET"; }
    bool isPost() const { return method() == "POST"; }
    bool isPut() const { return method() == "PUT"; }
    bool isDelete() const { return method() == "DELETE"; }
    bool isPatch() const { return method() == "PATCH"; }
    
    // URL and Path
    String url() const;
    String path() const;
    String query() const;
    
    // Parameters
    String input(const String& key, const String& defaultValue = "") const;
    String get(const String& key, const String& defaultValue = "") const;
    String post(const String& key, const String& defaultValue = "") const;
    bool has(const String& key) const;
    
    // Headers
    String header(const String& name, const String& defaultValue = "") const;
    bool hasHeader(const String& name) const;
    
    // Body
    String getBody() const { return body; }
    void setBody(const String& content) { body = content; }
    
    // Files (for future implementation)
    bool hasFile(const String& name) const;
    
    // Validation helpers
    bool filled(const String& key) const;
    bool missing(const String& key) const { return !has(key); }
    
    // JSON support
    JsonDocument json() const;
    bool wantsJson() const;
    
    // Client info
    String ip() const;
    String userAgent() const;
    
    // Route parameters (set by router)
    void setRouteParameter(const String& key, const String& value);
    String route(const String& key, const String& defaultValue = "") const;
    
    AsyncWebServerRequest* getServerRequest() const { return serverRequest; }
};

#endif
