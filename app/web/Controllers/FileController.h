#pragma once

#include <MVCFramework.h>
#include "Http/Controller.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "FileManager.h"
#include "AuthController.h"

class FileController : public Controller {
public:
    // File operations
    static Response download(Request& request);
    static Response upload(Request& request);
    static Response listFiles(Request& request);
    static Response deleteFile(Request& request);
    static Response getFileInfo(Request& request);
    
    // Directory operations
    static Response createDirectory(Request& request);
    static Response listDirectory(Request& request);
    
    // Storage information
    static Response getStorageInfo(Request& request);
    
private:
    // Helper methods
    static bool isValidPath(const String& path);
    static bool isAllowedFileType(const String& filename);
    static String sanitizePath(const String& path);
    static JsonDocument formatFileInfo(const String& path);
    static JsonDocument createErrorResponse(const String& message, const String& code = "");
    static JsonDocument createSuccessResponse(const JsonDocument& data = JsonDocument());
    static bool requiresAuthentication(const String& operation);
    static Response unauthorizedResponse(Request& request);
		static String formatBytes(size_t bytes);
};