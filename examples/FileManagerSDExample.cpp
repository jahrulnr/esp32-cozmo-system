#include "lib/Utils/FileManager.h"

Utils::FileManager fileManager;

void setup() {
    Serial.begin(115200);
    
    // Initialize file manager with both SPI SD and SD_MMC support
    // Parameters: enableSD, sdPin, enableSDMMC, use1bitMode, formatIfMountFailed, sdmmcFreq
    if (fileManager.init(true, 5, true, true, false, 20)) {
        Serial.println("FileManager initialized successfully");
        
        // Check what storage options are available
        Serial.println("\nAvailable storage options:");
        Serial.println("- SPIFFS: Always available");
        
        if (fileManager.isSDAvailable()) {
            Serial.println("- SPI SD card: Available");
        } else {
            Serial.println("- SPI SD card: Not available");
        }
        
        if (fileManager.isSDMMCAvailable()) {
            Serial.println("- SD_MMC (SDIO): Available");
        } else {
            Serial.println("- SD_MMC (SDIO): Not available");
        }
        
        // Set default storage (you can choose based on what's available)
        if (fileManager.isSDMMCAvailable()) {
            fileManager.setDefaultStorage(Utils::FileManager::STORAGE_SD_MMC);
            Serial.println("\nDefault storage set to SD_MMC");
        } else if (fileManager.isSDAvailable()) {
            fileManager.setDefaultStorage(Utils::FileManager::STORAGE_SD);
            Serial.println("\nDefault storage set to SPI SD");
        } else {
            Serial.println("\nUsing SPIFFS as default storage");
        }
    } else {
        Serial.println("FileManager initialization failed");
        return;
    }
    
    // Example: Write to different storage types
    Serial.println("\n=== Writing to different storage types ===");
    
    // Write to SPIFFS
    if (fileManager.writeFile("/spiffs_config.txt", "SPIFFS configuration data", Utils::FileManager::STORAGE_SPIFFS)) {
        Serial.println("✓ Successfully wrote to SPIFFS");
    } else {
        Serial.println("✗ Failed to write to SPIFFS");
    }
    
    // Write to SPI SD card (if available)
    if (fileManager.isSDAvailable()) {
        if (fileManager.writeFile("/sd_data.txt", "SPI SD card data", Utils::FileManager::STORAGE_SD)) {
            Serial.println("✓ Successfully wrote to SPI SD card");
        } else {
            Serial.println("✗ Failed to write to SPI SD card");
        }
    }
    
    // Write to SD_MMC (if available)
    if (fileManager.isSDMMCAvailable()) {
        if (fileManager.writeFile("/sdmmc_data.txt", "SD_MMC (SDIO) data", Utils::FileManager::STORAGE_SD_MMC)) {
            Serial.println("✓ Successfully wrote to SD_MMC");
        } else {
            Serial.println("✗ Failed to write to SD_MMC");
        }
    }
    
    // Example: Read from different storage types
    Serial.println("\n=== Reading from different storage types ===");
    
    // Read from SPIFFS
    String spiffsContent = fileManager.readFile("/spiffs_config.txt", Utils::FileManager::STORAGE_SPIFFS);
    if (spiffsContent.length() > 0) {
        Serial.println("SPIFFS content: " + spiffsContent);
    }
    
    // Read from SPI SD card
    if (fileManager.isSDAvailable()) {
        String sdContent = fileManager.readFile("/sd_data.txt", Utils::FileManager::STORAGE_SD);
        if (sdContent.length() > 0) {
            Serial.println("SPI SD content: " + sdContent);
        }
    }
    
    // Read from SD_MMC
    if (fileManager.isSDMMCAvailable()) {
        String sdmmcContent = fileManager.readFile("/sdmmc_data.txt", Utils::FileManager::STORAGE_SD_MMC);
        if (sdmmcContent.length() > 0) {
            Serial.println("SD_MMC content: " + sdmmcContent);
        }
    }
    
    // Example: List files on different storage types
    Serial.println("\n=== Listing files on different storage types ===");
    
    // List SPIFFS files
    Serial.println("\nSPIFFS files:");
    auto spiffsFiles = fileManager.listFiles("/", Utils::FileManager::STORAGE_SPIFFS);
    for (const auto& file : spiffsFiles) {
        Serial.println("  " + file.name + " (" + String(file.size) + " bytes)");
    }
    
    // List SPI SD files
    if (fileManager.isSDAvailable()) {
        Serial.println("\nSPI SD card files:");
        auto sdFiles = fileManager.listFiles("/", Utils::FileManager::STORAGE_SD);
        for (const auto& file : sdFiles) {
            Serial.println("  " + file.name + " (" + String(file.size) + " bytes)");
        }
    }
    
    // List SD_MMC files
    if (fileManager.isSDMMCAvailable()) {
        Serial.println("\nSD_MMC files:");
        auto sdmmcFiles = fileManager.listFiles("/", Utils::FileManager::STORAGE_SD_MMC);
        for (const auto& file : sdmmcFiles) {
            Serial.println("  " + file.name + " (" + String(file.size) + " bytes)");
        }
    }
    
    Serial.println("\n=== FileManager SD card example completed ===");
}

void loop() {
    // Nothing to do in loop for this example
    delay(1000);
}
