#include "WebSocketHandler.h"

namespace Communication {

WebSocketHandler::WebSocketHandler() : _server(nullptr), _webSocket(nullptr), _initialized(false), _ownsServer(false) {
}

WebSocketHandler::~WebSocketHandler() {
    if (_webSocket) {
        delete _webSocket;
    }
    
    if (_ownsServer && _server) {
        delete _server;
    }
}

bool WebSocketHandler::init(const String& path, AsyncWebServer* server) {
    // If server is not provided, create our own
    if (server == nullptr) {
        _server = new AsyncWebServer(80);
        _ownsServer = true;
    } else {
        _server = server;
        _ownsServer = false;
    }
    
    // Create WebSocket instance
    _webSocket = new AsyncWebSocket(path);
    
    // Add WebSocket handler to server
    _server->addHandler(_webSocket);
    
    // Set default event handler
    _webSocket->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                              AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (this->_eventCallback) {
            this->_eventCallback(server, client, type, arg, data, len);
        }
    });
    
    _initialized = true;
    return true;
}

void WebSocketHandler::begin() {
    // AsyncWebSocket doesn't need explicit begin
    // But we'll start the server if we created it ourselves
    if (_ownsServer && _server) {
        _server->begin();
    }
}

void WebSocketHandler::loop() {
    // AsyncWebSocket doesn't need manual loop handling
    // This is kept for API compatibility
}

void WebSocketHandler::sendText(int clientId, const String& message) {
    if (!_initialized || !_webSocket) {
        return;
    }
    
    if (clientId < 0) {
        _webSocket->textAll(message);
    } else {
        AsyncWebSocketClient* client = _webSocket->client(clientId);
        if (client) {
            client->setCloseClientOnQueueFull(false);
            client->text(message);
        }
    }
}

void WebSocketHandler::sendJsonMessage(int clientId, const String& type, const JsonVariant& data) {
    Utils::SpiJsonDocument doc;
    doc["version"] = "1.0";
    doc["type"] = type;
    doc["data"] = data;
    
    String jsonString;
    serializeJson(doc, jsonString);
    sendText(clientId, jsonString);
}

void WebSocketHandler::sendJsonMessage(int clientId, const String& type, const String& jsonString) {
    String message = "{\"version\":\"1.0\",\"type\":\"" + type + "\",\"data\":" + jsonString + "}";
    sendText(clientId, message);
}

void WebSocketHandler::sendError(int clientId, int code, const String& message) {
    Utils::SpiJsonDocument doc;
    doc["version"] = "1.0";
    doc["type"] = "error";
    doc["data"]["code"] = code;
    doc["data"]["message"] = message;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    sendText(clientId, jsonString);
}

void WebSocketHandler::sendOk(int clientId, const String& message) {
    Utils::SpiJsonDocument doc;
    doc["version"] = "1.0";
    doc["type"] = "ok";
    doc["data"]["message"] = message;
    
    String jsonString;
    serializeJson(doc, jsonString);
    sendText(clientId, jsonString);
}

void WebSocketHandler::sendBinary(int clientId, const uint8_t* data, size_t length) {
    if (!_initialized || !_webSocket || !data || length == 0) {
        return;
    }
    
    if (clientId < 0) {
        _webSocket->binaryAll(data, length);
    } else {
        // Send to specific client
        AsyncWebSocketClient* client = _webSocket->client(clientId);
        client->setCloseClientOnQueueFull(false);
        if (client && client->status() == WS_CONNECTED) {
            client->binary((const char*)data, length);
        }
    }
    
    // For large binary frames, give the ESP some time to breathe
    if (length > 10000) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void WebSocketHandler::onEvent(std::function<void(AsyncWebSocket* server, AsyncWebSocketClient* client, 
    AwsEventType type, void* arg, uint8_t* data, size_t len)> callback) {
    _eventCallback = callback;
}

IPAddress WebSocketHandler::remoteIP(uint32_t clientId) {
    if (!_initialized || !_webSocket) {
        return IPAddress(0, 0, 0, 0);
    }
    
    AsyncWebSocketClient* client = _webSocket->client(clientId);
    if (client) {
        client->setCloseClientOnQueueFull(false);
        return client->remoteIP();
    }
    
    return IPAddress(0, 0, 0, 0);
}

Utils::SpiJsonDocument WebSocketHandler::parseJsonMessage(uint8_t* data, size_t len) {
    Utils::SpiJsonDocument doc;
    
    // Ensure null termination for the JSON parser
    char* jsonStr = new char[len + 1];
    memcpy(jsonStr, data, len);
    jsonStr[len] = 0;
    
    DeserializationError error = deserializeJson(doc, jsonStr);
    delete[] jsonStr;
    
    if (error) {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return Utils::SpiJsonDocument();
    }
    
    // Check if this is a v1.0 format message with version field
    if (!doc["version"].isUnbound()) {
        // It's already in the new format, return it as is
        return doc;
    } else if (!doc["type"].isUnbound() && !doc["data"].isUnbound()) {
        // It's in the old format, convert to new format
        Utils::SpiJsonDocument newFormatDoc;
        newFormatDoc["version"] = "1.0";
        newFormatDoc["type"] = doc["type"];
        newFormatDoc["data"] = doc["data"];
        return newFormatDoc;
    }
    
    return doc;
}

bool WebSocketHandler::hasClients() {
    if (!_initialized || !_webSocket) {
        return false;
    }
    
    return _webSocket->count() > 0;
}

bool WebSocketHandler::clientWantsCameraFrames(int clientId) {
    if (clientId < 0) return false;
    return _clientWantsCameraFrames.count(clientId) > 0 && _clientWantsCameraFrames[clientId];
}

void WebSocketHandler::setCameraSubscription(int clientId, bool wantsCameraFrames) {
    if (clientId < 0) return;
    _clientWantsCameraFrames[clientId] = wantsCameraFrames;
}

bool WebSocketHandler::hasClientsForCameraFrames() {
    if (_clientWantsCameraFrames.empty()) {
        return true;  // Default behavior: if no specific subscriptions, assume all clients want frames
    }
    
    for (const auto& pair : _clientWantsCameraFrames) {
        if (pair.second) {
            return true;
        }
    }
    
    return false;
}

size_t WebSocketHandler::count() const {
    return _webSocket->count();
}

} // namespace Communication
