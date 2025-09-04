#include <Arduino.h>
#include "setup/setup.h"

Communication::WiFiManager *wifiManager;

void setupWiFi() {
  if (WIFI_ENABLED) {
    logger->info("Setting up WiFi...");
    wifiManager = new Communication::WiFiManager(fileManager);
    wifiManager->init();
    
    // Get config (already loaded from file or defaults in constructor)
    Communication::WiFiManager::WiFiConfig config = wifiManager->getConfig();
    
    if (screen) {
      screen->clear();
      screen->drawCenteredText(20, "Connecting to");
      screen->drawCenteredText(40, config.ssid.c_str());
      screen->update();
    }
    
    // Try to connect to WiFi using saved configuration
    if (wifiManager->connect(config.ssid, config.password, 10000)) {
      logger->info("Connected to WiFi: %s", config.ssid.c_str());
      logger->info("IP: %s", wifiManager->getIP().c_str());
      
      if (screen) {
        screen->clear();
        screen->drawCenteredText(10, "WiFi Connected");
        screen->drawCenteredText(30, config.ssid.c_str());
        screen->drawCenteredText(50, wifiManager->getIP().c_str());
        screen->update();
        delay(2000);
      }
    } else {
      logger->warning("WiFi connection failed, starting AP mode");
      
      if (screen) {
        screen->clear();
        screen->drawCenteredText(20, "Starting AP");
        screen->drawCenteredText(40, config.apSsid.c_str());
        screen->update();
      }
      
      if (wifiManager->startAP(config.apSsid, config.apPassword)) {
        logger->info("AP started: %s", config.apSsid.c_str());
        logger->info("IP: %s", wifiManager->getIP().c_str());
        
        if (screen) {
          screen->clear();
          screen->drawCenteredText(10, "AP Mode Active");
          screen->drawCenteredText(30, config.apSsid.c_str());
          screen->drawCenteredText(50, wifiManager->getIP().c_str());
          screen->update();
          delay(2000);
        }
      } else {
        logger->error("AP start failed");
      }
    }
  }
}