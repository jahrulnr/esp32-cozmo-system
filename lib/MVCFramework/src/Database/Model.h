#ifndef MODEL_H
#define MODEL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <MVCFramework.h>

class Model {
protected:
    std::map<String, String> attributes;
    std::map<String, String> original;
    bool exists = false;
    String primaryKey = "id";
    String table;
    static CsvDatabase* database;
    
public:
    Model() = default;
    Model(const String& tableName) : table(tableName) {}
    virtual ~Model() = default;
    
    // Database connection
    static void setDatabase(CsvDatabase* db) { database = db; }
    static CsvDatabase* getDatabase() { return database; }
    
    // Attribute management
    void setAttribute(const String& key, const String& value);
    String getAttribute(const String& key, const String& defaultValue = "") const;
    bool hasAttribute(const String& key) const;
    void fill(const std::map<String, String>& data);
    void fill(const JsonDocument& data);
    
    // Getters and setters
    String get(const String& key, const String& defaultValue = "") const { return getAttribute(key, defaultValue); }
    void set(const String& key, const String& value) { setAttribute(key, value); }
    
    // Model state
    bool isDirty(const String& key = "") const;
    bool wasChanged(const String& key = "") const;
    std::map<String, String> getDirty() const;
    std::map<String, String> getChanges() const;
    
    // Persistence (to be implemented with storage backend)
    virtual bool save();
    virtual bool delete_();
    virtual bool remove();
    virtual bool refresh();
    
    // Serialization
    JsonDocument toJson() const;
    String toJsonString() const;
    std::map<String, String> toMap() const { return attributes; }
    
    // Static methods for querying
    static std::vector<Model*> all(const String& tableName);
    static Model* find(const String& tableName, const String& id);
    static Model* findWhere(const String& tableName, const std::map<String, String>& where);
    static std::vector<Model*> where(const String& tableName, const std::map<String, String>& conditions);
    static bool createTable(const String& tableName, const std::vector<String>& columns);
    static Model* find(const String& id);
    static std::vector<Model*> where(const String& column, const String& value);
    
    // Table management
    String getTable() const { return table; }
    void setTable(const String& tableName) { table = tableName; }
    
    // Primary key
    String getKeyName() const { return primaryKey; }
    String getKey() const { return getAttribute(primaryKey); }
    void setKeyName(const String& key) { primaryKey = key; }
    
    // Timestamps (if needed)
    virtual void touch() {}
    
protected:
    void syncOriginal();
    void finishSave();
};

// Helper macros for model creation
#define DECLARE_MODEL(ClassName, TableName) \
class ClassName : public Model { \
public: \
    ClassName() : Model(TableName) {} \
    static std::vector<ClassName*> all(); \
    static ClassName* find(const String& id); \
    static std::vector<ClassName*> where(const String& column, const String& value); \
};

#endif
