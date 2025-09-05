#pragma once

#include <esp_log.h>
#include <vector>
#include <Config.h>
#include <Constants.h>
#include <picotts.h>
#include <csr.h>
#include <Notification.h>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <display/Display.h>
#include <FileManager.h>
#include <Logger.h>
#include <I2CScanner.h>
#include <I2CManager.h>
#include <IOExtern.h>
#include <Sstring.h>
#include <AnalogMicrophone.h>
#include <I2SMicrophone.h>
#include <I2SSpeaker.h>
#include <AudioSamples.h>
#include <MP3Player.h>
#include <FTPServer.h>
#include "core/Utils/SpiAllocator.h"
#include "core/Automation/Automation.h"
#include "core/Sensors/Camera.h"
#include "core/Sensors/OrientationSensor.h"
#include "core/Sensors/DistanceSensor.h"
#include "core/Sensors/CliffDetector.h"
#include "core/Sensors/TouchDetector.h"
#include "core/Sensors/TemperatureSensor.h"
#include "core/Motors/MotorControl.h"
#include "core/Motors/ServoControl.h"
#include "core/Communication/WiFiManager.h"
#include "core/Communication/GPTAdapter.h"
#include "core/Communication/WeatherService.h"
#include "core/Utils/CommandMapper.h"
#include "repository/Configuration.h"
#include "repository/AdministrativeRegion.h"
#include "tasks/register.h"
#include "callback/register.h"
#include "web/Routes/routes.h"

#ifndef WEB_VAR_H
#define WEB_VAR_H
struct {
  bool authenticated = false;
} sessions[5];
#endif

extern Notification* notification;
extern Automation::Automation* automation;
extern Sensors::Camera* camera;
extern Sensors::OrientationSensor* orientation;
extern Sensors::DistanceSensor* distanceSensor;
extern Sensors::CliffDetector* cliffLeftDetector;
extern Sensors::CliffDetector* cliffRightDetector;
extern Sensors::TouchDetector* touchDetector;
extern Sensors::TemperatureSensor* temperatureSensor;
extern Motors::MotorControl* motors;
extern Motors::ServoControl* servos;
extern Communication::WiFiManager* wifiManager;
extern Communication::GPTAdapter* gptAdapter;
extern Communication::WeatherService* weatherService;
extern Display::Display* display;
extern Utils::FileManager* fileManager;
extern Utils::Logger* logger;
extern Utils::CommandMapper* commandMapper;
extern Utils::IOExtern ioExpander;
extern AnalogMicrophone* amicrophone;
extern I2SMicrophone* microphone;
extern I2SSpeaker* i2sSpeaker;
extern AudioSamples* audioSamples;
extern FTPServer ftpSrv;

void setupApp();

void setupLogger();
void setupNotification();
void setupFilemanager();
void setupWebServer();
void setupCamera();
void setupMotors();
void setupServos();
void setupOrientation();
void setupDistanceSensor();
void setupCliffDetector();
void setupTouchDetector();
void setupTemperatureSensor();
void setupMicrophone();
void setupSpeakers();
void setupDisplay();
void setupWiFi();
void setupGPT();
void setupTasks();
void setupCommandMapper();
void setupAutomation();
void setupExtender();
void setupSpeechRecognition();
void setupPicoTTS();
void setupFTPServer();
void setupWeather();

#if PICOTTS_ENABLED
extern bool picotts_initialized;
#endif
bool sayText(const char* text);