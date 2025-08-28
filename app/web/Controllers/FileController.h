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
    
    // Storage information
    static Response getStorageInfo(Request& request);
    
private:
    // Helper methods
    static bool isValidPath(const Utils::Sstring& path);
    static bool isAllowedFileType(const Utils::Sstring& filename);
    static Utils::Sstring sanitizePath(const Utils::Sstring& path);
    static Utils::SpiJsonDocument formatFileInfo(const Utils::Sstring& path);
    static Utils::SpiJsonDocument createErrorResponse(const Utils::Sstring& message, const Utils::Sstring& code = "");
    static Utils::SpiJsonDocument createSuccessResponse(const Utils::SpiJsonDocument& data = Utils::SpiJsonDocument());
    static bool requiresAuthentication(const Utils::Sstring& operation);
    static Response unauthorizedResponse(Request& request);
    static Utils::Sstring formatBytes(size_t bytes);
};