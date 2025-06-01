#include "FileManager.h"

namespace Utils {

FileManager::FileManager() : _initialized(false) {
}

FileManager::~FileManager() {
    // Clean up resources if needed
}

bool FileManager::init() {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
        return false;
    }
    
    _initialized = true;
    return true;
}

String FileManager::readFile(const String& path) {
    if (!_initialized) {
        return "";
    }
    
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file for reading: " + path);
        return "";
    }
    
    String content = file.readString();
    file.close();
    return content;
}

bool FileManager::writeFile(const String& path, const String& content) {
    if (!_initialized) {
        return false;
    }
    
    File file = SPIFFS.open(path, "w");
    if (!file) {
        Serial.println("Failed to open file for writing: " + path);
        return false;
    }
    
    size_t written = file.print(content);
    file.close();
    
    return written == content.length();
}

bool FileManager::appendFile(const String& path, const String& content) {
    if (!_initialized) {
        return false;
    }
    
    File file = SPIFFS.open(path, "a");
    if (!file) {
        Serial.println("Failed to open file for appending: " + path);
        return false;
    }
    
    size_t written = file.print(content);
    file.close();
    
    return written == content.length();
}

bool FileManager::deleteFile(const String& path) {
    if (!_initialized) {
        return false;
    }
    
    if (!SPIFFS.exists(path)) {
        return false;
    }
    
    return SPIFFS.remove(path);
}

bool FileManager::exists(const String& path) {
    if (!_initialized) {
        return false;
    }
    
    return SPIFFS.exists(path);
}

int FileManager::getSize(const String& path) {
    if (!_initialized) {
        return -1;
    }
    
    File file = SPIFFS.open(path, "r");
    if (!file) {
        return -1;
    }
    
    int size = file.size();
    file.close();
    return size;
}

std::vector<FileManager::FileInfo> FileManager::listFiles(const String& path) {
    std::vector<FileInfo> files;
    
    if (!_initialized) {
        return files;
    }
    
    File root = SPIFFS.open(path);
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory: " + path);
        return files;
    }
    
    File file = root.openNextFile();
    while (file) {
        FileInfo info;
        info.name = file.name();
        info.size = file.size();
        info.isDirectory = file.isDirectory();
        files.push_back(info);
        
        file = root.openNextFile();
    }
    
    root.close();
    return files;
}

bool FileManager::createDir(const String& path) {
    if (!_initialized) {
        return false;
    }
    
    return SPIFFS.mkdir(path);
}

bool FileManager::removeDir(const String& path) {
    if (!_initialized) {
        return false;
    }
    
    return SPIFFS.rmdir(path);
}

} // namespace Utils
