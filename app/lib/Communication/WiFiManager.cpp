#include "WiFiManager.h"
#include "Config.h"

#include "lib/Utils/SpiAllocator.h"
#include "lib/Utils/FileManager.h"

namespace Communication {

WiFiManager::WiFiManager(Utils::FileManager *fileManager) : _initialized(false), _isAP(false) {
    // Initialize with default config from Config.h
    _config.ssid = WIFI_SSID;
    _config.password = WIFI_PASSWORD;
    _config.apSsid = WIFI_AP_SSID;
    _config.apPassword = WIFI_AP_PASSWORD;
    
    if (!fileManager) {
        fileManager = new Utils::FileManager();
        fileManager->init();
    }

    _fileManager = fileManager;

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
    // Debug output to help diagnose the issue
    Serial.println("FileManager initialized successfully");
    Serial.println("Checking for wifi.json...");
    
    if (!_fileManager->exists("/config/wifi.json")) {
        Serial.println("No wifi.json found at /config/wifi.json, using default config");
        return false;
    }
    
    Serial.println("Found wifi.json, reading file");
    String jsonContent = _fileManager->readFile("/config/wifi.json");
    if (jsonContent.isEmpty()) {
        Serial.println("Failed to read wifi.json or file is empty");
        return false;
    }
    
    Utils::SpiJsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    
    if (error) {
        Serial.println("Failed to parse wifi.json: " + String(error.c_str()));
        
        // If we had a parsing error, rename the broken config file for debugging
        if (_fileManager->exists("/config/wifi.json")) {
            // Create backup of broken file
            _fileManager->writeFile("/config/wifi.json.broken", jsonContent);
            _fileManager->deleteFile("/config/wifi.json");
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
    if (!_fileManager->init()) {
        Serial.println("Failed to initialize FileManager");
        return false;
    }
    
    // Create /config directory if it doesn't exist
    if (!_fileManager->exists("/config")) {
        if (!_fileManager->createDir("/config")) {
            Serial.println("Failed to create /config directory");
        }
    }
    
    // Create a backup of the existing file if it exists
    if (_fileManager->exists("/config/wifi.json")) {
        if (_fileManager->exists("/config/wifi.json.bak")) {
            _fileManager->deleteFile("/config/wifi.json.bak");
        }
        
        // Create backup by reading and writing to new file
        String backupContent = _fileManager->readFile("/config/wifi.json");
        if (!backupContent.isEmpty()) {
            if (!_fileManager->writeFile("/config/wifi.json.bak", backupContent)) {
                Serial.println("Warning: Failed to create backup of wifi.json");
            } else {
                Serial.println("Created backup of previous wifi.json");
            }
        }
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
    
    // Serialize to string
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Write to file
    if (!_fileManager->writeFile("/config/wifi.json", jsonString)) {
        Serial.println("Failed to write wifi.json");
        return false;
    }
    
    // Verify that the file was written correctly
    if (_fileManager->exists("/config/wifi.json")) {
        int fileSize = _fileManager->getSize("/config/wifi.json");
        if (fileSize > 10) {
            Serial.println("WiFi config saved to file");
            return true;
        }
    }
    
    // If verification failed, restore backup if available
    Serial.println("WiFi config verification failed, attempting to restore backup");
    if (_fileManager->exists("/config/wifi.json.bak")) {
        String backupContent = _fileManager->readFile("/config/wifi.json.bak");
        if (!backupContent.isEmpty()) {
            if (_fileManager->writeFile("/config/wifi.json", backupContent)) {
                Serial.println("Restored backup wifi.json");
            }
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
