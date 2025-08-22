#ifndef CSV_DATABASE_H
#define CSV_DATABASE_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <vector>
#include <map>

class CsvDatabase {
private:
    String basePath = "/database/";
    
    // Helper methods
    String escapeValue(const String& value) const;
    String unescapeValue(const String& value) const;
    std::vector<String> parseCsvLine(const String& line) const;
    String buildCsvLine(const std::vector<String>& fields) const;
    
public:
    CsvDatabase();
    
    // File operations
    bool tableExists(const String& tableName) const;
    bool createTable(const String& tableName, const std::vector<String>& columns);
    bool dropTable(const String& tableName);
    
    // Data operations
    std::vector<std::map<String, String>> select(const String& tableName, 
        const std::map<String, String>& where = {}) const;
    
    std::map<String, String> find(const String& tableName, const String& id) const;
    std::map<String, String> findWhere(const String& tableName, 
        const std::map<String, String>& where) const;
    
    bool insert(const String& tableName, const std::map<String, String>& data);
    bool update(const String& tableName, const String& id, 
        const std::map<String, String>& data);
    bool delete_(const String& tableName, const String& id);
    
    // Utility methods
    std::vector<String> getTableColumns(const String& tableName) const;
    int getNextId(const String& tableName) const;
    bool backup(const String& tableName) const;
    bool restore(const String& tableName) const;
    
    // Statistics
    int count(const String& tableName, const std::map<String, String>& where = {}) const;
    std::vector<String> getTables() const;
    
private:
    String getTablePath(const String& tableName) const;
    String getBackupPath(const String& tableName) const;
    bool writeToFile(const String& filePath, const String& content) const;
    String readFromFile(const String& filePath) const;
    std::vector<String> readLines(const String& filePath) const;
    bool matchesWhere(const std::map<String, String>& record, 
        const std::map<String, String>& where) const;
};

#endif
