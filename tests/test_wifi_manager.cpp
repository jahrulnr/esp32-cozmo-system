#include <Arduino.h>
#include "lib/Communication/WiFiManager.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Testing WiFiManager with FileManager");
  
  Communication::WiFiManager wifiManager;
  
  // Initialize
  if (wifiManager.init()) {
    Serial.println("WiFiManager initialized successfully");
  } else {
    Serial.println("WiFiManager initialization failed");
    return;
  }
  
  // Load config
  if (wifiManager.loadConfig()) {
    Serial.println("WiFi config loaded successfully");
    
    Communication::WiFiManager::WiFiConfig config = wifiManager.getConfig();
    Serial.println("Current config:");
    Serial.println("  SSID: " + config.ssid);
    Serial.println("  AP SSID: " + config.apSsid);
  } else {
    Serial.println("Failed to load WiFi config, using defaults");
  }
  
  // Save config with modified values
  Communication::WiFiManager::WiFiConfig newConfig;
  newConfig.ssid = "TestSSID";
  newConfig.password = "TestPassword";
  newConfig.apSsid = "TestAPSSID";
  newConfig.apPassword = "TestAPPassword";
  
  if (wifiManager.saveConfig(newConfig)) {
    Serial.println("New config saved successfully");
  } else {
    Serial.println("Failed to save new config");
  }
  
  // Load config again to verify
  if (wifiManager.loadConfig()) {
    Communication::WiFiManager::WiFiConfig config = wifiManager.getConfig();
    Serial.println("Loaded config after save:");
    Serial.println("  SSID: " + config.ssid);
    Serial.println("  AP SSID: " + config.apSsid);
  }
}

void loop() {
  delay(1000);
}
