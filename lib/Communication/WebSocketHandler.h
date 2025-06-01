#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>

namespace Communication {

class WebSocketHandler {
public:
    WebSocketHandler();
    ~WebSocketHandler();

    /**
     * Initialize WebSocket server
     * @param path The URL path for the WebSocket
     * @param port The port to listen on (used if server is not provided)
     * @return true if initialization was successful, false otherwise
     */
    bool init(const String& path = "/ws", AsyncWebServer* server = nullptr);

    /**
     * Start the WebSocket server (not needed with AsyncWebSocket)
     * Kept for API compatibility
     */
    void begin();

    /**
     * Handle incoming WebSocket messages (not needed with AsyncWebSocket)
     * Kept for API compatibility
     */
    void loop();

    /**
     * Send a text message to a specific client
     * @param clientId The client ID to send to (or -1 for broadcast)
     * @param message The message to send
     */
    void sendText(int clientId, const String& message);

    /**
     * Send binary data to a specific client
     * @param clientId The client ID to send to (or -1 for broadcast)
     * @param data Pointer to the data to send
     * @param length Length of the data in bytes
     */
    void sendBinary(int clientId, const uint8_t* data, size_t length);

    /**
     * Set the callback function for handling events
     * @param callback The callback function
     */
    void onEvent(std::function<void(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                                  AwsEventType type, void* arg, uint8_t* data, size_t len)> callback);
    
    /**
     * Get remote IP address for client
     * @param clientId The client ID
     * @return IP address of the client
     */
    IPAddress remoteIP(uint32_t clientId);

private:
    AsyncWebServer* _server;
    AsyncWebSocket* _webSocket;
    bool _initialized;
    bool _ownsServer;

    // Event handler function
    std::function<void(AsyncWebSocket* server, AsyncWebSocketClient* client,
                      AwsEventType type, void* arg, uint8_t* data, size_t len)> _eventCallback;
};

} // namespace Communication
