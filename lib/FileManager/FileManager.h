#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>
#ifdef CONFIG_IDF_TARGET_ESP32S3
#include <SD_MMC.h>
#endif
#include <vector>
#include <algorithm>

namespace Utils {

class FileManager {
public:
    enum StorageType {
        STORAGE_SPIFFS,
        STORAGE_LITTLEFS,
        STORAGE_SD_MMC
    };

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
     * @param enableSD Enable SPI SD card support (default: true)
     * @param sdPin SD card chip select pin for SPI mode (default: 5)
     * @param enableSDMMC Enable SD_MMC (SDIO) support for ESP32S3 (default: false)
     * @param use1bitMode Use 1-bit mode for SD_MMC (default: true)
     * @param formatIfMountFailed Format SD card if mount fails (default: false)
     * @param sdmmcFreq SD_MMC frequency in MHz (default: 20)
     * @return true if initialization was successful, false otherwise
     */
    bool init(bool enableSDMMC = true, 
              bool use1bitMode = true, bool formatIfMountFailed = false, uint32_t sdmmcFreq = 20);

    /**
     * Set default storage type for operations
     * @param storageType The storage type to use by default
     */
    void setDefaultStorage(StorageType storageType);

    /**
     * Check if SD_MMC is available
     * @return true if SD_MMC is available, false otherwise
     */
    bool isSDMMCAvailable() const;

    /**
     * Read a file
     * @param path The file path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return The file contents as a string
     */
    String readFile(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Write to a file
     * @param path The file path
     * @param content The content to write
     * @param storageType Storage type (optional, uses default if not specified)
     * @return true if successful, false otherwise
     */
    bool writeFile(const String& path, const String& content, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Append to a file
     * @param path The file path
     * @param content The content to append
     * @param storageType Storage type (optional, uses default if not specified)
     * @return true if successful, false otherwise
     */
    bool appendFile(const String& path, const String& content, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Delete a file
     * @param path The file path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return true if successful, false otherwise
     */
    bool deleteFile(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Check if a file exists
     * @param path The file path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return true if the file exists, false otherwise
     */
    bool exists(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Get file size
     * @param path The file path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return The file size in bytes, or -1 if the file doesn't exist
     */
    int getSize(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * List files in a directory
     * @param path The directory path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return Vector of FileInfo structures
     */
    std::vector<FileInfo> listFiles(String path = "/", StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Create a directory
     * @param path The directory path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return true if successful, false otherwise
     */
    bool createDir(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Remove a directory
     * @param path The directory path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return true if successful, false otherwise
     */
    bool removeDir(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    // Stream reading methods
    /**
     * Open file for streaming read operations
     * @param path The file path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return File handle for streaming operations
     */
    File openFileForReading(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Open file for binary write operations
     * @param path The file path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return File handle for binary writing operations
     */
    File openFileForWriting(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Open file for read-write operations (allows updating existing files)
     * @param path The file path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return File handle for read-write operations
     */
    File openFileForReadWrite(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Open file for append operations
     * @param path The file path
     * @param storageType Storage type (optional, uses default if not specified)
     * @return File handle for append operations
     */
    File openFileForAppend(const String& path, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Write binary data to file
     * @param file Open file handle
     * @param buffer Buffer containing binary data
     * @param size Number of bytes to write
     * @return Number of bytes actually written
     */
    size_t writeBinary(File& file, const uint8_t* buffer, size_t size);

    /**
     * Read a chunk of data from file stream
     * @param file Open file handle
     * @param buffer Buffer to store read data
     * @param size Number of bytes to read
     * @return Number of bytes actually read
     */
    size_t readStream(File& file, uint8_t* buffer, size_t size);

    /**
     * Read a specific range from file (start to end)
     * @param path The file path
     * @param start Starting byte position
     * @param end Ending byte position (exclusive)
     * @param buffer Buffer to store read data
     * @param storageType Storage type (optional, uses default if not specified)
     * @return Number of bytes actually read
     */
    size_t readStream(const String& path, size_t start, size_t end, uint8_t* buffer, StorageType storageType = STORAGE_LITTLEFS);

    /**
     * Seek to position in file stream
     * @param file Open file handle
     * @param position Position to seek to
     * @return true if successful
     */
    bool seekFile(File& file, size_t position);

    /**
     * Close file handle
     * @param file File handle to close
     */
    void closeFile(File& file);

private:
    bool _initialized;
    bool _sdInitialized;
    bool _sdmmcInitialized;
    StorageType _defaultStorage;
    
    /**
     * Get the appropriate file system based on storage type
     * @param storageType The storage type
     * @return Reference to the file system
     */
    fs::FS& getFileSystem(StorageType storageType);
};

} // namespace Utils
