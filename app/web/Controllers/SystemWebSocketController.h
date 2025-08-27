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
    static JsonDocument createSystemStatusData();
    static JsonDocument createStorageData();
    static JsonDocument createStorageStatusData(const String& storageType);
    static JsonDocument createErrorResponse(const String& message, const String& code = "");
    static JsonDocument createSuccessResponse(const JsonDocument& data = JsonDocument());
    static bool requiresAuthentication(const String& operation);
    static WebSocketResponse unauthorizedResponse(WebSocketRequest& request);
    static String formatBytes(size_t bytes);
    static bool isApOnlyMode();
    static bool isValidStorageType(const String& storageType);
};
