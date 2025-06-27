/**
 * MP3 Audio Test - Cozmo System
 * 
 * This file demonstrates MP3 playback capabilities including:
 * - Playing MP3 files directly from SPIFFS
 * - Getting MP3 file information
 * - Converting MP3 to custom audio format
 * - Using voice commands for MP3 playback
 */

#include <Arduino.h>
#include "app.h"

// Test MP3 playback functionality
void testMP3Playback() {
    logger->info("=== MP3 Playback Test ===");
    
    // Test 1: Play MP3 file directly
    logger->info("Test 1: Playing MP3 file directly");
    if (playSpeakerMP3File("/sounds/test.mp3", 60)) {
        logger->info("✓ MP3 playback successful");
        delay(5000); // Wait for playback
    } else {
        logger->error("✗ MP3 playback failed");
    }
    
    // Test 2: Get MP3 file information
    logger->info("Test 2: Getting MP3 file information");
    int sampleRate, channels, bitRate, duration;
    if (getMP3FileInfo("/sounds/test.mp3", &sampleRate, &channels, &bitRate, &duration)) {
        logger->info("✓ MP3 Info Retrieved:");
        logger->info("  Sample Rate: " + String(sampleRate) + " Hz");
        logger->info("  Channels: " + String(channels));
        logger->info("  Bit Rate: " + String(bitRate) + " kbps");
        logger->info("  Duration: " + String(duration) + " seconds");
    } else {
        logger->error("✗ Failed to get MP3 information");
    }
    
    // Test 3: Convert MP3 to custom format
    logger->info("Test 3: Converting MP3 to custom audio format");
    if (convertMP3ToAudioFile("/sounds/test.mp3", "/sounds/test_converted.czmo")) {
        logger->info("✓ MP3 conversion successful");
        
        // Play the converted file
        if (playSpeakerAudioFile("/sounds/test_converted.czmo", 60)) {
            logger->info("✓ Converted file playback successful");
            delay(5000);
        } else {
            logger->error("✗ Converted file playback failed");
        }
    } else {
        logger->error("✗ MP3 conversion failed");
    }
    
    logger->info("=== MP3 Test Complete ===");
}

// Test MP3 voice commands
void testMP3Commands() {
    logger->info("=== MP3 Command Test ===");
    
    // Test commands
    String commands[] = {
        "[PLAY_MP3_FILE=/sounds/test.mp3,50]",
        "[MP3_INFO=/sounds/test.mp3]",
        "[CONVERT_MP3=/sounds/test.mp3,/sounds/converted.czmo]",
        "[PLAY_AUDIO_FILE=/sounds/converted.czmo,60]",
        "[STOP_AUDIO]"
    };
    
    for (int i = 0; i < 5; i++) {
        logger->info("Executing: " + commands[i]);
        bool success = executeCommand(commands[i]);
        logger->info(success ? "✓ Command successful" : "✗ Command failed");
        delay(2000); // Wait between commands
    }
    
    logger->info("=== MP3 Command Test Complete ===");
}

// Create test MP3 files (simulated - you'd need real MP3 files)
void setupMP3TestFiles() {
    logger->info("=== Setting up MP3 Test Files ===");
    
    // Note: For real testing, you would need to upload actual MP3 files
    // This is just to demonstrate the expected file structure
    
    if (!fileManager->exists("/sounds")) {
        fileManager->createDir("/sounds");
        logger->info("Created /sounds directory");
    }
    
    // List available audio files
    auto files = fileManager->listFiles("/sounds");
    logger->info("Available audio files in /sounds:");
    for (const auto& file : files) {
        logger->info("  " + file.name + " (" + String(file.size) + " bytes)");
    }
    
    logger->info("=== MP3 Test Setup Complete ===");
    logger->info("Note: Upload real MP3 files to /sounds/ for testing");
}

// Main test function - call this from setup() or loop()
void runMP3Tests() {
    logger->info("🎵 Starting MP3 Audio System Tests 🎵");
    
    setupMP3TestFiles();
    delay(1000);
    
    testMP3Playback();
    delay(2000);
    
    testMP3Commands();
    
    logger->info("🎵 All MP3 Tests Complete! 🎵");
}

// Example usage in main app
void demonstrateMP3Usage() {
    // Simple MP3 playback
    playSpeakerMP3File("/sounds/welcome.mp3", 70);
    
    // Get information about an MP3 file
    int sampleRate, channels;
    if (getMP3FileInfo("/sounds/music.mp3", &sampleRate, &channels)) {
        logger->info("Music file: " + String(sampleRate) + "Hz, " + String(channels) + " channels");
    }
    
    // Convert MP3 for faster playback (pre-decode)
    convertMP3ToAudioFile("/sounds/long_music.mp3", "/sounds/long_music.czmo");
    
    // Use voice commands
    executeCommand("[PLAY_MP3_FILE=/sounds/notification.mp3,60]");
    executeCommand("[MP3_INFO=/sounds/voice_message.mp3]");
}
