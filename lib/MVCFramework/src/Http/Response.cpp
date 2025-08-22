#include "Response.h"
#include <SPIFFS.h>

Response::Response(AsyncWebServerRequest* req) 
    : request(req), statusCode(200), type("text/html"), 
      binaryData(nullptr), binaryLength(0), isBinaryResponse(false) {
}

Response& Response::status(int code) {
    statusCode = code;
    return *this;
}

Response& Response::content(const String& body) {
    this->body = body;
    isBinaryResponse = false;
    return *this;
}

Response& Response::html(const String& html) {
    body = html;
    type = "text/html";
    isBinaryResponse = false;
    return *this;
}

Response& Response::text(const String& text) {
    body = text;
    type = "text/plain";
    isBinaryResponse = false;
    return *this;
}

Response& Response::json(const JsonDocument& data) {
    String jsonString;
    serializeJson(data, jsonString);
    body = jsonString;
    type = "application/json";
    isBinaryResponse = false;
    return *this;
}

Response& Response::json(const String& jsonString) {
    body = jsonString;
    type = "application/json";
    isBinaryResponse = false;
    return *this;
}

Response& Response::binary(const uint8_t* data, size_t length, const String& contentType) {
    binaryData = data;
    binaryLength = length;
    type = contentType;
    isBinaryResponse = true;
    body = ""; // Clear text body for binary data
    return *this;
}

Response& Response::header(const String& name, const String& value) {
    headers[name] = value;
    return *this;
}

Response& Response::contentType(const String& type) {
    this->type = type;
    return *this;
}

Response& Response::redirect(const String& url, int code) {
    statusCode = code;
    header("Location", url);
    return *this;
}

Response& Response::back() {
    // Get referer header and redirect there, or to home
    String referer = "";
    if (request->hasHeader("Referer")) {
        referer = request->getHeader("Referer")->value();
    }
    if (referer.length() == 0) {
        referer = "/";
    }
    return redirect(referer);
}

Response& Response::view(const String& template_name, const JsonDocument& data) {
    // TODO: Implement view rendering
    // For now, return simple HTML
    body = "<html><body><h1>View: " + template_name + "</h1></body></html>";
    type = "text/html";
    return *this;
}

Response& Response::file(const String& path) {
    // Check if file exists in SPIFFS
    if (!SPIFFS.exists(path)) {
        statusCode = 404;
        body = "File not found";
        type = "text/plain";
        return *this;
    }
    
    // Open file
    File file = SPIFFS.open(path, "r");
    if (!file) {
        statusCode = 500;
        body = "Unable to open file";
        type = "text/plain";
        return *this;
    }
    
    // Determine content type based on file extension
    String contentType = "application/octet-stream"; // Default binary type
    String lowerPath = path;
    lowerPath.toLowerCase();
    
    if (lowerPath.endsWith(".html") || lowerPath.endsWith(".htm")) {
        contentType = "text/html";
    } else if (lowerPath.endsWith(".css")) {
        contentType = "text/css";
    } else if (lowerPath.endsWith(".js")) {
        contentType = "application/javascript";
    } else if (lowerPath.endsWith(".json")) {
        contentType = "application/json";
    } else if (lowerPath.endsWith(".txt")) {
        contentType = "text/plain";
    } else if (lowerPath.endsWith(".ico")) {
        contentType = "image/x-icon";
    } else if (lowerPath.endsWith(".png")) {
        contentType = "image/png";
    } else if (lowerPath.endsWith(".jpg") || lowerPath.endsWith(".jpeg")) {
        contentType = "image/jpeg";
    } else if (lowerPath.endsWith(".gif")) {
        contentType = "image/gif";
    } else if (lowerPath.endsWith(".svg")) {
        contentType = "image/svg+xml";
    } else if (lowerPath.endsWith(".pdf")) {
        contentType = "application/pdf";
    }
    
    type = contentType;
    
    // For binary files, we need to handle them differently
    // Store the file path for later use in send() method
    body = ""; // Clear body for binary files
    header("X-File-Path", path); // Store path in custom header for send() method
    
    file.close();
    return *this;
}

Response& Response::download(const String& path, const String& name) {
    String filename = name.length() > 0 ? name : path;
    header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    return file(path);
}

void Response::send() {
    if (!request) return;
    
    AsyncWebServerResponse* response;
    
    // Check if this is a binary response
    if (isBinaryResponse && binaryData && binaryLength > 0) {
        // Send binary data
        response = request->beginResponse_P(statusCode, type, binaryData, binaryLength);
    }
    // Check if this is a file response
    else if (headers.find("X-File-Path") != headers.end()) {
        String filePath = headers["X-File-Path"];
        
        // Remove the custom header before sending
        headers.erase("X-File-Path");
        
        // Serve file directly from SPIFFS
        response = request->beginResponse(SPIFFS, filePath, type);
        
        if (!response) {
            // Fallback if file serving fails
            response = request->beginResponse(404, "text/plain", "File not found");
        }
    } else {
        // Regular text/json response
        response = request->beginResponse(statusCode, type, body);
    }
    
    // Add custom headers
    for (const auto& pair : headers) {
        response->addHeader(pair.first, pair.second);
    }
    
    request->send(response);
}
