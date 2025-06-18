#include <Arduino.h>
#include "app.h"
#include "lib/Utils/ConfigManager.h"

void setupConfigManager() {
    if (Utils::ConfigManager::initialize(fileManager)) {
        logger->info("ConfigManager initialized");
    } else {
        logger->error("Failed to initialize ConfigManager");
    }
}
