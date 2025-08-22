#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include "FileManager.h"

namespace Communication {

class WiFiManager {
public:
    struct NetworkInfo {
        String ssid;
        int32_t rssi;
        uint8_t encryptionType;
        String bssid;
        int32_t channel;
    };
    
    struct WiFiConfig {
        String ssid;
        String password;
        String apSsid;
        String apPassword;
    };

    WiFiManager(Utils::FileManager *fileManager);
    ~WiFiManager();

    /**
     * Initialize WiFi manager
     * @return true if initialization was successful, false otherwise
     */
    bool init();

    /**
     * Connect to a WiFi network
     * @param ssid The network SSID
     * @param password The network password
     * @param timeout Connection timeout in milliseconds
     * @return true if connection was successful, false otherwise
     */
    bool connect(const String& ssid, const String& password, uint32_t timeout = 30000);

    /**
     * Start access point mode
     * @param ssid The AP SSID
     * @param password The AP password (empty for open network)
     * @return true if AP was started successfully, false otherwise
     */
    bool startAP(const String& ssid, const String& password = "");

    /**
     * Check if WiFi is connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * Disconnect from WiFi network
     */
    void disconnect();

    /**
     * Scan for available networks
     * @return Vector of NetworkInfo structures
     */
    std::vector<NetworkInfo> scanNetworks();

    /**
     * Get the current IP address
     * @return IP address as a string
     */
    String getIP() const;

    /**
     * Get the current MAC address
     * @return MAC address as a string
     */
    String getMAC() const;

    /**
     * Get the current RSSI (signal strength)
     * @return RSSI value in dBm
     */
    int32_t getRSSI() const;
    
    /**
     * Load WiFi configuration from the filesystem
     * @return true if configuration was loaded successfully, false otherwise
     */
    bool loadConfig();
    
    /**
     * Save WiFi configuration to the filesystem
     * @param config The WiFi configuration to save
     * @return true if configuration was saved successfully, false otherwise
     */
    bool saveConfig(const WiFiConfig& config);
    
    /**
     * Get the current WiFi configuration
     * @return The current WiFi configuration
     */
    WiFiConfig getConfig() const;
    
    /**
     * Update the WiFi configuration
     * @param config The new WiFi configuration
     * @return true if configuration was updated successfully, false otherwise
     */
    bool updateConfig(const WiFiConfig& config);

private:
    bool _initialized;
    bool _isAP;
    WiFiConfig _config;
    Utils::FileManager *_fileManager;
};

} // namespace Communication
