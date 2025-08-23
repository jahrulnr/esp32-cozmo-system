#include "SystemController.h"
#include <esp_system.h>
#include <esp_heap_caps.h>

Response SystemController::getStats(Request& request) {
    JsonDocument response;
    response["success"] = true;
    response["data"] = getSystemInfo();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response SystemController::restart(Request& request) {
    JsonDocument response;
    response["success"] = true;
    response["message"] = "System restart initiated";
    
    // Send response first, then restart
    Response res = Response(request.getServerRequest())
        .status(200)
        .json(response);
    
    // Schedule restart after a short delay
    delay(100);
    ESP.restart();
    
    return res;
}

Response SystemController::getNetworkInfo(Request& request) {
    JsonDocument response;
    JsonDocument networkInfo;
    
    if (WiFi.status() == WL_CONNECTED) {
        networkInfo["connected"] = true;
        networkInfo["ip"] = WiFi.localIP().toString();
        networkInfo["gateway"] = WiFi.gatewayIP().toString();
        networkInfo["subnet"] = WiFi.subnetMask().toString();
        networkInfo["dns"] = WiFi.dnsIP().toString();
        networkInfo["ssid"] = WiFi.SSID();
        networkInfo["rssi"] = WiFi.RSSI();
        networkInfo["mac"] = WiFi.macAddress();
    } else {
        networkInfo["connected"] = false;
        networkInfo["status"] = "Disconnected";
    }
    
    response["success"] = true;
    response["network"] = networkInfo;
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response SystemController::getMemoryInfo(Request& request) {
    JsonDocument response;
    JsonDocument memoryInfo;
    
    // Heap memory information
    memoryInfo["free_heap"] = ESP.getFreeHeap();
    memoryInfo["total_heap"] = ESP.getHeapSize();
    memoryInfo["used_heap"] = ESP.getHeapSize() - ESP.getFreeHeap();
    memoryInfo["largest_free_block"] = ESP.getMaxAllocHeap();
    memoryInfo["heap_fragmentation"] = 100 - (ESP.getMaxAllocHeap() * 100) / ESP.getFreeHeap();
    
    // PSRAM information (if available)
    if (psramFound()) {
        memoryInfo["psram_found"] = true;
        memoryInfo["free_psram"] = ESP.getFreePsram();
        memoryInfo["total_psram"] = ESP.getPsramSize();
        memoryInfo["used_psram"] = ESP.getPsramSize() - ESP.getFreePsram();
    } else {
        memoryInfo["psram_found"] = false;
    }
    
    // Flash memory information
    memoryInfo["flash_size"] = ESP.getFlashChipSize();
    memoryInfo["flash_speed"] = ESP.getFlashChipSpeed();
    
    response["success"] = true;
    response["memory"] = memoryInfo;
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

JsonDocument SystemController::getSystemInfo() {
    JsonDocument systemInfo;
    
    // Basic system information
    systemInfo["uptime"] = millis();
    systemInfo["uptime_formatted"] = formatUptime(millis());
    
    // Memory information
    JsonDocument memory;
    memory["free_heap"] = ESP.getFreeHeap();
    memory["free_heap_formatted"] = formatBytes(ESP.getFreeHeap());
    memory["total_heap"] = ESP.getHeapSize();
    memory["used_heap"] = ESP.getHeapSize() - ESP.getFreeHeap();
    memory["largest_free_block"] = ESP.getMaxAllocHeap();
    
    if (psramFound()) {
        memory["psram_found"] = true;
        memory["free_psram"] = ESP.getFreePsram();
        memory["total_psram"] = ESP.getPsramSize();
    } else {
        memory["psram_found"] = false;
    }
    
    systemInfo["memory"] = memory;
    
    // Network information
    JsonDocument network;
    if (WiFi.status() == WL_CONNECTED) {
        network["connected"] = true;
        network["ip"] = WiFi.localIP().toString();
        network["ssid"] = WiFi.SSID();
        network["rssi"] = WiFi.RSSI();
        network["mac"] = WiFi.macAddress();
    } else {
        network["connected"] = false;
    }
    systemInfo["network"] = network;
    
    // Hardware information
    JsonDocument hardware;
    hardware["chip_model"] = ESP.getChipModel();
    hardware["chip_revision"] = ESP.getChipRevision();
    hardware["chip_cores"] = ESP.getChipCores();
    hardware["cpu_freq"] = ESP.getCpuFreqMHz();
    hardware["flash_size"] = ESP.getFlashChipSize();
    hardware["flash_speed"] = ESP.getFlashChipSpeed();
    
    systemInfo["hardware"] = hardware;
    
    // Software information
    JsonDocument software;
    software["sdk_version"] = ESP.getSdkVersion();
    software["arduino_version"] = ARDUINO;
    software["compile_date"] = __DATE__;
    software["compile_time"] = __TIME__;
    
    systemInfo["software"] = software;
    
    return systemInfo;
}

String SystemController::formatUptime(unsigned long milliseconds) {
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    
    String uptime = "";
    if (days > 0) {
        uptime += String(days) + "d ";
    }
    uptime += String(hours).c_str();
    uptime += ":";
    if (minutes < 10) uptime += "0";
    uptime += String(minutes).c_str();
    uptime += ":";
    if (seconds < 10) uptime += "0";
    uptime += String(seconds).c_str();
    
    return uptime;
}

String SystemController::formatBytes(size_t bytes) {
    if (bytes < 1024) {
        return String(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return String(bytes / 1024.0, 1) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return String(bytes / (1024.0 * 1024.0), 1) + " MB";
    } else {
        return String(bytes / (1024.0 * 1024.0 * 1024.0), 1) + " GB";
    }
}

// Add these methods at the end of the file before the closing bracket

Response SystemController::getHostname(Request& request) {
    JsonDocument response;
    
    // Get hostname from configuration
    String hostname = Configuration::get("hostname", WiFi.getHostname());
    
    response["success"] = true;
    response["hostname"] = hostname;
    response["current"] = WiFi.getHostname(); // Current active hostname
    response["mdns"] = String(WiFi.getHostname()) + ".local";
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

// Add these new methods at the end of the file

Response SystemController::getConfigurations(Request& request) {
    JsonDocument response;
    
    // Get all configurations
    CsvDatabase* db = Model::getDatabase();
    if (!db || !db->tableExists("configurations")) {
        response["success"] = false;
        response["message"] = "Configuration database not initialized";
        return Response(request.getServerRequest())
            .status(500)
            .json(response);
    }
    
    std::vector<std::map<String, String>> results = db->select("configurations");
    JsonArray configs = response["configurations"].to<JsonArray>();
    
    for (const auto& row : results) {
        JsonObject config = configs.add<JsonObject>();
        
        // Find key and value in the row
        auto keyIt = row.find("key");
        auto valueIt = row.find("value");
        
        if (keyIt != row.end() && valueIt != row.end()) {
            config["key"] = keyIt->second;
            config["value"] = valueIt->second;
        }
    }
    
    response["success"] = true;
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response SystemController::updateConfiguration(Request& request) {
    String key = request.input("key");
    String value = request.input("value");
    JsonDocument response;
    
    if (key.length() == 0) {
        response["success"] = false;
        response["message"] = "Configuration key is required";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    // Handle special configuration keys that need additional processing
    bool requiresRestart = false;
    
    if (key == "hostname") {
        return updateHostname(request); // Use the dedicated method for hostname
    }
    
    // Store the configuration
    if (Configuration::set(key, value)) {
        response["success"] = true;
        response["message"] = "Configuration updated successfully";
        response["key"] = key;
        response["value"] = value;
        response["restart_required"] = requiresRestart;
    } else {
        response["success"] = false;
        response["message"] = "Failed to update configuration";
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 500)
        .json(response);
}

Response SystemController::updateHostname(Request& request) {
    String newHostname = request.input("hostname");
    JsonDocument response;
    
    if (newHostname.length() == 0) {
        response["success"] = false;
        response["message"] = "Hostname is required";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    if (newHostname.length() > 32) {
        response["success"] = false;
        response["message"] = "Hostname must be 32 characters or less";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    // Validate hostname (alphanumeric, dash)
    for (size_t i = 0; i < newHostname.length(); i++) {
        char c = newHostname.charAt(i);
        if (!(isAlphaNumeric(c) || c == '-')) {
            response["success"] = false;
            response["message"] = "Hostname must contain only letters, numbers, and hyphens";
            return Response(request.getServerRequest())
                .status(400)
                .json(response);
        }
    }
    
    // Store in Configuration model
    if (!Configuration::set("hostname", newHostname)) {
        response["success"] = false;
        response["message"] = "Failed to save hostname configuration";
        return Response(request.getServerRequest())
            .status(500)
            .json(response);
    }
    
    // Update current hostname
    WiFi.setHostname(newHostname.c_str());
    
    // Update mDNS
    MDNS.end();
    if (MDNS.begin(newHostname.c_str())) {
        MDNS.addService("http", "tcp", 80);
        response["success"] = true;
        response["message"] = "Hostname updated to: " + newHostname;
        response["hostname"] = newHostname;
        response["mdns"] = newHostname + ".local";
        response["restart_required"] = true;
    } else {
        response["success"] = false;
        response["message"] = "Hostname updated but mDNS failed";
        response["hostname"] = newHostname;
    }
    
    return Response(request.getServerRequest())
            .status(200)
            .json(response);
}
