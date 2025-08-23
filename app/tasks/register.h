#pragma once
#include <Arduino.h>
#include "setup/setup.h"

// Task handles
extern TaskHandle_t cameraStreamTaskHandle;
extern TaskHandle_t sensorMonitorTaskHandle;
extern TaskHandle_t gptTaskHandle;

void protectCozmoTask(void * param);
void gptChatTask(void* parameter);
void cameraStreamTask(void* parameter);
void sensorMonitorTask(void* parameter);

extern bool g_isApOnlyMode;
extern bool _cameraStreaming;
void startCameraStreaming();
void stopCameraStreaming();
bool isCameraStreaming();

void checkTemperature();struct gptRequest
{
	String prompt;
	Communication::GPTAdapter::ResponseCallback callback;
};

// Automation control
extern bool _enableAutomation;
extern unsigned long _lastManualControlTime;

// Function prototypes
void protectCozmo();
void sendGPT(const String &prompt, Communication::GPTAdapter::ResponseCallback callback);

// Forward declarations
bool cliffDetected();
void playSpeakerTone(int frequency, int duration, int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerBeep(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerConfirmation(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerError(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerStartup(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerNotification(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void stopSpeaker();
void setSpeakerVolume(int volume);
int getSpeakerVolume();
bool isSpeakerPlaying();
void playBehaviorSound(const String& behavior);
bool getSpeakerStatus();
String getSpeakerType();
bool playSpeakerAudioFile(const String& filePath, int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerAudioData(const uint8_t* data, size_t dataSize, uint32_t sampleRate = I2S_SPEAKER_SAMPLE_RATE, int volume = I2S_SPEAKER_DEFAULT_VOLUME);
bool createAudioFile(const String& filePath, const int16_t* samples, size_t sampleCount, uint32_t sampleRate = I2S_SPEAKER_SAMPLE_RATE);

// MP3 audio functions
bool playSpeakerMP3File(const String& filePath, int volume = I2S_SPEAKER_DEFAULT_VOLUME);
bool getMP3FileInfo(const String& filePath, int* sampleRate = nullptr, int* channels = nullptr, int* bitRate = nullptr, int* duration = nullptr);
bool convertMP3ToAudioFile(const String& mp3FilePath, const String& audioFilePath);

// Random MP3 playback functions
bool playSpeakerRandomMP3(int volume, Utils::FileManager::StorageType storageType);
bool playSpeakerRandomMP3(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
std::vector<String> getAvailableMP3Files(Utils::FileManager::StorageType storageType);
std::vector<String> getAvailableMP3Files();

bool isApOnlyMode();
void updateManualControlTime();
bool isAutomationEnabled();
void setAutomationEnabled(bool enabled);
String processTextCommands(const String& text);
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                         AwsEventType type, void* arg, uint8_t* data, size_t len);
