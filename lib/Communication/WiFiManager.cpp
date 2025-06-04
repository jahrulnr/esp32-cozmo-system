#include "WiFiManager.h"
#include "../../include/Config.h"

#include "../Utils/SpiAllocatorUtils.h"
#include "SPIFFS.h"

namespace Communication {

WiFiManager::WiFiManager() : _initialized(false), _isAP(false) {
    // Initialize with default config from Config.h
    _config.ssid = WIFI_SSID;
    _config.password = WIFI_PASSWORD;
    _config.apSsid = WIFI_AP_SSID;
    _config.apPassword = WIFI_AP_PASSWORD;
    
    // Initialize SPIFFS and ensure config directory exists
    if (SPIFFS.begin(true)) {
        if (!SPIFFS.exists("/config")) {
            if (!SPIFFS.mkdir("/config")) {
                Serial.println("Failed to create /config directory");
            } else {
                Serial.println("Created /config directory");
            }
        }
    }
    
    // Try to load config from file
    loadConfig();
}

WiFiManager::~WiFiManager() {
    disconnect();
}

bool WiFiManager::init() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    _initialized = true;
    
    // Debug: List all files in SPIFFS to help diagnose issues
    if (SPIFFS.begin(true)) {
        Serial.println("SPIFFS init successful in init()");
        Serial.println("SPIFFS files in root:");
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while(file) {
            Serial.print("  ");
            Serial.println(file.name());
            file = root.openNextFile();
        }
        Serial.println("End of file list");
    } else {
        Serial.println("SPIFFS init failed in init()");
    }
    
    return true;
}

bool WiFiManager::connect(const String& ssid, const String& password, uint32_t timeout) {
    if (!_initialized) {
        init();
    }
    
    // Disconnect if already connected or in AP mode
    disconnect();
    
    // Begin connection
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection with timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("\nWiFi connection failed");
        return false;
    }
}

bool WiFiManager::startAP(const String& ssid, const String& password) {
    if (!_initialized) {
        init();
    }
    
    // Disconnect if already connected
    disconnect();
    
    // Set mode to AP
    WiFi.mode(WIFI_AP);
    
    // Start AP
    bool success;
    if (password.length() > 0) {
        success = WiFi.softAP(ssid.c_str(), password.c_str());
    } else {
        success = WiFi.softAP(ssid.c_str());
    }
    
    if (success) {
        _isAP = true;
        Serial.println("AP started");
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("AP failed to start");
    }
    
    return success;
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::disconnect() {
    if (_isAP) {
        WiFi.softAPdisconnect(true);
        _isAP = false;
    } else {
        WiFi.disconnect(true);
    }
    delay(100);
}

std::vector<WiFiManager::NetworkInfo> WiFiManager::scanNetworks() {
    std::vector<NetworkInfo> networks;
    
    int numNetworks = WiFi.scanNetworks();
    for (int i = 0; i < numNetworks; i++) {
        NetworkInfo network;
        network.ssid = WiFi.SSID(i);
        network.rssi = WiFi.RSSI(i);
        network.encryptionType = WiFi.encryptionType(i);
        network.bssid = WiFi.BSSIDstr(i);
        network.channel = WiFi.channel(i);
        networks.push_back(network);
    }
    
    return networks;
}

String WiFiManager::getIP() const {
    if (_isAP) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}

String WiFiManager::getMAC() const {
    if (_isAP) {
        return WiFi.softAPmacAddress();
    } else {
        return WiFi.macAddress();
    }
}

int32_t WiFiManager::getRSSI() const {
    return WiFi.RSSI();
}

bool WiFiManager::loadConfig() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }
    
    // Debug output to help diagnose the issue
    Serial.println("SPIFFS mounted successfully");
    Serial.println("Checking for wifi.json...");
    
    if (!SPIFFS.exists("/config/wifi.json")) {
        Serial.println("No wifi.json found at /config/wifi.json, using default config");
        return false;
    }
    
    Serial.println("Found wifi.json, opening file");
    File file = SPIFFS.open("/config/wifi.json", "r");
    if (!file) {
        Serial.println("Failed to open wifi.json");
        return false;
    }
    
    Utils::SpiJsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.println("Failed to parse wifi.json: " + String(error.c_str()));
        
        // If we had a parsing error, rename the broken config file for debugging
        if (SPIFFS.exists("/config/wifi.json")) {
            SPIFFS.rename("/config/wifi.json", "/config/wifi.json.broken");
            Serial.println("Renamed broken config to wifi.json.broken");
        }
        
        return false;
    }
    
    // Check if all required fields exist
    bool isValid = !doc["ssid"].isUnbound() && !doc["password"].isUnbound() && 
                   !doc["ap_ssid"].isUnbound() && !doc["ap_password"].isUnbound();
    
    if (!isValid) {
        Serial.println("wifi.json is missing required fields");
        return false;
    }
    
    // Update config with values from file
    _config.ssid = doc["ssid"].as<String>();
    _config.password = doc["password"].as<String>();
    _config.apSsid = doc["ap_ssid"].as<String>();
    _config.apPassword = doc["ap_password"].as<String>();
    
    // Validate that we have at least AP settings (minimum required)
    if (_config.apSsid.length() == 0) {
        Serial.println("Warning: AP SSID is empty in config, using default");
        _config.apSsid = WIFI_AP_SSID;
    }
    
    Serial.println("WiFi config loaded from file");
    return true;
}

