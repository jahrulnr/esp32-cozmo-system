#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "SPIFFS.h"
#include "SpiAllocator.h"
#include "FileManager.h"

namespace Utils {

class ConfigManager {
public:
    ConfigManager();
    static bool initialize(FileManager *fm);
    static bool loadConfig();
    static bool saveConfig(const String& configJson);
    static String getConfigAsJson();
    static bool applyConfigToSystem();

private:
    static String configPath;
    static SpiJsonDocument configDoc;
    static SpiJsonDocument defaultConfigDoc;

    static void setDefaultConfig();
    static bool readJsonFile(const String& filename, SpiJsonDocument& doc);
    static bool writeJsonFile(const String& filename, const SpiJsonDocument& doc);
    static void mergeConfigs(JsonVariant dst, JsonVariantConst src);
    
    // Helper to ensure FileManager is available
    static Utils::FileManager* getFileManager();
};

} // namespace Utils
