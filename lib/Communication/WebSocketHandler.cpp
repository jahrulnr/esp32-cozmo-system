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
            client->text(message);
        }
    }
}

void WebSocketHandler::sendBinary(int clientId, const uint8_t* data, size_t length) {
    if (!_initialized || !_webSocket) {
        return;
    }
    
    if (clientId < 0) {
        _webSocket->binaryAll(data, length);
    } else {
        AsyncWebSocketClient* client = _webSocket->client(clientId);
        if (client) {
            client->binary((const char*)data, length);
        }
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
        return client->remoteIP();
    }
    
    return IPAddress(0, 0, 0, 0);
}

} // namespace Communication
