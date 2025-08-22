#include "Model.h"

// Initialize static database pointer
CsvDatabase* Model::database = nullptr;

void Model::setAttribute(const String& key, const String& value) {
    attributes[key] = value;
}

String Model::getAttribute(const String& key, const String& defaultValue) const {
    auto it = attributes.find(key);
    if (it != attributes.end()) {
        return it->second;
    }
    return defaultValue;
}

bool Model::hasAttribute(const String& key) const {
    return attributes.find(key) != attributes.end();
}

void Model::fill(const std::map<String, String>& data) {
    for (const auto& pair : data) {
        setAttribute(pair.first, pair.second);
    }
}

void Model::fill(const JsonDocument& data) {
		JsonObjectConst items = data.as<JsonObjectConst>();
    for (JsonPairConst kv : items) {
        setAttribute(kv.key().c_str(), kv.value().as<String>());
    }
}

bool Model::isDirty(const String& key) const {
    if (key.length() > 0) {
        // Check specific attribute
        auto origIt = original.find(key);
        auto attrIt = attributes.find(key);
        
        if (origIt == original.end() && attrIt == attributes.end()) {
            return false; // Both don't exist
        }
        
        if (origIt == original.end() || attrIt == attributes.end()) {
            return true; // One exists, the other doesn't
        }
        
        return origIt->second != attrIt->second;
    } else {
        // Check if any attribute is dirty
        return getDirty().size() > 0;
    }
}

bool Model::wasChanged(const String& key) const {
    return isDirty(key);
}

std::map<String, String> Model::getDirty() const {
    std::map<String, String> dirty;
    
    for (const auto& attr : attributes) {
        auto origIt = original.find(attr.first);
        if (origIt == original.end() || origIt->second != attr.second) {
            dirty[attr.first] = attr.second;
        }
    }
    
    return dirty;
}

std::map<String, String> Model::getChanges() const {
    return getDirty();
}

bool Model::save() {
    if (!database || table.length() == 0) {
        return false;
    }
    
    // Ensure table exists
    if (!database->tableExists(table)) {
        // Create table with current attributes as columns
        std::vector<String> columns;
        for (const auto& attr : attributes) {
            if (attr.first != "id") {
                columns.push_back(attr.first);
            }
        }
        database->createTable(table, columns);
    }
    
    // Call touch to update timestamps
    touch();
    
    bool success = false;
    
    if (exists && hasAttribute(primaryKey)) {
        // Update existing record
        success = database->update(table, getAttribute(primaryKey), attributes);
    } else {
        // Insert new record
        success = database->insert(table, attributes);
        if (success && !hasAttribute(primaryKey)) {
            // Get the auto-generated ID
            int nextId = database->getNextId(table) - 1; // We just inserted, so subtract 1
            setAttribute(primaryKey, String(nextId));
        }
        exists = true;
    }
    
    if (success) {
        // Sync original with current state
        syncOriginal();
        
        // Call finish save hook
        finishSave();
    }
    
    return success;
}

bool Model::remove() { return delete_(); }

bool Model::delete_() {
    if (!database || table.length() == 0 || !exists || !hasAttribute(primaryKey)) {
        return false;
    }
    
    bool success = database->delete_(table, getAttribute(primaryKey));
    if (success) {
        exists = false;
    }
    
    return success;
}

bool Model::refresh() {
    if (!database || table.length() == 0 || !hasAttribute(primaryKey)) {
        return false;
    }
    
    auto record = database->find(table, getAttribute(primaryKey));
    if (record.empty()) {
        return false;
    }
    
    // Update attributes with fresh data
    attributes = record;
    syncOriginal();
    exists = true;
    
    return true;
}

JsonDocument Model::toJson() const {
    JsonDocument doc;
    
    for (const auto& attr : attributes) {
        doc[attr.first] = attr.second;
    }
    
    return doc;
}

String Model::toJsonString() const {
    JsonDocument doc = toJson();
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

void Model::syncOriginal() {
    original = attributes;
}

void Model::finishSave() {
    // Hook for subclasses to implement post-save logic
}

// Static methods for querying
std::vector<Model*> Model::all(const String& tableName) {
    std::vector<Model*> models;
    
    if (!database) {
        return models;
    }
    
    auto records = database->select(tableName);
    for (const auto& record : records) {
        Model* model = new Model(tableName);
        model->fill(record);
        model->syncOriginal();
        model->exists = true;
        models.push_back(model);
    }
    
    return models;
}

Model* Model::find(const String& tableName, const String& id) {
    if (!database) {
        return nullptr;
    }
    
    auto record = database->find(tableName, id);
    if (record.empty()) {
        return nullptr;
    }
    
    Model* model = new Model(tableName);
    model->fill(record);
    model->syncOriginal();
    model->exists = true;
    
    return model;
}

Model* Model::findWhere(const String& tableName, const std::map<String, String>& where) {
    if (!database) {
        return nullptr;
    }
    
    auto record = database->findWhere(tableName, where);
    if (record.empty()) {
        return nullptr;
    }
    
    Model* model = new Model(tableName);
    model->fill(record);
    model->syncOriginal();
    model->exists = true;
    
    return model;
}

std::vector<Model*> Model::where(const String& tableName, const std::map<String, String>& conditions) {
    std::vector<Model*> models;
    
    if (!database) {
        return models;
    }
    
    auto records = database->select(tableName, conditions);
    for (const auto& record : records) {
        Model* model = new Model(tableName);
        model->fill(record);
        model->syncOriginal();
        model->exists = true;
        models.push_back(model);
    }
    
    return models;
}

bool Model::createTable(const String& tableName, const std::vector<String>& columns) {
    if (!database) {
        return false;
    }
    
    return database->createTable(tableName, columns);
}
