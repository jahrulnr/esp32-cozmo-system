#include <Arduino.h>
#include "app.h"
#include "lib/Audio/MP3Decoder.h"

void setupSpeakers() {
  logger->info("Setting up speakers...");
  
  #if SPEAKER_ENABLED
  
  #if SPEAKER_TYPE_PWM
  logger->info("Initializing PWM speaker...");
  pwmSpeaker = new Audio::PWMSpeaker(PWM_SPEAKER_PIN, PWM_SPEAKER_CHANNEL);
  if (pwmSpeaker->init()) {
    pwmSpeaker->setVolume(PWM_SPEAKER_DEFAULT_VOLUME);
    logger->info("PWM speaker initialized successfully");
    
    // Play startup sound to confirm speaker is working
    pwmSpeaker->playStartup(PWM_SPEAKER_DEFAULT_VOLUME);
  } else {
    logger->error("PWM speaker initialization failed");
    delete pwmSpeaker;
    pwmSpeaker = nullptr;
  }
  #else
  pwmSpeaker = nullptr;
  #endif
  
  #if SPEAKER_TYPE_I2S
  logger->info("Initializing I2S speaker (MAX98357)...");
  i2sSpeaker = new Audio::I2SSpeaker(
    I2S_SPEAKER_BCLK_PIN, 
    I2S_SPEAKER_WCLK_PIN, 
    I2S_SPEAKER_DATA_PIN
  );
  
  if (i2sSpeaker->init(I2S_SPEAKER_SAMPLE_RATE, I2S_SPEAKER_BITS_PER_SAMPLE)) {
    i2sSpeaker->setVolume(I2S_SPEAKER_DEFAULT_VOLUME);
    logger->info("I2S speaker (MAX98357) initialized successfully");
    
    // Play startup sound to confirm speaker is working
    // i2sSpeaker->playStartup(I2S_SPEAKER_DEFAULT_VOLUME);
  } else {
    logger->error("I2S speaker (MAX98357) initialization failed");
    delete i2sSpeaker;
    i2sSpeaker = nullptr;
  }
  #else
  i2sSpeaker = nullptr;
  #endif
  
  #else
  logger->info("Speakers disabled in configuration");
  pwmSpeaker = nullptr;
  i2sSpeaker = nullptr;
  #endif
}

void playSpeakerTone(int frequency, int duration, int volume) {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->playTone(frequency, duration, volume);
    return;
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->playTone(frequency, duration, volume);
    return;
  }
  #endif
  #endif
}

void playSpeakerBeep(int volume) {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->beep(volume);
    return;
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->beep(volume);
    return;
  }
  #endif
  #endif
}

void playSpeakerConfirmation(int volume) {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->playConfirmation(volume);
    return;
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->playConfirmation(volume);
    return;
  }
  #endif
  #endif
}

void playSpeakerError(int volume) {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->playError(volume);
    return;
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->playError(volume);
    return;
  }
  #endif
  #endif
}

void playSpeakerStartup(int volume) {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->playStartup(volume);
    return;
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->playStartup(volume);
    return;
  }
  #endif
  #endif
}

void playSpeakerNotification(int volume) {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->playNotification(volume);
    return;
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->playNotification(volume);
    return;
  }
  #endif
  #endif
}

void stopSpeaker() {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->stop();
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->stop();
  }
  #endif
  #endif
}

void setSpeakerVolume(int volume) {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->setVolume(volume);
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->setVolume(volume);
  }
  #endif
  #endif
}

bool isSpeakerPlaying() {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    return pwmSpeaker->isPlaying();
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    return i2sSpeaker->isPlaying();
  }
  #endif
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
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    return true;
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    return true;
  }
  #endif
  #endif
  
  return false;
}

// Get speaker type for WebSocket
String getSpeakerType() {
  #if SPEAKER_ENABLED
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    return "PWM";
  }
  #endif
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    return "I2S_MAX98357";
  }
  #endif
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
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    // For I2S speaker, play as raw audio data
    i2sSpeaker->playAudioData(audioData, dataSize, volume);
    return true;
  }
  #endif
  
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    // For PWM speaker, play as frequency data with default sample rate
    pwmSpeaker->playAudioData(audioData, dataSize, 8000, volume); // 8kHz sample rate
    return true;
  }
  #endif
  
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
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    i2sSpeaker->playAudioData(data, dataSize, volume);
    return;
  }
  #endif
  
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    pwmSpeaker->playAudioData(data, dataSize, sampleRate, volume);
    return;
  }
  #endif
  
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
  
  #if SPEAKER_TYPE_I2S
  if (i2sSpeaker && i2sSpeaker->isInitialized()) {
    return i2sSpeaker->playMP3File(filePath, volume);
  }
  #endif
  
  #if SPEAKER_TYPE_PWM
  if (pwmSpeaker && pwmSpeaker->isInitialized()) {
    return pwmSpeaker->playMP3File(filePath, volume);
  }
  #endif
  
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
  Audio::MP3Decoder decoder;
  if (!decoder.init()) {
    logger->error("Failed to initialize MP3 decoder");
    return false;
  }
  
  Audio::MP3Decoder::MP3Info info;
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
  Audio::MP3Decoder decoder;
  if (!decoder.init()) {
    logger->error("Failed to initialize MP3 decoder");
    return false;
  }
  
  // Decode MP3 to PCM
  int16_t* pcmBuffer = nullptr;
  size_t pcmSize = 0;
  Audio::MP3Decoder::MP3Info info;
  
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
