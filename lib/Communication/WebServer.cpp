#include "WebServer.h"

namespace Communication {

WebServer::WebServer() : _server(nullptr), _initialized(false) {
}

WebServer::~WebServer() {
    if (_server) {
        delete _server;
    }
}

bool WebServer::init(uint16_t port) {
    _server = new AsyncWebServer(port);
    _initialized = true;
    return true;
}

void WebServer::begin() {
    if (!_initialized || !_server) {
        return;
    }
    
    _server->begin();
}

void WebServer::on(const char* path, WebRequestMethod method, ArRequestHandlerFunction handler) {
    if (!_initialized || !_server) {
        return;
    }
    
    _server->on(path, method, handler);
}

void WebServer::on(const char* path, ArRequestHandlerFunction handler) {
    if (!_initialized || !_server) {
        return;
    }
    
    _server->on(path, handler);
}

void WebServer::serveStatic(const char* uri, const char* contentType) {
    if (!_initialized || !_server) {
        return;
    }

		String path = uri;
		if (path.endsWith("/")) {
			path += "index.html";
		}
    
    _server->serveStatic(uri, SPIFFS, path.c_str());
}

void WebServer::onNotFound(ArRequestHandlerFunction handler) {
    if (!_initialized || !_server) {
        return;
    }
    
    _server->onNotFound(handler);
}

String WebServer::getContentType(const String& filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".json")) return "application/json";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".jpg")) return "image/jpeg";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".svg")) return "image/svg+xml";
    else if (filename.endsWith(".pdf")) return "application/pdf";
    else if (filename.endsWith(".zip")) return "application/zip";
    else if (filename.endsWith(".gz")) return "application/gzip";
    return "text/plain";
}

} // namespace Communication
