#ifndef INIT_H
#define INIT_H

#include "Config.h"

// Include core libraries
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "lib/Sensors/Camera.h"
#include "lib/Sensors/OrientationSensor.h"
#include "lib/Sensors/DistanceSensor.h"
#include "lib/Motors/MotorControl.h"
#include "lib/Motors/ServoControl.h"
#include "lib/Communication/WiFiManager.h"
#include "lib/Communication/WebServer.h"
#include "lib/Communication/WebSocketHandler.h"
#include "lib/Communication/GPTAdapter.h"
#include "lib/Screen/Screen.h"
#include "lib/Utils/FileManager.h"
#include "lib/Utils/HealthCheck.h"
#include "lib/Utils/Logger.h"
#include "lib/Utils/SpiAllocator.h"
#include "lib/Utils/I2CScanner.h"
#include "lib/Utils/I2CManager.h"
#include "lib/Utils/Sstring.h"
#include "lib/Utils/CommandMapper.h"

// Component instances
extern Sensors::Camera* camera;
extern Sensors::OrientationSensor* orientation;
extern Sensors::DistanceSensor* distanceSensor;
extern Motors::MotorControl* motors;
extern Motors::ServoControl* servos;
extern Communication::WiFiManager* wifiManager;
extern Communication::WebServer* webServer;
extern Communication::WebSocketHandler* webSocket;
extern Communication::GPTAdapter* gptAdapter;
extern Screen::Screen* screen;
extern Utils::FileManager* fileManager;
extern Utils::HealthCheck* healthCheck;
extern Utils::Logger* logger;
extern Utils::CommandMapper* commandMapper;
extern bool g_isApOnlyMode;

// Task handles
extern TaskHandle_t cameraStreamTaskHandle;
extern TaskHandle_t sensorMonitorTaskHandle;
extern TaskHandle_t automationTaskHandle;
extern TaskHandle_t gptTaskHandle;

// Function prototypes
void automationTask(void* parameter);
void cameraStreamTask(void* parameter);
void sensorMonitorTask(void* parameter);
void resetMap();
String getMapAsJson();
bool saveMapToFile();
bool loadMapFromFile();
void sendGPT(const String &prompt, Communication::GPTAdapter::ResponseCallback callback);
bool logGPTInteraction(const String& prompt, const String& response);
String getGPTLearningData();
bool clearGPTLearningData();

// Forward declarations
void setupCamera();
void startCameraStreaming();
void stopCameraStreaming();
bool isCameraStreaming();
void setupMotors();
void setupServos();
void setupOrientation();
void setupDistanceSensor();
void setupScreen();
void setupWiFi();
bool isApOnlyMode();
void setupWebServer();
void setupWebSocket();
void setupGPT();
void setupHealthCheck();
void setupTasks();
void setupCommandMapper();
String processTextCommands(const String& text);
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                         AwsEventType type, void* arg, uint8_t* data, size_t len);

#endif