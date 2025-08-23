#include <Arduino.h>
#include <vector>
#include "setup/setup.h"
#include "tasks/register.h"

I2SSpeaker *i2sSpeaker;
AudioSamples *audioSamples;

void setupSpeakers() {
  logger->info("Setting up speakers...");
  
  #if SPEAKER_ENABLED
  logger->info("Initializing I2S speaker (MAX98357)...");
  i2sSpeaker = new I2SSpeaker(
    I2S_SPEAKER_DATA_PIN,
    I2S_SPEAKER_BCLK_PIN, 
    I2S_SPEAKER_WCLK_PIN
  );
  
  if (i2sSpeaker->init(I2S_SPEAKER_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO) == ESP_OK) {
    logger->info("I2S speaker (MAX98357) initialized successfully");
    
    // Now initialize dependent components AFTER i2sSpeaker is created
    audioSamples = new AudioSamples(i2sSpeaker);
    if (!MP3Player::init(i2sSpeaker)) {
      logger->error("MP3Player initialization failed");
    } else {
      logger->info("MP3Player initialized successfully");
    }
    
    // Test with a simple beep first
    logger->info("Testing I2S speaker with beep...");
    if (audioSamples) {
      audioSamples->playSample(AudioSamples::BEEP_SHORT);
    }
  } else {
    logger->error("I2S speaker (MAX98357) initialization failed");
    delete i2sSpeaker;
    i2sSpeaker = nullptr;
    audioSamples = nullptr;
  }
  
  #else
  logger->info("Speakers disabled in configuration");
  pwmSpeaker = nullptr;
  i2sSpeaker = nullptr;
  audioSamples = nullptr;
  #endif
}

void playSpeakerTone(int frequency, int duration, int volume) {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->playTone(frequency, duration, volume);
    return;
  }
  #endif
}

void playSpeakerBeep(int volume) {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    audioSamples->playSample(AudioSamples::BEEP_SHORT);
    return;
  }
  #endif
}

void playSpeakerConfirmation(int volume) {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    audioSamples->playSample(AudioSamples::CONFIRMATION);
    return;
  }
  #endif
}

void playSpeakerError(int volume) {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    audioSamples->playSample(AudioSamples::ERROR);
    return;
  }
  #endif
}

void playSpeakerStartup(int volume) {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    audioSamples->playSample(AudioSamples::POWER_ON);
    return;
  }
  #endif
}

void playSpeakerNotification(int volume) {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    audioSamples->playSample(AudioSamples::NOTIFICATION);
    return;
  }
  #endif
}

void stopSpeaker() {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->stop();
  }
  #endif
}

void setSpeakerVolume(int volume) {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    MP3Player::setVolume(volume / 100.0f);
  }
  #endif
}

// Get current speaker volume
int getSpeakerVolume() {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    return (int)(MP3Player::getVolume() * 100);
  }
  #endif
  
  return 0;
}

bool isSpeakerPlaying() {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    return i2sSpeaker->isPlaying();
  }
  #endif
  
  return false;
}

// Integration with automation system - play sounds based on behaviors
void playBehaviorSound(const String& behavior) {
  #if SPEAKER_ENABLED
  String lowerBehavior = behavior;
  lowerBehavior.toLowerCase();
  
  if (lowerBehavior.indexOf("happy") >= 0 || lowerBehavior.indexOf("joy") >= 0) {
    playSpeakerConfirmation();
  } else if (lowerBehavior.indexOf("sad") >= 0 || lowerBehavior.indexOf("disappointed") >= 0) {
    playSpeakerError();
  } else if (lowerBehavior.indexOf("surprised") >= 0 || lowerBehavior.indexOf("startled") >= 0) {
    playSpeakerBeep();
  } else if (lowerBehavior.indexOf("notification") >= 0 || lowerBehavior.indexOf("alert") >= 0) {
    playSpeakerNotification();
  } else if (lowerBehavior.indexOf("beep") >= 0) {
    playSpeakerBeep();
  }
  #endif
}

// Get speaker status for WebSocket
bool getSpeakerStatus() {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    return true;
  }
  #endif
  
  return false;
}

// Get speaker type for WebSocket
String getSpeakerType() {
  #if SPEAKER_ENABLED
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    return "I2S_MAX98357";
  }
  #endif
  
  return "None";
}

