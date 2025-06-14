#pragma once

#include <Arduino.h>
#include <map>
#include <WiFi.h>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "lib/Utils/SpiAllocator.h"

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
     * Check if there are any connected clients
     * @return true if there are any connected clients, false otherwise
     */
    bool hasClients();

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
     * Send a standardized JSON format message to a specific client
     * Format: {version: "1.0", type: "command_type", data: {...}}
     * @param clientId The client ID to send to (or -1 for broadcast)
     * @param type The message type (command_type)
     * @param data The JsonVariant object containing the data payload
     */
    void sendJsonMessage(int clientId, const String& type, const JsonVariant& data);

    /**
     * Send a standardized JSON format message to a specific client
     * Format: {version: "1.0", type: "command_type", data: {...}}
     * @param clientId The client ID to send to (or -1 for broadcast)
     * @param type The message type (command_type)
     * @param jsonString A pre-formatted JSON string for the data field
     */
    void sendJsonMessage(int clientId, const String& type, const String& jsonString);
    
    /**
     * Send an error message in the standard format
     * @param clientId The client ID to send to
     * @param code The error code
     * @param message The error message
     */
    void sendError(int clientId, int code, const String& message);
    
    /**
     * Send a simple OK response
     * @param clientId The client ID to send to
     * @param message Optional success message
     */
    void sendOk(int clientId, const String& message = "Success");

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
    
    /**
     * Helper to parse incoming JSON messages
     * Supports both old format and new DTO contract format (v1.0)
     * For old format: {type, data}
     * For new format: {version, type, data}
     * 
     * @param data Pointer to the message data
     * @param len Length of the message
     * @return JsonDocument with the parsed message (always in new format), or null if parsing failed
     */
    static Utils::SpiJsonDocument parseJsonMessage(uint8_t* data, size_t len);

    /**
     * Check if a client is subscribed to camera frames
     * @param clientId The client ID to check
     * @return true if the client wants camera frames, false otherwise
     */
    bool clientWantsCameraFrames(int clientId);
    
    /**
     * Set whether a client wants to receive camera frames
     * @param clientId The client ID
     * @param wantsCameraFrames Whether the client wants camera frames
     */
    void setCameraSubscription(int clientId, bool wantsCameraFrames);
    
    /**
     * Check if there are any clients subscribed to camera frames
     * @return true if there are any clients that want camera frames, false otherwise
     */
    bool hasClientsForCameraFrames();

private:
    AsyncWebServer* _server;
    AsyncWebSocket* _webSocket;
    bool _initialized;
    bool _ownsServer;
    
    // Client subscriptions for camera frames
    std::map<int, bool> _clientWantsCameraFrames;

    // Event handler function
    std::function<void(AsyncWebSocket* server, AsyncWebSocketClient* client,
                      AwsEventType type, void* arg, uint8_t* data, size_t len)> _eventCallback;
};

} // namespace Communication
