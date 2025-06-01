#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

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

    WiFiManager();
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

private:
    bool _initialized;
    bool _isAP;
};

} // namespace Communication
