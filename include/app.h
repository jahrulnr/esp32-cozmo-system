#ifndef INIT_H
#define INIT_H

#include "Config.h"
#include <vector>

// Include core libraries
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "lib/Utils/SpiAllocator.h"
#include "lib/Automation/Automation.h"
#include "lib/Sensors/Camera.h"
#include "lib/Sensors/OrientationSensor.h"
#include "lib/Sensors/DistanceSensor.h"
#include "lib/Sensors/CliffDetector.h"
#include "lib/Sensors/TemperatureSensor.h"
#include "lib/Sensors/MicrophoneSensor.h"
#include "lib/Motors/MotorControl.h"
#include "lib/Motors/ServoControl.h"
#include "lib/Communication/WiFiManager.h"
#include "lib/Communication/WebServer.h"
#include "lib/Communication/WebSocketHandler.h"
#include "lib/Communication/GPTAdapter.h"
#include "lib/Utils/CommandMapper.h"
#include "lib/Screen/Screen.h"
#include "FileManager.h"
#include "Logger.h"
#include "I2CScanner.h"
#include "I2CManager.h"
#include "IOExtern.h"
#include "Sstring.h"
#include "I2SSpeaker.h"
#include <AudioSamples.h>
#include <MP3Player.h>

struct gptRequest
{
	String prompt;
	Communication::GPTAdapter::ResponseCallback callback;
};

// Component instances
extern Automation::Automation* automation;
extern Sensors::Camera* camera;
extern Sensors::OrientationSensor* orientation;
extern Sensors::DistanceSensor* distanceSensor;
extern Sensors::CliffDetector* cliffLeftDetector;
extern Sensors::CliffDetector* cliffRightDetector;
extern Sensors::TemperatureSensor* temperatureSensor;
extern Sensors::MicrophoneSensor* microphoneSensor;
extern Motors::MotorControl* motors;
extern Motors::ServoControl* servos;
extern I2SSpeaker* i2sSpeaker;
extern AudioSamples* audioSamples;
extern Communication::WiFiManager* wifiManager;
extern Communication::WebServer* webServer;
extern Communication::WebSocketHandler* webSocket;
extern Communication::GPTAdapter* gptAdapter;
// extern Communication::SPIHandler* spiHandler;
extern Screen::Screen* screen;
extern Utils::FileManager* fileManager;
extern Utils::Logger* logger;
extern Utils::CommandMapper* commandMapper;
extern Utils::IOExtern ioExpander;
extern bool g_isApOnlyMode;
extern bool _cameraStreaming;

// Task handles
extern TaskHandle_t cameraStreamTaskHandle;
extern TaskHandle_t sensorMonitorTaskHandle;
extern TaskHandle_t gptTaskHandle;
extern TaskHandle_t automationTaskHandle;

// Automation control
extern bool _enableAutomation;
extern unsigned long _lastManualControlTime;

// Function prototypes
void protectCozmo();
void protectCozmoTask(void * param);
void gptChatTask(void* parameter);
void cameraStreamTask(void* parameter);
void sensorMonitorTask(void* parameter);
void sendGPT(const String &prompt, Communication::GPTAdapter::ResponseCallback callback);
void setupExtender();

// Forward declarations
void setupCamera();
void startCameraStreaming();
void stopCameraStreaming();
bool isCameraStreaming();
void setupMotors();
void setupServos();
void setupOrientation();
void setupDistanceSensor();
void setupCliffDetector();
void setupTemperatureSensor();
void setupMicrophone();
void setupSpeakers();
void checkTemperature();
bool cliffDetected();
void checkMicrophone();
int getCurrentSoundLevel();
int getPeakSoundLevel();
bool isSoundDetected();
void calibrateMicrophone();
void setMicrophoneGain(int gainLevel);
void checkVoiceActivity(unsigned long currentTime);
void startVoiceRecording(unsigned long currentTime);
void stopVoiceRecording(unsigned long currentTime);
void processVoiceRecording(unsigned long duration);
bool isVoiceRecording();
bool isVoiceDetected();
void triggerVoiceRecording();
void stopVoiceRecordingManual();
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

void setupScreen();
void setupWiFi();
bool isApOnlyMode();
void setupWebServer();
void setupWebSocket();
void setupGPT();
void setupTasks();
void setupCommandMapper();
void setupAutomation();
void updateManualControlTime();
bool isAutomationEnabled();
void setAutomationEnabled(bool enabled);
String processTextCommands(const String& text);
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                         AwsEventType type, void* arg, uint8_t* data, size_t len);

#endif