#include <Arduino.h>
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

// Forward declarations
void setupCamera();
void setupMotors();
void setupServos();
void setupGyro();
void setupScreen();
void setupWiFi();
void setupWebServer();
void setupWebSocket();
void setupHealthCheck();
void setupTasks();
void initTasks();
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                         AwsEventType type, void* arg, uint8_t* data, size_t len);

// Component instances
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
  setupTasks();
  
  logger->info("System initialization complete");
  
  if (screen) {
    screen->clear();
    screen->drawCenteredText(20, "Cozmo System");
    screen->drawCenteredText(40, "Ready!");
    screen->update();
  }
}

long timer = millis();
void loop() {
  // Run health checks
  if (healthCheck) {
    healthCheck->update();
  }
  
  if (millis() - timer > 5000){
    screen->clear();
    screen->drawCenteredText(40, "hello");
    screen->mutexUpdate();
    timer = millis();
  }
  else if (screen)
    screen->mutexUpdateFace();
  vTaskDelay(pdMS_TO_TICKS(33));
}

// Component setup functions
void setupCamera() {
  if (CAMERA_ENABLED) {
    logger->info("Setting up camera...");
    camera = new Sensors::Camera();
    if (camera->init()) {
      camera->setResolution(CAMERA_FRAME_SIZE);
      logger->info("Camera initialized successfully");
    } else {
      logger->error("Camera initialization failed");
    }
  }
}

void setupMotors() {
  if (MOTOR_ENABLED) {
    logger->info("Setting up motors...");
    motors = new Motors::MotorControl();
    if (motors->init(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2)) {
      motors->setSpeed(MOTOR_SPEED_DEFAULT);
      logger->info("Motors initialized successfully");
    } else {
      logger->error("Motors initialization failed");
    }
  }
}

void setupServos() {
  if (SERVO_ENABLED) {
    logger->info("Setting up servos...");
    servos = new Motors::ServoControl();
    if (servos->init(HEAD_SERVO_PIN, HAND_SERVO_PIN)) {
      servos->setHead(DEFAULT_HEAD_ANGLE);
      servos->setHand(DEFAULT_HAND_ANGLE);
      logger->info("Servos initialized successfully");
    } else {
      logger->error("Servos initialization failed");
    }
  }
}

void setupGyro() {
  if (GYRO_ENABLED) {
    logger->info("Setting up gyroscope...");
    gyro = new Sensors::Gyro();
    if (gyro->init(GYRO_SDA_PIN, GYRO_SCL_PIN)) {
      gyro->calibrate();
      logger->info("Gyroscope initialized successfully");
    } else {
      logger->error("Gyroscope initialization failed");
    }
  }
}

void setupScreen() {
  if (SCREEN_ENABLED) {
    logger->info("Setting up screen...");
    screen = new Screen::Screen();
    if (screen->init(SCREEN_SDA_PIN, SCREEN_SCL_PIN)) {
      screen->clear();
      screen->drawCenteredText(20, "Cozmo System");
      screen->drawCenteredText(40, "Starting...");
      screen->update();
      logger->info("Screen initialized successfully");
    } else {
      logger->error("Screen initialization failed");
    }
  }
}

void setupWiFi() {
  if (WIFI_ENABLED) {
    logger->info("Setting up WiFi...");
    wifiManager = new Communication::WiFiManager();
    wifiManager->init();
    
    if (screen) {
      screen->clear();
      screen->drawCenteredText(20, "Connecting to");
      screen->drawCenteredText(40, WIFI_SSID);
      screen->update();
    }
    
    if (wifiManager->connect(WIFI_SSID, WIFI_PASSWORD)) {
      logger->info("Connected to WiFi: " + String(WIFI_SSID));
      logger->info("IP: " + wifiManager->getIP());
      
      if (screen) {
        screen->clear();
        screen->drawCenteredText(10, "WiFi Connected");
        screen->drawCenteredText(30, WIFI_SSID);
        screen->drawCenteredText(50, wifiManager->getIP());
        screen->update();
        delay(2000);
      }
    } else {
      logger->warning("WiFi connection failed, starting AP mode");
      
      if (screen) {
        screen->clear();
        screen->drawCenteredText(20, "Starting AP");
        screen->drawCenteredText(40, WIFI_AP_SSID);
        screen->update();
      }
      
      if (wifiManager->startAP(WIFI_AP_SSID, WIFI_AP_PASSWORD)) {
        logger->info("AP started: " + String(WIFI_AP_SSID));
        logger->info("IP: " + wifiManager->getIP());
        
        if (screen) {
          screen->clear();
          screen->drawCenteredText(10, "AP Mode Active");
          screen->drawCenteredText(30, WIFI_AP_SSID);
          screen->drawCenteredText(50, wifiManager->getIP());
          screen->update();
          delay(2000);
        }
      } else {
        logger->error("AP start failed");
      }
    }
  }
}