// Play audio file from SPIFFS
bool playSpeakerAudioFile(const String& filePath, int volume) {
  #if SPEAKER_ENABLED
  if (!fileManager) {
    logger->error("FileManager not available for audio file playback");
    return false;
  }
  
  if (!fileManager->exists(filePath)) {
    logger->error("Audio file not found: " + filePath);
    return false;
  }
  
  // Read the file data
  String fileData = fileManager->readFile(filePath);
  if (fileData.length() == 0) {
    logger->error("Failed to read audio file: " + filePath);
    return false;
  }
  
  // Convert string data to byte array
  const uint8_t* audioData = (const uint8_t*)fileData.c_str();
  size_t dataSize = fileData.length();
  
  logger->info("Playing audio file: " + filePath + " (" + String(dataSize) + " bytes)");
  
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    MP3Decoder decoder;
    MP3Decoder::MP3Info *info;
    decoder.getFileInfo(filePath, info);

    // For I2S speaker, play as raw audio data
    // if (info) 
    i2sSpeaker->start();
    size_t bytesWritten;
    i2sSpeaker->writeAudioData(audioData, dataSize, &bytesWritten);
    i2sSpeaker->stop();
    return true;
  }
  
  logger->error("No speaker available for audio file playback");
  return false;
  
  #else
  logger->warning("Speakers disabled - cannot play audio file");
  return false;
  #endif
}

// Play audio data from memory
void playSpeakerAudioData(const uint8_t* data, size_t dataSize, uint32_t sampleRate, int volume) {
  #if SPEAKER_ENABLED
  if (!data || dataSize == 0) {
    logger->error("Invalid audio data");
    return;
  }
  
  logger->info("Playing audio data (" + String(dataSize) + " bytes, " + String(sampleRate) + "Hz)");
  
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->start();
    i2sSpeaker->writeSamples((int16_t*)data, dataSize);
    i2sSpeaker->stop();
    return;
  }
  
  logger->error("No speaker available for audio data playback");
  #endif
}

// Create a simple WAV-like format for storing audio data in SPIFFS
bool createAudioFile(const String& filePath, const int16_t* samples, size_t sampleCount, uint32_t sampleRate) {
  #if SPEAKER_ENABLED
  if (!fileManager) {
    logger->error("FileManager not available");
    return false;
  }
  
  // Create a simple header + data format
  String audioData = "";
  
  // Simple header (16 bytes)
  audioData += "CZMO"; // Magic number
  audioData += String((char)(sampleRate & 0xFF));
  audioData += String((char)((sampleRate >> 8) & 0xFF));
  audioData += String((char)((sampleRate >> 16) & 0xFF));
  audioData += String((char)((sampleRate >> 24) & 0xFF));
  audioData += String((char)(sampleCount & 0xFF));
  audioData += String((char)((sampleCount >> 8) & 0xFF));
  audioData += String((char)((sampleCount >> 16) & 0xFF));
  audioData += String((char)((sampleCount >> 24) & 0xFF));
  audioData += String((char)16); // Bits per sample
  audioData += String((char)1);  // Channels
  audioData += String((char)0);  // Reserved
  audioData += String((char)0);  // Reserved
  
  // Add sample data
  for (size_t i = 0; i < sampleCount; i++) {
    audioData += String((char)(samples[i] & 0xFF));
    audioData += String((char)((samples[i] >> 8) & 0xFF));
  }
  
  bool result = fileManager->writeFile(filePath, audioData);
  if (result) {
    logger->info("Audio file created: " + filePath + " (" + String(audioData.length()) + " bytes)");
  } else {
    logger->error("Failed to create audio file: " + filePath);
  }
  
  return result;
  
  #else
  logger->warning("Speakers disabled - cannot create audio file");
  return false;
  #endif
}

// Play MP3 file from SPIFFS
bool playSpeakerMP3File(const String& filePath, int volume) {
  #if SPEAKER_ENABLED
  logger->info("Playing MP3 file: " + filePath + " at volume " + String(volume));

  if (!fileManager->exists(filePath)) {
    logger->error("%s is not exists on spiffs", filePath);
    return false;
  }
  
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    // return i2sSpeaker->playMP3File(filePath, volume);
    
    return MP3Player::playFile(filePath, volume / 100.0f);
  }
  
  logger->error("No speaker available for MP3 playback");
  return false;
  
  #else
  logger->warning("Speakers disabled - cannot play MP3 file");
  return false;
  #endif
}

// Get MP3 file information
bool getMP3FileInfo(const String& filePath, int* sampleRate, int* channels, int* bitRate, int* duration) {
  #if SPEAKER_ENABLED
  MP3Decoder decoder;
  if (!decoder.init()) {
    logger->error("Failed to initialize MP3 decoder");
    return false;
  }
  
  MP3Decoder::MP3Info info;
  if (!decoder.getFileInfo(filePath, &info)) {
    logger->error("Failed to get MP3 file info: " + filePath);
    return false;
  }
  
  if (sampleRate) *sampleRate = info.sampleRate;
  if (channels) *channels = info.channels;
  if (bitRate) *bitRate = info.bitRate;
  if (duration) *duration = info.duration;
  
  logger->info("MP3 Info - " + filePath + ": " + String(info.sampleRate) + "Hz, " + 
               String(info.channels) + " channels, " + String(info.bitRate) + " kbps, " + 
               String(info.duration) + "s");
  
  return true;
  
  #else
  logger->warning("Speakers disabled - cannot get MP3 file info");
  return false;
  #endif
}

