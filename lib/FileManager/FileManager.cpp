#include "FileManager.h"

namespace Utils {

FileManager::FileManager() : _initialized(false), _sdInitialized(false), _sdmmcInitialized(false), _defaultStorage(STORAGE_LITTLEFS) {
}

FileManager::~FileManager() {
    // Clean up resources if needed
}

bool FileManager::init(bool enableSDMMC, bool use1bitMode, bool formatIfMountFailed, uint32_t sdmmcFreq) {
    // Initialize LittleFS
    if (!_initialized && !LittleFS.begin(false)) {
        Serial.println("LittleFS mount failed");
        return false;
    }
    _initialized = true;

    Serial.println("FileManager init successful in init()");
    Serial.println("Files in LittleFS root directory:");
    
    std::vector<Utils::FileManager::FileInfo> files = listFiles();
    for (const auto& fileInfo : files) {
        Serial.print("  ");
        Serial.print(fileInfo.name);
        if (fileInfo.isDirectory) {
            Serial.println("/");
        } else {
            Serial.print(" (");
            Serial.print(fileInfo.size);
            Serial.println(" bytes)");
        }
    }
    
    // Initialize SD_MMC if requested and on ESP32S3
    if (enableSDMMC) {
#ifdef CONFIG_IDF_TARGET_ESP32S3
        // Check if SD_MMC pins are defined
#if defined(SD_MMC_CLK) && defined(SD_MMC_CMD) && defined(SD_MMC_D0)
        // Set up pins based on available definitions
#if defined(SD_MMC_D1) && defined(SD_MMC_D2) && defined(SD_MMC_D3)
        // 4-bit mode available
        if (!use1bitMode) {
            SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0, SD_MMC_D1, SD_MMC_D2, SD_MMC_D3);
            Serial.println("SD_MMC configured for 4-bit mode");
        } else {
            SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
            Serial.println("SD_MMC configured for 1-bit mode");
        }
#else
        // Only 1-bit mode available
        SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
        use1bitMode = true;
        Serial.println("SD_MMC configured for 1-bit mode (only mode available)");
#endif
        
        // Initialize SD_MMC
        if (SD_MMC.begin("/sdcard", use1bitMode, formatIfMountFailed, sdmmcFreq)) {
            _sdmmcInitialized = true;
            Serial.println("SD_MMC initialized successfully");
            Serial.println("Files in MMC root directory:");
            std::vector<Utils::FileManager::FileInfo> files = listFiles("/", STORAGE_SD_MMC);
            for (const auto& fileInfo : files) {
                Serial.print("  ");
                Serial.print(fileInfo.name);
                if (fileInfo.isDirectory) {
                    Serial.println("/");
                } else {
                    Serial.print(" (");
                    Serial.print(fileInfo.size);
                    Serial.println(" bytes)");
                }
            }
        } else {
            Serial.println("SD_MMC initialization failed");
        }
#else
        Serial.println("SD_MMC pins not defined for this board");
#endif
#else
        Serial.println("SD_MMC only supported on ESP32S3");
#endif
    }
    
    return true;
}

bool FileManager::isSDMMCAvailable() const {
    return _sdmmcInitialized;
}

void FileManager::setDefaultStorage(StorageType storageType) {
    _defaultStorage = storageType;
}

fs::FS& FileManager::getFileSystem(StorageType storageType) {
#ifdef CONFIG_IDF_TARGET_ESP32S3
    if (storageType == STORAGE_SD_MMC && _sdmmcInitialized) {
        return SD_MMC;
    }
#endif
    return LittleFS;
}

String FileManager::readFile(const String& path, StorageType storageType) {
    if (!_initialized) {
        return "";
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        Serial.println("SD_MMC not available, falling back to LittleFS");
        storageType = STORAGE_LITTLEFS;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    File file = fs.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file for reading: " + path);
        return "";
    }
    
    String content = file.readString();
    file.close();
    return content;
}

bool FileManager::writeFile(const String& path, const String& content, StorageType storageType) {
    if (!_initialized) {
        return false;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        Serial.println("SD_MMC not available, falling back to LittleFS");
        storageType = STORAGE_LITTLEFS;
    }

    fs::FS& fs = getFileSystem(storageType);
    
    if (exists(path, storageType)) {
        deleteFile(path, storageType);
        vTaskDelay(pdMS_TO_TICKS(7));
    }
    
    File file = fs.open(path, "w");
    if (!file) {
        Serial.println("Failed to open file for writing: " + path);
        return false;
    }
    
    size_t written = file.print(content);
    file.close();
    
    return written == content.length();
}

