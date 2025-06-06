#include <Arduino.h>
#include "init.h"

Sensors::Camera* camera = nullptr;
Sensors::Gyro* gyro = nullptr;
Motors::MotorControl* motors = nullptr;
Motors::ServoControl* servos = nullptr;
Communication::WiFiManager* wifiManager = nullptr;
Communication::WebServer* webServer = nullptr;
Communication::WebSocketHandler* webSocket = nullptr;
Screen::Screen* screen = nullptr;
Utils::FileManager* fileManager = nullptr;
Utils::HealthCheck* healthCheck = nullptr;
Utils::Logger* logger = nullptr;

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
  setupScreen();
  setupWiFi();
  setupCamera();
  setupMotors();
  setupServos();
  setupGyro();
  setupWebServer();
  setupWebSocket();
  setupHealthCheck();
  
  logger->info("System initialization complete");
  
  if (screen) {
    screen->clear();
    screen->drawCenteredText(20, "Cozmo System");
    screen->drawCenteredText(40, "Ready!");
    screen->update();
  }

  setupTasks();
}

long timer = millis();
void loop() {
  // Run health checks
  if (healthCheck) {
    healthCheck->update();
  }
  
  if (millis() - timer > 5000){
    screen->mutexClear();
    screen->drawCenteredText(40, "hello");
    screen->mutexUpdate();
    timer = millis();
  }
  else if (screen)
    screen->mutexUpdateFace();
  vTaskDelay(pdMS_TO_TICKS(33));
}