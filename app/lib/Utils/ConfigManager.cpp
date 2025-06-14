#include "ConfigManager.h"
#include <Arduino.h>

namespace Utils {

String ConfigManager::configPath = "/config/config.json";
SpiJsonDocument ConfigManager::configDoc;
SpiJsonDocument ConfigManager::defaultConfigDoc;


bool ConfigManager::initialize() {
    // Get a reference to FileManager (either global or local)
    Utils::FileManager* fm = getFileManager();
    if (!fm) {
        Serial.println("Failed to get FileManager");
        return false;
    }
    
    setDefaultConfig();
    return loadConfig();
}

void ConfigManager::setDefaultConfig() {
    // Set default configuration values based on Config.h
    JsonObject dev = defaultConfigDoc["development"];
    dev["enabled"] = true;

    JsonObject cozmo = defaultConfigDoc["cozmo"];
    cozmo["protect"] = true;
    
    JsonObject automation = cozmo["automation"];
    automation["enabled"] = true;
    automation["inactivity_timeout"] = 10000;
    automation["check_interval"] = 1000;
    automation["max_behaviors"] = 50;
    automation["max_behavior_length"] = 512;

    JsonObject camera = defaultConfigDoc["camera"];
    camera["enabled"] = false;
    camera["frame_size"] = "FRAMESIZE_VGA";
    camera["quality"] = 12;
    camera["fps"] = 15;

    JsonObject motor = defaultConfigDoc["motor"];
    motor["enabled"] = true;
    motor["left_pin1"] = 12;
    motor["left_pin2"] = 13;
    motor["right_pin1"] = 10;
    motor["right_pin2"] = 11;

    JsonObject servo = defaultConfigDoc["servo"];
    servo["enabled"] = true;
    servo["head_pin"] = 14;
    servo["hand_pin"] = 12;
    servo["default_head_angle"] = 90;
    servo["default_hand_angle"] = 180;

    // ... add other default configurations ...

    JsonObject misc = defaultConfigDoc["misc"];
    misc["serial_baud_rate"] = 115200;
    misc["debug_enabled"] = true;
}

bool ConfigManager::loadConfig() {
    // Make a fresh copy of defaults
    defaultConfigDoc.clear();
    setDefaultConfig();

    // Try to read the config file
    if (readJsonFile(configPath, configDoc)) {
        // Merge with defaults to ensure all keys exist
        mergeConfigs(configDoc.as<JsonVariant>(), defaultConfigDoc.as<JsonVariant>());
        return true;
    }
    
    // If file doesn't exist or is invalid, use defaults
    configDoc.clear();
    configDoc.set(defaultConfigDoc);
    
    // Try to save default config
    return writeJsonFile(configPath, configDoc);
}

String ConfigManager::getConfigAsJson() {
    String output;
    serializeJsonPretty(configDoc, output);
    return output;
}

bool ConfigManager::saveConfig(const String& configJson) {
    // Parse the JSON string into a temporary document
    SpiJsonDocument tempDoc;
    DeserializationError error = deserializeJson(tempDoc, configJson);
    
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Copy the temporary document to the main config document
    configDoc.set(tempDoc);
    
    // Write the updated config to file
    return writeJsonFile(configPath, configDoc);
}

bool ConfigManager::applyConfigToSystem() {
    bool success = true;
    
    // Handle WiFi configuration
    if (configDoc["wifi"]) {
        JsonObject wifi = configDoc["wifi"].as<JsonObject>();
        if (wifi["enabled"] && wifi["ssid"] && 
            wifi["password"] && wifi["ap_ssid"] && 
            wifi["ap_password"]) {
            
            // Create or update wifi.json with the new settings
            SpiJsonDocument wifiDoc;
            wifiDoc["ssid"] = wifi["ssid"].as<String>();
            wifiDoc["password"] = wifi["password"].as<String>();
            wifiDoc["ap_ssid"] = wifi["ap_ssid"].as<String>();
            wifiDoc["ap_password"] = wifi["ap_password"].as<String>();
            
            // Use FileManager to write the configuration
            String wifiJson;
            serializeJsonPretty(wifiDoc, wifiJson);
            
            // Get a reference to FileManager (either global or local)
            Utils::FileManager* fm = getFileManager();
            if (fm) {
                // Ensure the config directory exists
                if (!fm->exists("/config")) {
                    Serial.println("Config directory doesn't exist, creating it...");
                    if (!fm->createDir("/config")) {
                        Serial.println("Failed to create config directory");
                        success = false;
                    }
                }
                
                // Write the WiFi configuration
                if (!fm->writeFile("/config/wifi.json", wifiJson)) {
                    Serial.println("Failed to write WiFi configuration");
                    success = false;
                }
            } else {
                Serial.println("FileManager not available");
                success = false;
            }
        }
    }
    
    // Apply other runtime configurations
    
    // 1. GPT configuration
    if (configDoc["gpt"]) {
        JsonObject gpt = configDoc["gpt"].as<JsonObject>();
        // Update GPT settings (if application supports runtime update)
    }
    
    // 2. Motor configuration
    if (configDoc["motor"]) {
        JsonObject motor = configDoc["motor"].as<JsonObject>();
        // Update motor settings (if application supports runtime update)
    }
    
    // 3. Servo configuration
    if (configDoc["servo"]) {
        JsonObject servo = configDoc["servo"].as<JsonObject>();
        // Update servo settings (if application supports runtime update)
    }
    
    // 4. Camera configuration
    if (configDoc["camera"]) {
        JsonObject camera = configDoc["camera"].as<JsonObject>();
        // Update camera settings (if application supports runtime update)
    }
    
    // 5. Automation configuration
    if (configDoc["cozmo"] && configDoc["cozmo"]["automation"]) {
        JsonObject automation = configDoc["cozmo"]["automation"].as<JsonObject>();
        
        // Example of updating a runtime configuration value
        if (automation["enabled"]) {
            bool automationEnabled = automation["enabled"].as<bool>();
            // Call global function to update automation state
            setAutomationEnabled(automationEnabled);
        }
    }
    
    // For configurations that require restart, we could create a flag file
    // that the system checks on boot to know if it needs to apply startup-only changes
    Utils::FileManager* fm = getFileManager();
    if (fm) {
        fm->writeFile("/config/.needs_restart", "1");
    }
    
    return success;
}

bool ConfigManager::readJsonFile(const String& filename, SpiJsonDocument& doc) {
    // Get a reference to FileManager (either global or local)
    Utils::FileManager* fm = getFileManager();
    if (!fm) {
        Serial.println("FileManager not available");
        return false;
    }
    
    if (!fm->exists(filename)) {
        Serial.println("File doesn't exist: " + filename);
        return false;
    }
    
    String jsonContent = fm->readFile(filename);
    if (jsonContent.isEmpty()) {
        Serial.println("Failed to read file or file is empty: " + filename);
        return false;
    }
    
    DeserializationError error = deserializeJson(doc, jsonContent);
    
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return false;
    }
    