bool FileManager::appendFile(const String& path, const String& content, StorageType storageType) {
    if (!_initialized) {
        return false;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        Serial.println("SD_MMC not available, falling back to LittleFS");
        storageType = STORAGE_LITTLEFS;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    File file = fs.open(path, "a");
    if (!file) {
        return writeFile(path, content, storageType);
    }
    
    size_t written = file.print(content);
    file.close();
    
    return written == content.length();
}

bool FileManager::deleteFile(const String& path, StorageType storageType) {
    if (!_initialized) {
        return false;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        Serial.println("SD_MMC not available, falling back to LittleFS");
        storageType = STORAGE_LITTLEFS;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    
    if (!fs.exists(path)) {
        return false;
    }
    
    return fs.remove(path);
}

bool FileManager::exists(const String& path, StorageType storageType) {
    if (!_initialized) {
        return false;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        return false;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    return fs.exists(path);
}

int FileManager::getSize(const String& path, StorageType storageType) {
    if (!_initialized) {
        return -1;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        return -1;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    File file = fs.open(path, "r");
    if (!file) {
        return -1;
    }
    
    int size = file.size();
    file.close();
    return size;
}

std::vector<FileManager::FileInfo> FileManager::listFiles(String path, StorageType storageType) {
    std::vector<FileInfo> files;
    std::vector<FileInfo> directories;
    
    if (!_initialized) {
        return files;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        return files;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    File root = fs.open(path);
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory: " + path);
        return files;
    }

    String dir = path;
    if (dir != "/" && !dir.endsWith("/")) 
        dir += "/";

    File file = root.openNextFile();
    while (file)
    {
        FileInfo info;
        String fullpath = file.path();
        String tempname = fullpath.substring(path.length());
        int isDir = tempname.indexOf("/");
        if (isDir > 0) {
            tempname = tempname.substring(0, isDir);
            info.name = tempname;
            info.dir = dir;
            info.size = 0;
            info.isDirectory = true;
            bool exists = false;
            for (auto dir: directories) {
                if (dir.name == info.name) {
                    exists = true;
                    break;
                }
            }
            if (!exists)
                directories.push_back(info);
            file = root.openNextFile();
            continue;
        }

        info.name = file.name();
        info.size = file.size();
        info.dir = dir;
        info.isDirectory = file.isDirectory();
        
        files.push_back(info);
        file = root.openNextFile();
    }
    
    root.close();
    
    // Sort directories alphabetically
    std::sort(directories.begin(), directories.end(), 
              [](const FileInfo& a, const FileInfo& b) { 
                  return a.name.compareTo(b.name) < 0; 
              });
              
    // Sort files alphabetically
    std::sort(files.begin(), files.end(), 
              [](const FileInfo& a, const FileInfo& b) { 
                  return a.name.compareTo(b.name) < 0; 
              });
    
    // Combine directories followed by files
    std::vector<FileInfo> result;
    result.reserve(directories.size() + files.size());
    
    // Add directories first
    result.insert(result.end(), directories.begin(), directories.end());
    
    // Then add files
    result.insert(result.end(), files.begin(), files.end());
    
    return result;
}

bool FileManager::createDir(const String& path, StorageType storageType) {
    if (!_initialized) {
        return false;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        Serial.println("SD_MMC not available, falling back to LittleFS");
        storageType = STORAGE_LITTLEFS;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    return fs.mkdir(path);
}

bool FileManager::removeDir(const String& path, StorageType storageType) {
    if (!_initialized) {
        return false;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        Serial.println("SD_MMC not available, falling back to LittleFS");
        storageType = STORAGE_LITTLEFS;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    return fs.rmdir(path);
}

File FileManager::openFileForReading(const String& path, StorageType storageType) {
    if (!_initialized) {
        return File();
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        Serial.println("SD_MMC not available, falling back to LittleFS");
        storageType = STORAGE_LITTLEFS;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    return fs.open(path, "r");
}

size_t FileManager::readStream(File& file, uint8_t* buffer, size_t size) {
    if (!file || !buffer || size == 0) {
        return 0;
    }
    return file.readBytes((char*)buffer, size);
}

size_t FileManager::readStream(const String& path, size_t start, size_t end, uint8_t* buffer, StorageType storageType) {
    if (!_initialized || !buffer || start >= end) {
        return 0;
    }
    
    // Check if SD_MMC is requested but not available
    if (storageType == STORAGE_SD_MMC && !_sdmmcInitialized) {
        Serial.println("SD_MMC not available, falling back to LittleFS");
        storageType = STORAGE_LITTLEFS;
    }
    
    fs::FS& fs = getFileSystem(storageType);
    File file = fs.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file for streaming: " + path);
        return 0;
    }
    
    // Check file size
    size_t fileSize = file.size();
    if (start >= fileSize) {
        file.close();
        return 0;
    }
    
    // Adjust end position if it exceeds file size
    if (end > fileSize) {
        end = fileSize;
    }
    
    // Seek to start position
    if (!file.seek(start)) {
        file.close();
        return 0;
    }
    
    // Read the requested range
    size_t bytesToRead = end - start;
    size_t bytesRead = file.readBytes((char*)buffer, bytesToRead);
    
    file.close();
    return bytesRead;
}

bool FileManager::seekFile(File& file, size_t position) {
    if (!file) {
        return false;
    }
    return file.seek(position);
}

void FileManager::closeFile(File& file) {
    if (file) {
        file.close();
    }
}

} // namespace Utils
