#include <Arduino.h>
#include "app.h"

// Global flag to track if we're running in AP-only mode
bool g_isApOnlyMode = false;

void setupWiFi() {
  if (WIFI_ENABLED) {
    logger->info("Setting up WiFi...");
    wifiManager = new Communication::WiFiManager(fileManager);
    wifiManager->init();
    
    // Get config (already loaded from file or defaults in constructor)
    Communication::WiFiManager::WiFiConfig config = wifiManager->getConfig();
    
    // Log the current config source - note: config was already loaded in constructor
    if (fileManager->exists("/config/wifi.json")) {
      logger->info("Using Wi-Fi configuration from wifi.json file");
    } else {
      logger->info("Using default Wi-Fi configuration from Config.h");
    }
    
    if (screen) {
      screen->clear();
      screen->drawCenteredText(20, "Connecting to");
      screen->drawCenteredText(40, config.ssid);
      screen->update();
    }
    
    // Try to connect to WiFi using saved configuration
    if (wifiManager->connect(config.ssid, config.password)) {
      logger->info("Connected to WiFi: " + config.ssid);
      logger->info("IP: " + wifiManager->getIP());
      
      // Set AP-only mode flag to false since we're connected
      g_isApOnlyMode = false;
      
      if (screen) {
        screen->clear();
        screen->drawCenteredText(10, "WiFi Connected");
        screen->drawCenteredText(30, config.ssid);
        screen->drawCenteredText(50, wifiManager->getIP());
        screen->update();
        delay(2000);
      }
    } else {
      logger->warning("WiFi connection failed, starting AP mode");
      
      // Set AP-only mode flag to true since we're in fallback mode
      g_isApOnlyMode = true;
      
      if (screen) {
        screen->clear();
        screen->drawCenteredText(20, "Starting AP");
        screen->drawCenteredText(40, config.apSsid);
        screen->update();
      }
      
      if (wifiManager->startAP(config.apSsid, config.apPassword)) {
        logger->info("AP started: " + config.apSsid);
        logger->info("IP: " + wifiManager->getIP());
        
        if (screen) {
          screen->clear();
          screen->drawCenteredText(10, "AP Mode Active");
          screen->drawCenteredText(30, config.apSsid);
          screen->drawCenteredText(50, wifiManager->getIP());
          screen->update();
          delay(2000);
        }
      } else {
        logger->error("AP start failed");
      }
    }
  }
}

// Helper function to determine if we're running in AP-only mode
bool isApOnlyMode() {
  return g_isApOnlyMode;
}