#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include <vector>
#include <algorithm>
#include "Sstring.h"

namespace Utils {

class FileManager {
public:
    struct FileInfo {
        String name;
        String dir;
        size_t size;
        bool isDirectory;
    };

    FileManager();
    ~FileManager();

    /**
     * Initialize file manager
     * @return true if initialization was successful, false otherwise
     */
    bool init();

    /**
     * Read a file
     * @param path The file path
     * @return The file contents as a string
     */
    String readFile(const String& path);

    /**
     * Write to a file
     * @param path The file path
     * @param content The content to write
     * @return true if successful, false otherwise
     */
    bool writeFile(const String& path, const String& content);

    /**
     * Append to a file
     * @param path The file path
     * @param content The content to append
     * @return true if successful, false otherwise
     */
    bool appendFile(const String& path, const String& content);

    /**
     * Delete a file
     * @param path The file path
     * @return true if successful, false otherwise
     */
    bool deleteFile(const String& path);

    /**
     * Check if a file exists
     * @param path The file path
     * @return true if the file exists, false otherwise
     */
    bool exists(const String& path);

    /**
     * Get file size
     * @param path The file path
     * @return The file size in bytes, or -1 if the file doesn't exist
     */
    int getSize(const String& path);

    /**
     * List files in a directory
     * @param path The directory path
     * @return Vector of FileInfo structures
     */
    std::vector<FileInfo> listFiles(String path = "/");

    /**
     * Create a directory
     * @param path The directory path
     * @return true if successful, false otherwise
     */
    bool createDir(const String& path);

    /**
     * Remove a directory
     * @param path The directory path
     * @return true if successful, false otherwise
     */
    bool removeDir(const String& path);

private:
    bool _initialized;
};

} // namespace Utils
