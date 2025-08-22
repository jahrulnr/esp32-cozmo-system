#include "CsvDatabase.h"

CsvDatabase::CsvDatabase() {
    // Ensure database directory exists
    if (!SPIFFS.exists(basePath)) {
        // Create directory structure (SPIFFS doesn't have mkdir, so we create a dummy file)
        File file = SPIFFS.open(basePath + ".keep", "w");
        if (file) {
            file.print("");
            file.close();
        }
    }
}

String CsvDatabase::getTablePath(const String& tableName) const {
    return basePath + tableName + ".csv";
}

String CsvDatabase::getBackupPath(const String& tableName) const {
    return basePath + tableName + ".backup.csv";
}

String CsvDatabase::escapeValue(const String& value) const {
    String escaped = value;
    
    // Replace quotes with double quotes
    escaped.replace("\"", "\"\"");
    
    // If value contains comma, quote, or newline, wrap in quotes
    if (escaped.indexOf(',') >= 0 || escaped.indexOf('"') >= 0 || 
        escaped.indexOf('\n') >= 0 || escaped.indexOf('\r') >= 0) {
        escaped = "\"" + escaped + "\"";
    }
    
    return escaped;
}

String CsvDatabase::unescapeValue(const String& value) const {
    String unescaped = value;
    
    // Remove surrounding quotes if present
    if (unescaped.startsWith("\"") && unescaped.endsWith("\"")) {
        unescaped = unescaped.substring(1, unescaped.length() - 1);
        // Replace double quotes with single quotes
        unescaped.replace("\"\"", "\"");
    }
    
    return unescaped;
}

std::vector<String> CsvDatabase::parseCsvLine(const String& line) const {
    std::vector<String> fields;
    String currentField = "";
    bool inQuotes = false;
    
    for (int i = 0; i < line.length(); i++) {
        char c = line.charAt(i);
        
        if (c == '"' && !inQuotes) {
            inQuotes = true;
        } else if (c == '"' && inQuotes) {
            // Check if next character is also a quote (escaped quote)
            if (i + 1 < line.length() && line.charAt(i + 1) == '"') {
                currentField += '"';
                i++; // Skip next quote
            } else {
                inQuotes = false;
            }
        } else if (c == ',' && !inQuotes) {
            fields.push_back(currentField);
            currentField = "";
        } else {
            currentField += c;
        }
    }
    
    // Add last field
    fields.push_back(currentField);
    
    return fields;
}

String CsvDatabase::buildCsvLine(const std::vector<String>& fields) const {
    String line = "";
    
    for (size_t i = 0; i < fields.size(); i++) {
        if (i > 0) {
            line += ",";
        }
        line += escapeValue(fields[i]);
    }
    
    return line;
}

bool CsvDatabase::tableExists(const String& tableName) const {
    return SPIFFS.exists(getTablePath(tableName));
}

bool CsvDatabase::createTable(const String& tableName, const std::vector<String>& columns) {
    if (tableExists(tableName)) {
        return false; // Table already exists
    }
    
    // Create header line
    std::vector<String> headers;
    headers.push_back("id"); // Always include ID column
    
    for (const String& col : columns) {
        if (col != "id") { // Don't duplicate ID column
            headers.push_back(col);
        }
    }
    
    String headerLine = buildCsvLine(headers);
    return writeToFile(getTablePath(tableName), headerLine + "\n");
}

bool CsvDatabase::dropTable(const String& tableName) {
    if (!tableExists(tableName)) {
        return false;
    }
    
    return SPIFFS.remove(getTablePath(tableName));
}

std::vector<String> CsvDatabase::getTableColumns(const String& tableName) const {
    std::vector<String> columns;
    
    if (!tableExists(tableName)) {
        return columns;
    }
    
    File file = SPIFFS.open(getTablePath(tableName), "r");
    if (!file) {
        return columns;
    }
    
    if (file.available()) {
        String headerLine = file.readStringUntil('\n');
        headerLine.trim();
        columns = parseCsvLine(headerLine);
    }
    
    file.close();
    return columns;
}

std::vector<std::map<String, String>> CsvDatabase::select(const String& tableName, 
    const std::map<String, String>& where) const {
    
    std::vector<std::map<String, String>> results;
    
    if (!tableExists(tableName)) {
        return results;
    }
    
    File file = SPIFFS.open(getTablePath(tableName), "r");
    if (!file) {
        return results;
    }
    
    // Read header
    std::vector<String> columns;
    if (file.available()) {
        String headerLine = file.readStringUntil('\n');
        headerLine.trim();
        columns = parseCsvLine(headerLine);
    }
    
    // Read data rows
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        
        if (line.length() == 0) continue;
        
        std::vector<String> values = parseCsvLine(line);
        
        // Create record map
        std::map<String, String> record;
        for (size_t i = 0; i < columns.size() && i < values.size(); i++) {
            record[columns[i]] = unescapeValue(values[i]);
        }
        
        // Check where conditions
        if (matchesWhere(record, where)) {
            results.push_back(record);
        }
    }
    
    file.close();
    return results;
}

std::map<String, String> CsvDatabase::find(const String& tableName, const String& id) const {
    std::map<String, String> where;
    where["id"] = id;
    return findWhere(tableName, where);
}

std::map<String, String> CsvDatabase::findWhere(const String& tableName, 
    const std::map<String, String>& where) const {
    
    auto results = select(tableName, where);
    if (results.size() > 0) {
        return results[0];
    }
    
    return std::map<String, String>();
}

