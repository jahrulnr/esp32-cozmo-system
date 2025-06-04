#include <Arduino.h>
#include "init.h"

void setupHealthCheck() {
  if (HEALTH_CHECK_ENABLED) {
    logger->info("Setting up health checks...");
    healthCheck = new Utils::HealthCheck();
    healthCheck->init(HEALTH_CHECK_INTERVAL);
    
    // Add health checks
    healthCheck->addCheck("WiFi", []() {
      if (!wifiManager || !wifiManager->isConnected()) {
        return Utils::HealthCheck::Status::WARNING;
      }
      return Utils::HealthCheck::Status::HEALTHY;
    });
    
    healthCheck->addCheck("Camera", []() {
      if (!camera) {
        return Utils::HealthCheck::Status::WARNING;
      }
      return Utils::HealthCheck::Status::HEALTHY;
    });
    
    // Set callback for status changes
    healthCheck->setStatusChangeCallback([](const String& name, Utils::HealthCheck::Status oldStatus, Utils::HealthCheck::Status newStatus) {
      logger->info("Health check '" + name + "' changed from " + String(static_cast<int>(oldStatus)) + " to " + String(static_cast<int>(newStatus)));
    });
    
    logger->info("Health checks initialized");
  }
}