    return true;
}

bool ConfigManager::writeJsonFile(const String& filename, const SpiJsonDocument& doc) {
    // Get a reference to FileManager (either global or local)
    Utils::FileManager* fm = getFileManager();
    if (!fm) {
        Serial.println("FileManager not available");
        return false;
    }
    
    // Serialize the JSON document to a string
    String jsonContent;
    serializeJsonPretty(doc, jsonContent);
    
    // Write the serialized content to the file using FileManager
    if (!fm->writeFile(filename, jsonContent)) {
        Serial.println("Failed to write to file: " + filename);
        return false;
    }
    
    return true;
}

void ConfigManager::mergeConfigs(JsonVariant dst, JsonVariantConst src) {
    if (!src.is<JsonObject>()) {
        return;
    }
    
    if (!dst.is<JsonObject>()) {
        dst.clear();
        dst.to<JsonObject>();
    }
    
    for (const auto& kvp : src.as<JsonObjectConst>()) {
        if (kvp.value().is<JsonObject>()) {
            // If the value is an object, recurse
            mergeConfigs(dst[kvp.key()], kvp.value());
        } else if (!dst[kvp.key()]) {
            // Only set if key doesn't exist in destination
            dst[kvp.key()] = kvp.value();
        }
    }
}

Utils::FileManager* ConfigManager::getFileManager() {
    if (!fileManager) {
        static Utils::FileManager localFileManager;
        if (!localFileManager.init()) {
            Serial.println("Failed to initialize local FileManager instance");
            return nullptr;
        }
        return &localFileManager;
    }
    return fileManager;
}

} // namespace Utils
