#include "SystemWebSocketController.h"
#include "setup/setup.h"

WebSocketResponse SystemWebSocketController::getSystemStatus(WebSocketRequest& request) {
    Utils::SpiJsonDocument statusData = createSystemStatusData();
    Utils::SpiJsonDocument response = createSuccessResponse(statusData);
    
    return WebSocketResponse(request)
        .type("system_status")
        .data(response);
}

WebSocketResponse SystemWebSocketController::getStorageInfo(WebSocketRequest& request) {
    Utils::SpiJsonDocument storageData = createStorageData();
    Utils::SpiJsonDocument response = createSuccessResponse(storageData);
    
    return WebSocketResponse(request)
        .type("storage_info")
        .data(response);
}

WebSocketResponse SystemWebSocketController::getStorageStatus(WebSocketRequest& request) {
    Utils::Sstring storageType = request.getParameter("storage_type", "STORAGE_SPIFFS");
    
    if (!isValidStorageType(storageType)) {
        Utils::SpiJsonDocument error = createErrorResponse("Invalid storage type", "INVALID_STORAGE_TYPE");
        return WebSocketResponse(request)
            .type("error")
            .data(error);
    }
    
    Utils::SpiJsonDocument statusData = createStorageStatusData(storageType);
    Utils::SpiJsonDocument response = createSuccessResponse(statusData);
    
    return WebSocketResponse(request)
        .type("storage_status")
        .data(response);
}

// Helper methods
Utils::SpiJsonDocument SystemWebSocketController::createSystemStatusData() {
    Utils::SpiJsonDocument statusData;
    
    // WiFi status with more detailed information
    if (wifiManager) {
        bool connected = wifiManager->isConnected();
        statusData["wifi"] = connected;
        statusData["wifi_mode"] = isApOnlyMode() ? "ap" : "station";

        if (connected) {
            statusData["ip"] = wifiManager->getIP();
            statusData["rssi"] = wifiManager->getRSSI();
        }

        // Include the AP info if in AP mode
        if (isApOnlyMode()) {
            Communication::WiFiManager::WiFiConfig config = wifiManager->getConfig();
            statusData["ap_ssid"] = config.apSsid;
        }
    }

    // Other system statuses
    statusData["battery"] = -1; // Sample data
    statusData["memory"] = Utils::Sstring(ESP.getFreeHeap() / 1024) + " KB";  // Actual free memory in KB
    statusData["cpu"] = Utils::Sstring(ESP.getCpuFreqMHz()) + "Mhz";
    statusData["spiffs_total"] = Utils::Sstring(SPIFFS.totalBytes() / 1024) + " KB"; // KB
    statusData["spiffs_used"] = Utils::Sstring((SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024) + " KB"; // KB
    statusData["temperature"] = temperatureSensor ? temperatureSensor->readTemperature() : 0.0;

#if MICROPHONE_I2S
    statusData["microphone"]["enabled"] = amicrophone != nullptr;
    if (amicrophone && amicrophone->isInitialized()) {
        statusData["microphone"]["level"] = amicrophone->readLevel();
    }
#elif MICROPHONE_ANALOG
    statusData["microphone"]["enabled"] = amicrophone != nullptr;
    if (amicrophone && amicrophone->isInitialized()) {
        statusData["microphone"]["level"] = amicrophone->readLevel();
    }
#endif
    statusData["uptime"] = millis() / 1000;
    
    return statusData;
}

Utils::SpiJsonDocument SystemWebSocketController::createStorageData() {
    Utils::SpiJsonDocument storageData;

    // Get SPIFFS information
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;
    float percentUsed = (float)usedBytes / (float)totalBytes * 100.0;

    storageData["total"] = totalBytes;
    storageData["used"] = usedBytes;
    storageData["free"] = freeBytes;
    storageData["percent"] = percentUsed;
    
    // Human readable formats
    storageData["total_formatted"] = formatBytes(totalBytes);
    storageData["used_formatted"] = formatBytes(usedBytes);
    storageData["free_formatted"] = formatBytes(freeBytes);

    return storageData;
}

Utils::SpiJsonDocument SystemWebSocketController::createStorageStatusData(const Utils::Sstring& storageType) {
    Utils::SpiJsonDocument statusData;
    statusData["storage_type"] = storageType;
    
    if (storageType == "STORAGE_SPIFFS") {
        statusData["available"] = true;
        statusData["status"] = "Connected";
        statusData["type"] = "Internal Flash";
        
        // Add SPIFFS specific info
        statusData["total_bytes"] = SPIFFS.totalBytes();
        statusData["used_bytes"] = SPIFFS.usedBytes();
        statusData["free_bytes"] = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    } 
    else if (storageType == "STORAGE_SD_MMC") {
        // Initialize FileManager to check SD/MMC availability
        static Utils::FileManager fileManager;
        bool sdAvailable = fileManager.isSDMMCAvailable();
        
        statusData["available"] = sdAvailable;
        statusData["status"] = sdAvailable ? "Connected" : "Not Available";
        statusData["type"] = "SD/MMC Card";
        
        if (sdAvailable) {
            // Could add SD card specific information here
            statusData["total_bytes"] = 0; // Would need SD card size detection
            statusData["used_bytes"] = 0;
            statusData["free_bytes"] = 0;
        }
    }
    else {
        statusData["available"] = false;
        statusData["status"] = "Unknown";
        statusData["type"] = "Unknown";
    }
    
    return statusData;
}

Utils::SpiJsonDocument SystemWebSocketController::createErrorResponse(const Utils::Sstring& message, const Utils::Sstring& code) {
    Utils::SpiJsonDocument error;
    error["success"] = false;
    error["message"] = message;
    error["timestamp"] = millis();
    
    if (!code.isEmpty()) {
        error["error_code"] = code;
    }
    
    return error;
}

Utils::SpiJsonDocument SystemWebSocketController::createSuccessResponse(const Utils::SpiJsonDocument& data) {
    Utils::SpiJsonDocument response;
    response["success"] = true;
    response["timestamp"] = millis();
    
    if (!data.isNull()) {
        response["data"] = data;
    }
    
    return response;
}

bool SystemWebSocketController::requiresAuthentication(const Utils::Sstring& operation) {
    // System status operations require authentication in production
    // You can make this more granular based on security requirements
    return operation != "system_status"; // Allow system_status without auth for basic monitoring
}

WebSocketResponse SystemWebSocketController::unauthorizedResponse(WebSocketRequest& request) {
    Utils::SpiJsonDocument error = createErrorResponse("Authentication required", "UNAUTHORIZED");
    return WebSocketResponse(request)
        .type("error")
        .data(error);
}

Utils::Sstring SystemWebSocketController::formatBytes(size_t bytes) {
    if (bytes < 1024) {
        return Utils::Sstring(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return Utils::Sstring(bytes / 1024.0, 1) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return Utils::Sstring(bytes / (1024.0 * 1024.0), 1) + " MB";
    } else {
        return Utils::Sstring(bytes / (1024.0 * 1024.0 * 1024.0), 1) + " GB";
    }
}

bool SystemWebSocketController::isApOnlyMode() {
    // This should match the logic from your setup code
    if (wifiManager) {
        return !wifiManager->isConnected();
    }
    return true; // Default to AP mode if no WiFi manager
}

bool SystemWebSocketController::isValidStorageType(const Utils::Sstring& storageType) {
    return storageType == "STORAGE_SPIFFS" || 
           storageType == "STORAGE_SD_MMC";
}
