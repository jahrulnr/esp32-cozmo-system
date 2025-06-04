#ifndef INIT_H
#define INIT_H

#include "Config.h"

// Include core libraries
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "../lib/Sensors/Camera.h"
#include "../lib/Sensors/Gyro.h"
#include "../lib/Motors/MotorControl.h"
#include "../lib/Motors/ServoControl.h"
#include "../lib/Communication/WiFiManager.h"
#include "../lib/Communication/WebServer.h"
#include "../lib/Communication/WebSocketHandler.h"
#include "../lib/Screen/Screen.h"
#include "../lib/Utils/FileManager.h"
#include "../lib/Utils/HealthCheck.h"
#include "../lib/Utils/Logger.h"
#include "../lib/Utils/SpiAllocator.h"
#include "../lib/Utils/I2CScanner.h"
#include "../lib/Utils/I2CManager.h"
#include "../lib/Utils/Sstring.h"

// Component instances
extern Sensors::Camera* camera;
extern Sensors::Gyro* gyro;
extern Motors::MotorControl* motors;
extern Motors::ServoControl* servos;
extern Communication::WiFiManager* wifiManager;
extern Communication::WebServer* webServer;
extern Communication::WebSocketHandler* webSocket;
extern Screen::Screen* screen;
extern Utils::FileManager* fileManager;
extern Utils::HealthCheck* healthCheck;
extern Utils::Logger* logger;
extern bool g_isApOnlyMode;

// Task handles
extern TaskHandle_t cameraStreamTaskHandle;
extern TaskHandle_t sensorMonitorTaskHandle;

// Forward declarations
void setupCamera();
void startCameraStreaming();
void stopCameraStreaming();
bool isCameraStreaming();
void cameraStreamTask(void* parameter);
void setupMotors();
void setupServos();
void setupGyro();
void setupScreen();
void setupWiFi();
bool isApOnlyMode();
void setupWebServer();
void setupWebSocket();
void setupHealthCheck();
void setupTasks();
void initTasks();
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                         AwsEventType type, void* arg, uint8_t* data, size_t len);

#endif