/*
 * Random MP3 Playback Example
 * 
 * This example demonstrates how to use the new random MP3 playback functionality
 * in the Cozmo system. It shows how to play random MP3 files from the /audio/
 * directory while excluding boot.mp3.
 */

#include "app.h"

void setupRandomMP3Example() {
    Serial.begin(115200);
    
    // Initialize the file manager and speakers
    // (This would typically be done in your main setup function)
    
    Serial.println("=== Random MP3 Playback Example ===");
    
    // Get list of available MP3 files
    std::vector<String> mp3Files = getAvailableMP3Files();
    
    Serial.println("Available MP3 files in /audio/ directory:");
    for (size_t i = 0; i < mp3Files.size(); i++) {
        Serial.println("  " + String(i + 1) + ". " + mp3Files[i]);
    }
    
    if (mp3Files.empty()) {
        Serial.println("No MP3 files found in /audio/ directory (excluding boot.mp3)");
        Serial.println("Please add some MP3 files to the /audio/ directory on SPIFFS");
        return;
    }
    
    Serial.println("\nTesting random MP3 playback...");
    
    // Example 1: Play random MP3 with default volume
    Serial.println("Playing random MP3 with default volume...");
    if (playSpeakerRandomMP3()) {
        Serial.println("✓ Random MP3 playback started successfully");
    } else {
        Serial.println("✗ Failed to start random MP3 playback");
    }
    
    // Wait for playback to finish
    delay(5000);
    
    // Example 2: Play random MP3 with specific volume
    Serial.println("\nPlaying random MP3 with volume 75...");
    if (playSpeakerRandomMP3(75)) {
        Serial.println("✓ Random MP3 playback started successfully");
    } else {
        Serial.println("✗ Failed to start random MP3 playback");
    }
    
    // Wait for playback to finish
    delay(5000);
    
    // Example 3: Play random MP3 from SD card (if available)
    if (fileManager && fileManager->isSDAvailable()) {
        Serial.println("\nPlaying random MP3 from SD card with volume 50...");
        if (playSpeakerRandomMP3(50, Utils::FileManager::STORAGE_SD)) {
            Serial.println("✓ Random MP3 playback from SD card started successfully");
        } else {
            Serial.println("✗ Failed to start random MP3 playback from SD card");
        }
    } else {
        Serial.println("\nSD card not available, skipping SD card example");
    }
}

void loopRandomMP3Example() {
    // Example of playing random MP3 files at intervals
    static unsigned long lastPlayTime = 0;
    static const unsigned long playInterval = 30000; // Play every 30 seconds
    
    if (millis() - lastPlayTime >= playInterval) {
        if (!isSpeakerPlaying()) {
            Serial.println("Playing random ambient sound...");
            playSpeakerRandomMP3(30); // Play at low volume for ambient sound
            lastPlayTime = millis();
        }
    }
}

/*
 * Integration examples with the Cozmo system:
 */

// Example: Play random sound on behavior trigger
void onBehaviorTrigger(const String& behavior) {
    if (behavior == "happy" || behavior == "excited") {
        Serial.println("Playing random happy sound...");
        playSpeakerRandomMP3(80); // Play at higher volume for positive emotions
    } else if (behavior == "curious" || behavior == "exploring") {
        Serial.println("Playing random ambient sound...");
        playSpeakerRandomMP3(40); // Play at medium volume for neutral emotions
    }
}

// Example: Play random sound when user interacts
void onUserInteraction() {
    if (!isSpeakerPlaying()) {
        Serial.println("User interaction detected, playing random sound...");
        playSpeakerRandomMP3(60);
    }
}

// Example: Play random sound at specific times
void onScheduledEvent() {
    static int lastHour = -1;
    int currentHour = hour(); // Assuming you have time functions available
    
    if (currentHour != lastHour && currentHour % 2 == 0) { // Every 2 hours
        if (!isSpeakerPlaying()) {
            Serial.println("Scheduled random sound playback...");
            playSpeakerRandomMP3(45);
            lastHour = currentHour;
        }
    }
}

/*
 * Utility function to manage MP3 files
 */
void listAllAudioFiles() {
    Serial.println("\n=== Audio Files Management ===");
    
    // Check SPIFFS
    std::vector<String> spiffsFiles = getAvailableMP3Files(Utils::FileManager::STORAGE_SPIFFS);
    Serial.println("SPIFFS MP3 files (" + String(spiffsFiles.size()) + "):");
    for (const String& file : spiffsFiles) {
        Serial.println("  " + file);
    }
    
    // Check SD card if available
    if (fileManager && fileManager->isSDAvailable()) {
        std::vector<String> sdFiles = getAvailableMP3Files(Utils::FileManager::STORAGE_SD);
        Serial.println("\nSD Card MP3 files (" + String(sdFiles.size()) + "):");
        for (const String& file : sdFiles) {
            Serial.println("  " + file);
        }
    }
    
    // Check SD_MMC if available
    if (fileManager && fileManager->isSDMMCAvailable()) {
        std::vector<String> sdmmcFiles = getAvailableMP3Files(Utils::FileManager::STORAGE_SD_MMC);
        Serial.println("\nSD_MMC MP3 files (" + String(sdmmcFiles.size()) + "):");
        for (const String& file : sdmmcFiles) {
            Serial.println("  " + file);
        }
    }
}
