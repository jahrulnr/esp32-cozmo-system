#pragma once

#include <esp_log.h>
#include "Config.h"
#include "Constants.h"
#include <Notification.h>
#include <vector>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "core/Utils/SpiAllocator.h"
#include "core/Automation/Automation.h"
#include "core/Sensors/Camera.h"
#include "core/Sensors/OrientationSensor.h"
#include "core/Sensors/DistanceSensor.h"
#include "core/Sensors/CliffDetector.h"
#include "core/Sensors/TemperatureSensor.h"
#include "core/Motors/MotorControl.h"
#include "core/Motors/ServoControl.h"
#include "core/Communication/WiFiManager.h"
#include "core/Communication/GPTAdapter.h"
#include "core/Utils/CommandMapper.h"
#include "Screen.h"
#include "FileManager.h"
#include "Logger.h"
#include "I2CScanner.h"
#include "I2CManager.h"
#include "IOExtern.h"
#include "Sstring.h"
#include <AnalogMicrophone.h>
#include <I2SMicrophone.h>
#include "I2SSpeaker.h"
#include <AudioSamples.h>
#include <MP3Player.h>
#include "picotts.h"
#include "tasks/register.h"
#include "callback/register.h"

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
extern Sensors::TemperatureSensor* temperatureSensor;
extern Motors::MotorControl* motors;
extern Motors::ServoControl* servos;
extern Communication::WiFiManager* wifiManager;
extern Communication::GPTAdapter* gptAdapter;
extern Screen::Screen* screen;
extern Utils::FileManager* fileManager;
extern Utils::Logger* logger;
extern Utils::CommandMapper* commandMapper;
extern Utils::IOExtern ioExpander;
extern AnalogMicrophone* amicrophone;
extern I2SMicrophone* microphone;
extern I2SSpeaker* i2sSpeaker;
extern AudioSamples* audioSamples;

extern bool sr_system_running;

#if PICOTTS_ENABLED
extern bool picotts_initialized;
bool sayText(const char* text);
#endif

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
void setupGPT();
void setupTasks();
void setupCommandMapper();
void setupAutomation();
void setupExtender();
void setupSpeechRecognition();
void setupPicoTTS();

void setupApp();