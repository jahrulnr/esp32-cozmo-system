#ifndef WEBSOCKET_REQUEST_H
#define WEBSOCKET_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>

class WebSocketRequest {
private:
    AsyncWebSocket* ws;
    AsyncWebSocketClient* client;
    String path;
    std::map<String, String> headers;
    std::map<String, String> parameters;

public:
    WebSocketRequest(AsyncWebSocket* webSocket, AsyncWebSocketClient* wsClient);
    
    // Client information
    uint32_t clientId() const;
    String clientIP() const;
    bool isConnected() const;
    
    // Path and parameters
    String getPath() const;
    void setPath(const String& wsPath);
    void setParameter(const String& key, const String& value);
    String getParameter(const String& key, const String& defaultValue = "") const;
    bool hasParameter(const String& key) const;
    
    // Headers (from initial HTTP upgrade request)
    void setHeader(const String& name, const String& value);
    String getHeader(const String& name, const String& defaultValue = "") const;
    bool hasHeader(const String& name) const;
    
    // Send messages to client
    void send(const String& message);
    void send(uint8_t* data, size_t length);
    void close(uint16_t code = 1000, const String& reason = "");
    
    // Server utilities
    void broadcast(const String& message);
    void broadcast(uint8_t* data, size_t length);
    
    // Get underlying objects for advanced usage
    AsyncWebSocket* getWebSocket() const { return ws; }
    AsyncWebSocketClient* getClient() const { return client; }
};

#endif