// Convert MP3 to custom audio format
bool convertMP3ToAudioFile(const String& mp3FilePath, const String& audioFilePath) {
  #if SPEAKER_ENABLED
  MP3Decoder decoder;
  if (!decoder.init()) {
    logger->error("Failed to initialize MP3 decoder");
    return false;
  }
  
  // Decode MP3 to PCM
  int16_t* pcmBuffer = nullptr;
  size_t pcmSize = 0;
  MP3Decoder::MP3Info info;
  
  if (!decoder.decodeFile(mp3FilePath, &pcmBuffer, &pcmSize, &info)) {
    logger->error("Failed to decode MP3 file: " + mp3FilePath);
    return false;
  }
  
  // Create audio file using custom format
  bool result = createAudioFile(audioFilePath, pcmBuffer, pcmSize, info.sampleRate);
  
  // Clean up
  decoder.freePCMBuffer(pcmBuffer);
  
  if (result) {
    logger->info("Converted MP3 to audio file: " + mp3FilePath + " -> " + audioFilePath);
  } else {
    logger->error("Failed to convert MP3 to audio file");
  }
  
  return result;
  
  #else
  logger->warning("Speakers disabled - cannot convert MP3 file");
  return false;
  #endif
}

// Play random MP3 file from /audio/ directory (excluding boot.mp3)
bool playSpeakerRandomMP3(int volume, Utils::FileManager::StorageType storageType) {
  #if SPEAKER_ENABLED
  logger->info("Playing random MP3 file from /audio/ directory at volume " + String(volume));

  if (!fileManager) {
    logger->error("FileManager not available for random MP3 playback");
    return false;
  }
  
  // Get list of files in /audio/ directory
  std::vector<Utils::FileManager::FileInfo> audioFiles = fileManager->listFiles("/audio", storageType);
  std::vector<String> mp3Files;
  
  // Filter for MP3 files (excluding boot.mp3)
  for (const auto& file : audioFiles) {
    if (!file.isDirectory) {
      String fileName = file.name;
      fileName.toLowerCase();
      
      // Check if it's an MP3 file and not boot.mp3
      if (fileName.endsWith(".mp3") && fileName != "boot.mp3") {
        // Store full path
        String fullPath = "/audio/" + file.name;
        mp3Files.push_back(fullPath);
        logger->debug("Found MP3 file: " + fullPath);
      }
    }
  }
  
  // Check if we found any MP3 files
  if (mp3Files.empty()) {
    logger->warning("No MP3 files found in /audio/ directory (excluding boot.mp3)");
    return false;
  }
  
  // Select random MP3 file
  randomSeed(millis()); // Seed random number generator
  int randomIndex = random(0, mp3Files.size());
  String selectedFile = mp3Files[randomIndex];
  
  logger->info("Selected random MP3: " + selectedFile + " (" + String(randomIndex + 1) + "/" + String(mp3Files.size()) + ")");
  
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    // return i2sSpeaker->playMP3File(selectedFile, volume);
    return MP3Player::playFile(selectedFile, volume / 100.0f);
  }
  
  logger->error("No speaker available for random MP3 playback");
  return false;
  
  #else
  logger->warning("Speakers disabled - cannot play random MP3 file");
  return false;
  #endif
}

// Play random MP3 file from /audio/ directory with default storage (SPIFFS)
bool playSpeakerRandomMP3(int volume) {
  return playSpeakerRandomMP3(volume, Utils::FileManager::STORAGE_SPIFFS);
}

// Get list of available MP3 files in /audio/ directory (excluding boot.mp3)
std::vector<String> getAvailableMP3Files(Utils::FileManager::StorageType storageType) {
  std::vector<String> mp3Files;
  
  #if SPEAKER_ENABLED
  if (!fileManager) {
    logger->error("FileManager not available");
    return mp3Files;
  }
  
  // Get list of files in /audio/ directory
  std::vector<Utils::FileManager::FileInfo> audioFiles = fileManager->listFiles("/audio", storageType);
  
  // Filter for MP3 files (excluding boot.mp3)
  for (const auto& file : audioFiles) {
    if (!file.isDirectory) {
      String fileName = file.name;
      fileName.toLowerCase();
      
      // Check if it's an MP3 file and not boot.mp3
      if (fileName.endsWith(".mp3") && fileName != "boot.mp3") {
        String fullPath = "/audio/" + file.name;
        mp3Files.push_back(fullPath);
      }
    }
  }
  
  logger->info("Found " + String(mp3Files.size()) + " MP3 files in /audio/ directory (excluding boot.mp3)");
  #endif
  
  return mp3Files;
}

// Get list of available MP3 files with default storage (SPIFFS)
std::vector<String> getAvailableMP3Files() {
  return getAvailableMP3Files(Utils::FileManager::STORAGE_SPIFFS);
}
