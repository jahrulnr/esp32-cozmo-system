#include "WiFiManager.h"

namespace Communication {

WiFiManager::WiFiManager() : _initialized(false), _isAP(false) {
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

} // namespace Communication
