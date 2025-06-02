#include "WebSocketHandler.h"

namespace Communication {

WebSocketHandler::WebSocketHandler() : _server(nullptr), _webSocket(nullptr), _initialized(false), _ownsServer(false), _mux(xSemaphoreCreateMutex()) {
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
    
    if (xSemaphoreTake(_mux, pdMS_TO_TICKS(1000)) == pdFALSE) return;
    if (clientId < 0) {
        _webSocket->textAll(message);
    } else {
        AsyncWebSocketClient* client = _webSocket->client(clientId);
        if (client) {
            client->text(message);
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    xSemaphoreGive(_mux);
}

void WebSocketHandler::sendJsonMessage(int clientId, const String& type, const JsonVariant& data) {
    Utils::SpiJsonDocument doc;
    doc["type"] = type;
    doc["data"] = data;
    
    String jsonString;
    serializeJson(doc, jsonString);
    sendText(clientId, jsonString);
}

void WebSocketHandler::sendJsonMessage(int clientId, const String& type, const String& jsonString) {
    String message = "{\"type\":\"" + type + "\",\"data\":" + jsonString + "}";
    sendText(clientId, message);
}

void WebSocketHandler::sendError(int clientId, int code, const String& message) {
    Utils::SpiJsonDocument doc;
    JsonObject data = doc["data"];
    data["code"] = code;
    data["message"] = message;
    
    String jsonString;
    doc["type"] = "error";
    serializeJson(doc, jsonString);
    
    sendText(clientId, jsonString);
}

void WebSocketHandler::sendOk(int clientId, const String& message) {
    Utils::SpiJsonDocument doc;
    JsonObject data = doc["data"];
    data["message"] = message;
    
    String jsonString;
    doc["type"] = "ok";
    serializeJson(doc, jsonString);
    
    sendText(clientId, jsonString);
}

void WebSocketHandler::sendBinary(int clientId, const uint8_t* data, size_t length) {
    if (!_initialized || !_webSocket) {
        return;
    }
    
    if (xSemaphoreTake(_mux, pdMS_TO_TICKS(1000)) == pdFALSE) return;
    if (clientId < 0) {
        _webSocket->binaryAll(data, length);
    } else {
        AsyncWebSocketClient* client = _webSocket->client(clientId);
        if (client) {
            client->binary((const char*)data, length);
        }
    }
    xSemaphoreGive(_mux);
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
    
    return doc;
}

} // namespace Communication
