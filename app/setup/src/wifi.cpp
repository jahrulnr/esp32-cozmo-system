#include <Arduino.h>
#include "setup/setup.h"

Services::WiFiManager *wifiManager;

void setupWiFi() {
  if (WIFI_ENABLED) {
    logger->info("Setting up WiFi...");
    wifiManager = new Services::WiFiManager(fileManager);
    wifiManager->init();
    
    // Get config (already loaded from file or defaults in constructor)
    Services::WiFiManager::WiFiConfig config = wifiManager->getConfig();
    
    if (display) {
      display->clear();
      display->drawCenteredText(20, "Connecting to");
      display->drawCenteredText(40, config.ssid.c_str());
      display->update();
    }
    
    // Try to connect to WiFi using saved configuration
    if (wifiManager->connect(config.ssid, config.password, 10000)) {
      logger->info("Connected to WiFi: %s", config.ssid.c_str());
      logger->info("IP: %s", wifiManager->getIP().c_str());
      
      if (display) {
        display->clear();
        display->drawCenteredText(10, "WiFi Connected");
        display->drawCenteredText(30, config.ssid.c_str());
        display->drawCenteredText(50, wifiManager->getIP().c_str());
        display->update();
        delay(2000);
      }
    } else {
      logger->warning("WiFi connection failed, starting AP mode");
      
      if (display) {
        display->clear();
        display->drawCenteredText(20, "Starting AP");
        display->drawCenteredText(40, config.apSsid.c_str());
        display->update();
      }
      
      if (wifiManager->startAP(config.apSsid, config.apPassword)) {
        logger->info("AP started: %s", config.apSsid.c_str());
        logger->info("IP: %s", wifiManager->getIP().c_str());
        
        if (display) {
          display->clear();
          display->drawCenteredText(10, "AP Mode Active");
          display->drawCenteredText(30, config.apSsid.c_str());
          display->drawCenteredText(50, wifiManager->getIP().c_str());
          display->update();
          delay(2000);
        }
      } else {
        logger->error("AP start failed");
      }
    }
  }
}