bool CsvDatabase::insert(const String& tableName, const std::map<String, String>& data) {
    if (!tableExists(tableName)) {
        return false;
    }
    
    std::vector<String> columns = getTableColumns(tableName);
    if (columns.empty()) {
        return false;
    }
    
    // Auto-generate ID if not provided
    std::map<String, String> insertData = data;
    if (insertData.find("id") == insertData.end()) {
        insertData["id"] = String(getNextId(tableName));
    }
    
    // Build row values in column order
    std::vector<String> values;
    for (const String& col : columns) {
        auto it = insertData.find(col);
        if (it != insertData.end()) {
            values.push_back(it->second);
        } else {
            values.push_back(""); // Empty value for missing columns
        }
    }
    
    // Append to file
    File file = SPIFFS.open(getTablePath(tableName), "a");
    if (!file) {
        return false;
    }
    
    String line = buildCsvLine(values);
    file.println(line);
    file.close();
    
    return true;
}

bool CsvDatabase::update(const String& tableName, const String& id, 
    const std::map<String, String>& data) {
    
    if (!tableExists(tableName)) {
        return false;
    }
    
    // Read all records
    auto allRecords = select(tableName);
    
    // Find and update the record
    bool found = false;
    for (auto& record : allRecords) {
        if (record["id"] == id) {
            // Update the record
            for (const auto& pair : data) {
                record[pair.first] = pair.second;
            }
            found = true;
            break;
        }
    }
    
    if (!found) {
        return false;
    }
    
    // Rewrite the entire file
    std::vector<String> columns = getTableColumns(tableName);
    String content = buildCsvLine(columns) + "\n";
    
    for (const auto& record : allRecords) {
        std::vector<String> values;
        for (const String& col : columns) {
            auto it = record.find(col);
            if (it != record.end()) {
                values.push_back(it->second);
            } else {
                values.push_back("");
            }
        }
        content += buildCsvLine(values) + "\n";
    }
    
    return writeToFile(getTablePath(tableName), content);
}

bool CsvDatabase::delete_(const String& tableName, const String& id) {
    if (!tableExists(tableName)) {
        return false;
    }
    
    // Read all records except the one to delete
    auto allRecords = select(tableName);
    
    // Filter out the record to delete
    std::vector<std::map<String, String>> filteredRecords;
    bool found = false;
    
    for (const auto& record : allRecords) {
        if (record.at("id") != id) {
            filteredRecords.push_back(record);
        } else {
            found = true;
        }
    }
    
    if (!found) {
        return false;
    }
    
    // Rewrite the file without the deleted record
    std::vector<String> columns = getTableColumns(tableName);
    String content = buildCsvLine(columns) + "\n";
    
    for (const auto& record : filteredRecords) {
        std::vector<String> values;
        for (const String& col : columns) {
            auto it = record.find(col);
            if (it != record.end()) {
                values.push_back(it->second);
            } else {
                values.push_back("");
            }
        }
        content += buildCsvLine(values) + "\n";
    }
    
    return writeToFile(getTablePath(tableName), content);
}

int CsvDatabase::getNextId(const String& tableName) const {
    auto records = select(tableName);
    int maxId = 0;
    
    for (const auto& record : records) {
        auto it = record.find("id");
        if (it != record.end()) {
            int id = it->second.toInt();
            if (id > maxId) {
                maxId = id;
            }
        }
    }
    
    return maxId + 1;
}

int CsvDatabase::count(const String& tableName, const std::map<String, String>& where) const {
    return select(tableName, where).size();
}

std::vector<String> CsvDatabase::getTables() const {
    std::vector<String> tables;
    
    File root = SPIFFS.open(basePath);
    if (!root || !root.isDirectory()) {
        return tables;
    }
    
    File file = root.openNextFile();
    while (file) {
        String fileName = file.name();
        if (fileName.endsWith(".csv") && !fileName.endsWith(".backup.csv")) {
            // Remove path and extension
            fileName.replace(basePath, "");
            fileName.replace(".csv", "");
            tables.push_back(fileName);
        }
        file = root.openNextFile();
    }
    
    return tables;
}

bool CsvDatabase::backup(const String& tableName) const {
    if (!tableExists(tableName)) {
        return false;
    }
    
    String content = readFromFile(getTablePath(tableName));
    return writeToFile(getBackupPath(tableName), content);
}

bool CsvDatabase::restore(const String& tableName) const {
    String backupPath = getBackupPath(tableName);
    if (!SPIFFS.exists(backupPath)) {
        return false;
    }
    
    String content = readFromFile(backupPath);
    return writeToFile(getTablePath(tableName), content);
}

bool CsvDatabase::writeToFile(const String& filePath, const String& content) const {
    File file = SPIFFS.open(filePath, "w");
    if (!file) {
        return false;
    }
    
    file.print(content);
    file.close();
    return true;
}

String CsvDatabase::readFromFile(const String& filePath) const {
    File file = SPIFFS.open(filePath, "r");
    if (!file) {
        return "";
    }
    
    String content = file.readString();
    file.close();
    return content;
}

std::vector<String> CsvDatabase::readLines(const String& filePath) const {
    std::vector<String> lines;
    
    File file = SPIFFS.open(filePath, "r");
    if (!file) {
        return lines;
    }
    
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            lines.push_back(line);
        }
    }
    
    file.close();
    return lines;
}

bool CsvDatabase::matchesWhere(const std::map<String, String>& record, 
    const std::map<String, String>& where) const {
    
    if (where.empty()) {
        return true; // No conditions, match all
    }
    
    for (const auto& condition : where) {
        auto it = record.find(condition.first);
        if (it == record.end() || it->second != condition.second) {
            return false;
        }
    }
    
    return true;
}
