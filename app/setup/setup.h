#pragma once

#include "Config.h"
#include "Constants.h"
#include <Notification.h>
#include <vector>
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
#include <AnalogMicrophone.h>
#include "I2SSpeaker.h"
#include <AudioSamples.h>
#include <MP3Player.h>
#include "tasks/register.h"
#include "callback/register.h"

extern Notification* notification;
extern Automation::Automation* automation;
extern Sensors::Camera* camera;
extern Sensors::OrientationSensor* orientation;
extern Sensors::DistanceSensor* distanceSensor;
extern Sensors::CliffDetector* cliffLeftDetector;
extern Sensors::CliffDetector* cliffRightDetector;
extern Sensors::TemperatureSensor* temperatureSensor;
extern Motors::MotorControl* motors;
extern Motors::ServoControl* servos;
extern Communication::WiFiManager* wifiManager;
extern Communication::WebServer* webServer;
extern Communication::WebSocketHandler* webSocket;
extern Communication::GPTAdapter* gptAdapter;
extern Screen::Screen* screen;
extern Utils::FileManager* fileManager;
extern Utils::Logger* logger;
extern Utils::CommandMapper* commandMapper;
extern Utils::IOExtern ioExpander;
extern AnalogMicrophone* amicrophone;
extern I2SSpeaker* i2sSpeaker;
extern AudioSamples* audioSamples;

void setupLogger();
void setupNotification();
void setupFilemanager();
void setupCamera();
void setupMotors();
void setupServos();
void setupOrientation();
void setupDistanceSensor();
void setupCliffDetector();
void setupTemperatureSensor();
void setupMicrophone();
void setupSpeakers();
void setupScreen();
void setupWiFi();
void setupWebServer();
void setupWebSocket();
void setupGPT();
void setupTasks();
void setupCommandMapper();
void setupAutomation();
void setupExtender();

void setupApp();