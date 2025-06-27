#ifndef INIT_H
#define INIT_H

#include "Config.h"

// Include core libraries
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
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
// #include "lib/Communication/SPIHandler.h"
#include "lib/Screen/Screen.h"
#include "lib/Utils/FileManager.h"
#include "lib/Utils/HealthCheck.h"
#include "lib/Utils/Logger.h"
#include "lib/Utils/SpiAllocator.h"
#include "lib/Utils/I2CScanner.h"
#include "lib/Utils/I2CManager.h"
#include "lib/Utils/IOExtern.h"
#include "lib/Utils/Sstring.h"
#include "lib/Utils/CommandMapper.h"
#include "lib/Utils/ConfigManager.h"
#include "lib/Audio/PWMSpeaker.h"
#include "lib/Audio/I2SSpeaker.h"

// Structure to store slave camera data information
struct SlaveCameraData {
  bool dataAvailable;          // Whether camera data is available
  uint16_t width;              // Image width
  uint16_t height;             // Image height
  uint32_t totalSize;          // Total size of the camera frame in bytes
  uint16_t totalBlocks;        // Total number of blocks
  uint16_t blockSize;          // Size of each block in bytes
  uint16_t receivedBlocks;     // Number of blocks received so far
  uint8_t* imageData;          // Buffer to store the assembled image
  bool* blockReceived;         // Array to track which blocks have been received
  bool frameComplete;          // Whether the frame is complete
  uint8_t dataVersion;         // Version of the data format
};

// Declare external reference to the global SlaveCameraData instance
extern SlaveCameraData slaveCameraData;

struct gptRequest
{
	String prompt;
	Communication::GPTAdapter::ResponseCallback callback;
	bool saveToLog;  // Flag to indicate if this interaction should be logged
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
extern Audio::PWMSpeaker* pwmSpeaker;
extern Audio::I2SSpeaker* i2sSpeaker;
extern Communication::WiFiManager* wifiManager;
extern Communication::WebServer* webServer;
extern Communication::WebSocketHandler* webSocket;
extern Communication::GPTAdapter* gptAdapter;
// extern Communication::SPIHandler* spiHandler;
extern Screen::Screen* screen;
extern Utils::FileManager* fileManager;
extern Utils::HealthCheck* healthCheck;
extern Utils::Logger* logger;
extern Utils::CommandMapper* commandMapper;
extern Utils::ConfigManager* configManager;
extern Utils::IOExtern ioExpander;
extern bool g_isApOnlyMode;

// Task handles
extern TaskHandle_t cameraStreamTaskHandle;
extern TaskHandle_t sensorMonitorTaskHandle;
extern TaskHandle_t gptTaskHandle;
extern TaskHandle_t automationTaskHandle;

// Automation control
extern bool _enableAutomation;
extern unsigned long _lastManualControlTime;

// SPI control
// bool sendPingToSlave();

// Function prototypes
void protectCozmo();
void protectCozmoTask(void * param);
void gptChatTask(void* parameter);
void cameraStreamTask(void* parameter);
void sensorMonitorTask(void* parameter);
void sendGPT(const String &prompt, Communication::GPTAdapter::ResponseCallback callback);
void setupSPI();
void setupExtender();

// Forward declarations
// void setupSPI();
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
void playSpeakerTone(int frequency, int duration, int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerBeep(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerConfirmation(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerError(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerStartup(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerNotification(int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void stopSpeaker();
void setSpeakerVolume(int volume);
bool isSpeakerPlaying();
void playBehaviorSound(const String& behavior);
bool getSpeakerStatus();
String getSpeakerType();
bool playSpeakerAudioFile(const String& filePath, int volume = I2S_SPEAKER_DEFAULT_VOLUME);
void playSpeakerAudioData(const uint8_t* data, size_t dataSize, uint32_t sampleRate = 16000, int volume = I2S_SPEAKER_DEFAULT_VOLUME);
bool createAudioFile(const String& filePath, const int16_t* samples, size_t sampleCount, uint32_t sampleRate = 16000);

// MP3 audio functions
bool playSpeakerMP3File(const String& filePath, int volume = I2S_SPEAKER_DEFAULT_VOLUME);
bool getMP3FileInfo(const String& filePath, int* sampleRate = nullptr, int* channels = nullptr, int* bitRate = nullptr, int* duration = nullptr);
bool convertMP3ToAudioFile(const String& mp3FilePath, const String& audioFilePath);

void setupScreen();
void setupWiFi();
bool isApOnlyMode();
void setupWebServer();
void setupWebSocket();
void setupGPT();
void setupHealthCheck();
void setupTasks();
void setupCommandMapper();
void setupConfigManager();
void setupAutomation();
void updateManualControlTime();
bool isAutomationEnabled();
void setAutomationEnabled(bool enabled);
String processTextCommands(const String& text);
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                         AwsEventType type, void* arg, uint8_t* data, size_t len);

#endif