void setupWebServer() {
  if (WEBSERVER_ENABLED) {
    logger->info("Setting up web server...");
    
    // Initialize SPIFFS for file operations
    fileManager = new Utils::FileManager();
    if (!fileManager->init()) {
      logger->error("SPIFFS initialization failed");
      return;
    }
    
    webServer = new Communication::WebServer();
    if (webServer->init(WEBSERVER_PORT)) {
      // Setup routes
      webServer->on("/", [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
      });
      
      webServer->serveStatic("/", "text/html");
      webServer->serveStatic("/css", "text/css");
      webServer->serveStatic("/js", "application/javascript");
      
      webServer->onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
      });
      
      webServer->begin();
      logger->info("Web server started on port " + String(WEBSERVER_PORT));
    } else {
      logger->error("Web server initialization failed");
    }
  }
}

void setupWebSocket() {
  if (WEBSOCKET_ENABLED) {
    logger->info("Setting up WebSocket server...");
    webSocket = new Communication::WebSocketHandler();
    
    // Initialize WebSocket with path and existing web server
    if (webSocket->init("/ws", webServer ? webServer->getServer() : nullptr)) {
      // Set the event handler
      webSocket->onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                           AwsEventType type, void* arg, uint8_t* data, size_t len) {
        handleWebSocketEvent(server, client, type, arg, data, len);
      });
      
      webSocket->begin();
      logger->info("WebSocket server started on path /ws");
    } else {
      logger->error("WebSocket server initialization failed");
    }
  }
}

void setupHealthCheck() {
  if (HEALTH_CHECK_ENABLED) {
    logger->info("Setting up health checks...");
    healthCheck = new Utils::HealthCheck();
    healthCheck->init(HEALTH_CHECK_INTERVAL);
    
    // Add health checks
    healthCheck->addCheck("WiFi", []() {
      if (!wifiManager || !wifiManager->isConnected()) {
        return Utils::HealthCheck::Status::WARNING;
      }
      return Utils::HealthCheck::Status::HEALTHY;
    });
    
    healthCheck->addCheck("Camera", []() {
      if (!camera) {
        return Utils::HealthCheck::Status::WARNING;
      }
      return Utils::HealthCheck::Status::HEALTHY;
    });
    
    // Set callback for status changes
    healthCheck->setStatusChangeCallback([](const String& name, Utils::HealthCheck::Status oldStatus, Utils::HealthCheck::Status newStatus) {
      logger->info("Health check '" + name + "' changed from " + String(static_cast<int>(oldStatus)) + " to " + String(static_cast<int>(newStatus)));
    });
    
    logger->info("Health checks initialized");
  }
}

void setupTasks() {
  // Setup tasks here (will be implemented in tasks.cpp)
	initTasks();
}

// WebSocket event handler
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                         AwsEventType type, void* arg, uint8_t* data, size_t len) {
  uint32_t clientId = client->id();
  
  switch (type) {
    case WS_EVT_DISCONNECT:
      logger->info("WebSocket client #" + String(clientId) + " disconnected");
      break;
      
    case WS_EVT_CONNECT:
      {
        IPAddress ip = client->remoteIP();
        logger->info("WebSocket client #" + String(clientId) + " connected from " + ip.toString());
        client->text("Connected");
      }
      break;
      
    case WS_EVT_DATA:
      {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        
        // Handle text data
        if (info->opcode == WS_TEXT) {
          // Null-terminate the data
          data[len] = 0;
          String text = String((char*)data);
          logger->debug("Received text from client #" + String(clientId) + ": " + text);
          
          // Process commands here
          if (text.startsWith("move:")) {
            String direction = text.substring(5);
            
            if (motors) {
              if (direction == "forward") {
                motors->move(Motors::MotorControl::Direction::FORWARD);
              } else if (direction == "backward") {
                motors->move(Motors::MotorControl::Direction::BACKWARD);
              } else if (direction == "left") {
                motors->move(Motors::MotorControl::Direction::LEFT);
              } else if (direction == "right") {
                motors->move(Motors::MotorControl::Direction::RIGHT);
              } else if (direction == "stop") {
                motors->stop();
              }
            }
          } else if (text.startsWith("servo:")) {
            // Handle servo commands
          } else if (text == "status") {
            // Send status information
          }
        }
        // Handle binary data
        else if (info->opcode == WS_BINARY) {
          logger->debug("Received binary data from client #" + String(clientId) + ", length: " + String(len));
        }
      }
      break;
      
    case WS_EVT_PONG:
      logger->debug("WebSocket client #" + String(clientId) + " pong");
      break;
      
    case WS_EVT_ERROR:
      logger->error("WebSocket client #" + String(clientId) + " error");
      break;
  }
}