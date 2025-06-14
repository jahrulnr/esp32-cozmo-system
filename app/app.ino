#include <Arduino.h>
#include <soc/soc.h>
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "app.h"

Automation::Automation* automation = nullptr;
Sensors::Camera* camera = nullptr;
Sensors::OrientationSensor* orientation = nullptr;
Sensors::DistanceSensor* distanceSensor = nullptr;
Sensors::CliffDetector* cliffLeftDetector = nullptr;
Sensors::CliffDetector* cliffRightDetector = nullptr;
Sensors::TemperatureSensor* temperatureSensor = nullptr;
Motors::MotorControl* motors = nullptr;
Motors::ServoControl* servos = nullptr;
Communication::WiFiManager* wifiManager = nullptr;
Communication::WebServer* webServer = nullptr;
Communication::WebSocketHandler* webSocket = nullptr;
Communication::GPTAdapter* gptAdapter = nullptr;
Screen::Screen* screen = nullptr;
Utils::FileManager* fileManager = nullptr;
Utils::HealthCheck* healthCheck = nullptr;
Utils::Logger* logger = nullptr;
Utils::CommandMapper* commandMapper = nullptr;

void setup() {
  heap_caps_malloc_extmem_enable(0);  
  disableLoopWDT();
  setCpuFrequencyMhz(240);

  // Initialize Serial
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n\nCozmo System Starting...");
  
  // Initialize Logger
  logger = &Utils::Logger::getInstance();
  logger->init(true, true, "/logs.txt");
  logger->info("Logger initialized");
  
  // Initialize components
  setupPins();
  setupConfigManager();
  setupScreen();
  setupWiFi();
  setupCamera();
  setupMotors();
  setupServos();
  setupOrientation();
  setupDistanceSensor();
  setupCliffDetector();
  setupTemperatureSensor();
  setupWebServer();
  setupWebSocket();
  setupGPT();
  setupCommandMapper();
  setupAutomation();
  setupHealthCheck();

  if (motors && screen)
    motors->setScreen(screen);
  if (servos && screen)
    servos->setScreen(screen);
  
  logger->info("System initialization complete");
  
  if (screen) {
    screen->clear();
    screen->drawCenteredText(20, "Cozmo System");
    screen->drawCenteredText(40, "Ready!");
    screen->update();
  }

  setupTasks();
}

void loop() {
  if (healthCheck)
    healthCheck->update();

  if (screen)
      screen->mutexUpdate();

  vTaskDelay(pdMS_TO_TICKS(33));
}