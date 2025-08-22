#include "Config.h"
#include <SPIFFS.h>

Config::Config() {
    // Set default values
    set("app.name", "ESP32 MVC App");
    set("app.env", "production");
    set("app.debug", "false");
    set("server.host", "0.0.0.0");
    set("server.port", "80");
}

void Config::load() {
    // Try to load config from SPIFFS
    if (SPIFFS.exists("/config.json")) {
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
            String configString = configFile.readString();
            configFile.close();
            
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, configString);
            
            if (!error) {
                for (JsonPair kv : doc.as<JsonObject>()) {
                    set(kv.key().c_str(), kv.value().as<String>());
                }
                Serial.println("Configuration loaded from file");
            } else {
                Serial.println("Failed to parse config file");
            }
        }
    } else {
        Serial.println("No config file found, using defaults");
    }
}

void Config::set(const String& key, const String& value) {
    values[key] = value;
}

String Config::get(const String& key, const String& defaultValue) {
    auto it = values.find(key);
    if (it != values.end()) {
        return it->second;
    }
    return defaultValue;
}

int Config::getInt(const String& key, int defaultValue) {
    String value = get(key);
    if (value.length() > 0) {
        return value.toInt();
    }
    return defaultValue;
}

bool Config::getBool(const String& key, bool defaultValue) {
    String value = get(key);
    if (value.length() > 0) {
        return value.equalsIgnoreCase("true") || value == "1";
    }
    return defaultValue;
}
