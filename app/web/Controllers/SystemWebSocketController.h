#pragma once

#include <MVCFramework.h>
#include "Http/WebSocketController.h"
#include "Http/WebSocketRequest.h"
#include "Http/WebSocketResponse.h"
#include <ArduinoJson.h>
#include "AuthController.h"

class SystemWebSocketController : public WebSocketController {
public:
    // System status operations
    static WebSocketResponse getSystemStatus(WebSocketRequest& request);
    static WebSocketResponse getStorageInfo(WebSocketRequest& request);
    static WebSocketResponse getStorageStatus(WebSocketRequest& request);
    
private:
    // Helper methods
    static Utils::SpiJsonDocument createSystemStatusData();
    static Utils::SpiJsonDocument createStorageData();
    static Utils::SpiJsonDocument createStorageStatusData(const Utils::Sstring& storageType);
    static Utils::SpiJsonDocument createErrorResponse(const Utils::Sstring& message, const Utils::Sstring& code = "");
    static Utils::SpiJsonDocument createSuccessResponse(const Utils::SpiJsonDocument& data = Utils::SpiJsonDocument());
    static bool requiresAuthentication(const Utils::Sstring& operation);
    static WebSocketResponse unauthorizedResponse(WebSocketRequest& request);
    static Utils::Sstring formatBytes(size_t bytes);
    static bool isApOnlyMode();
    static bool isValidStorageType(const Utils::Sstring& storageType);
};
