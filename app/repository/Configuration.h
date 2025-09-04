#ifndef CONFIGURATION_MODEL_H
#define CONFIGURATION_MODEL_H

#include "Database/Model.h"
#include <Arduino.h>

class Configuration : public Model {
public:
    // Constructor
    Configuration() : Model("configurations") {}
    
    // Create a new configuration instance
    Configuration(const String& key, const String& value) : Model("configurations") {
        setKey(key);
        setValue(value);
    }
    
    // Getters
    String getKey() const { return get("key"); }
    String getValue() const { return get("value"); }
    
    // Setters
    void setKey(const String& key) { set("key", key); }
    void setValue(const String& value) { set("value", value); }
    
    // Static methods to work with configurations
    
    // Initialize configurations table
    static bool initTable() {
        // Get the database from Model
        CsvDatabase* db = Model::getDatabase();
        if (!db) return false;
        
        // Create table if it doesn't exist
        if (!db->tableExists("configurations")) {
            std::vector<String> columns = {"key", "value"};
            return db->createTable("configurations", columns);
        }
        
        return true;
    }
    
    // Get a configuration value by key
    static String get(const String& key, const String& defaultValue = "") {
        Configuration* config = findByKey(key);
        if (!config) {
            return defaultValue;
        }
        
        String value = config->getValue();
        delete config;
        return value;
    }
    
    // Set a configuration value
    static bool set(const String& key, const String& value) {
        Configuration* config = findByKey(key);
        bool result = false;
        
        if (config) {
            // Update existing
            config->setValue(value);
            result = config->save();
            delete config;
        } else {
            // Create new
            Configuration newConfig(key, value);
            result = newConfig.save();
        }
        
        return result;
    }
    
    // Find a configuration by key
    static Configuration* findByKey(const String& key) {
        CsvDatabase* db = Model::getDatabase();
        if (!db) return nullptr;
        
        // Make sure table exists
        if (!db->tableExists("configurations")) {
            if (!initTable()) {
                return nullptr;
            }
        }
        
        // Find by key
        std::map<String, String> conditions;
        conditions["key"] = key;
        
        std::vector<std::map<String, String>> results = db->select("configurations", conditions);
        
        if (results.size() > 0) {
            Configuration* config = new Configuration();
            config->fill(results[0]);
            return config;
        }
        
        return nullptr;
    }
};

#endif // CONFIGURATION_MODEL_H