bool WiFiManager::saveConfig(const WiFiConfig& config) {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }
    
    // Create /config directory if it doesn't exist
    if (!SPIFFS.exists("/config")) {
        if (!SPIFFS.mkdir("/config")) {
            Serial.println("Failed to create /config directory");
        }
    }
    
    // Create a backup of the existing file if it exists
    if (SPIFFS.exists("/config/wifi.json")) {
        if (SPIFFS.exists("/config/wifi.json.bak")) {
            SPIFFS.remove("/config/wifi.json.bak");
        }
        if (!SPIFFS.rename("/config/wifi.json", "/config/wifi.json.bak")) {
            Serial.println("Warning: Failed to create backup of wifi.json");
        } else {
            Serial.println("Created backup of previous wifi.json");
        }
    }
    
    // Open file for writing
    File file = SPIFFS.open("/config/wifi.json", "w");
    if (!file) {
        Serial.println("Failed to open wifi.json for writing");
        return false;
    }
    
    // Validate config before saving
    WiFiConfig validConfig = config;
    
    // Ensure AP settings are always set (minimum required for recovery)
    if (validConfig.apSsid.isEmpty()) {
        validConfig.apSsid = WIFI_AP_SSID;
    }
    if (validConfig.apPassword.isEmpty()) {
        validConfig.apPassword = WIFI_AP_PASSWORD;
    }
    
    // Create JSON document
    Utils::SpiJsonDocument doc;
    doc["ssid"] = validConfig.ssid;
    doc["password"] = validConfig.password;
    doc["ap_ssid"] = validConfig.apSsid;
    doc["ap_password"] = validConfig.apPassword;
    
    // Write to file
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write wifi.json");
        file.close();
        return false;
    }
    
    file.close();
    
    // Verify that the file was written correctly
    if (SPIFFS.exists("/config/wifi.json")) {
        File verifyFile = SPIFFS.open("/config/wifi.json", "r");
        if (verifyFile && verifyFile.size() > 10) {
            verifyFile.close();
            Serial.println("WiFi config saved to file");
            return true;
        }
        if (verifyFile) verifyFile.close();
    }
    
    // If verification failed, restore backup if available
    Serial.println("WiFi config verification failed, attempting to restore backup");
    if (SPIFFS.exists("/config/wifi.json.bak")) {
        if (SPIFFS.rename("/config/wifi.json.bak", "/config/wifi.json")) {
            Serial.println("Restored backup wifi.json");
        }
    }
    
    return false;
}

WiFiManager::WiFiConfig WiFiManager::getConfig() const {
    return _config;
}

bool WiFiManager::updateConfig(const WiFiConfig& config) {
    _config = config;
    return saveConfig(_config);
}

} // namespace Communication
