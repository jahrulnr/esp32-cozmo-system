#include "WebSocketRequest.h"

WebSocketRequest::WebSocketRequest(AsyncWebSocket* webSocket, AsyncWebSocketClient* wsClient) 
    : ws(webSocket), client(wsClient) {
}

uint32_t WebSocketRequest::clientId() const {
    return client ? client->id() : 0;
}

String WebSocketRequest::clientIP() const {
    return client ? client->remoteIP().toString() : "";
}

bool WebSocketRequest::isConnected() const {
    return client && client->status() == WS_CONNECTED;
}

String WebSocketRequest::getPath() const {
    return path;
}

void WebSocketRequest::setPath(const String& wsPath) {
    path = wsPath;
}

void WebSocketRequest::setParameter(const String& key, const String& value) {
    parameters[key] = value;
}

String WebSocketRequest::getParameter(const String& key, const String& defaultValue) const {
    auto it = parameters.find(key);
    return it != parameters.end() ? it->second : defaultValue;
}

bool WebSocketRequest::hasParameter(const String& key) const {
    return parameters.find(key) != parameters.end();
}

void WebSocketRequest::setHeader(const String& name, const String& value) {
    headers[name] = value;
}

String WebSocketRequest::getHeader(const String& name, const String& defaultValue) const {
    auto it = headers.find(name);
    return it != headers.end() ? it->second : defaultValue;
}

bool WebSocketRequest::hasHeader(const String& name) const {
    return headers.find(name) != headers.end();
}

void WebSocketRequest::send(const String& message) {
    if (client && client->status() == WS_CONNECTED) {
        client->text(message);
    }
}

void WebSocketRequest::send(uint8_t* data, size_t length) {
    if (client && client->status() == WS_CONNECTED) {
        client->binary(data, length);
    }
}

void WebSocketRequest::close(uint16_t code, const String& reason) {
    if (client) {
        client->close(code, reason.c_str());
    }
}

void WebSocketRequest::broadcast(const String& message) {
    if (ws) {
        ws->textAll(message);
    }
}

void WebSocketRequest::broadcast(uint8_t* data, size_t length) {
    if (ws) {
        ws->binaryAll(data, length);
    }
}
