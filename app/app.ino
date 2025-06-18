#include <Arduino.h>
#include <soc/soc.h>
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "app.h"
#include "lib/Communication/SPIHandler.h"

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
Communication::SPIHandler* spiHandler = nullptr;
Screen::Screen* screen = nullptr;
Utils::FileManager* fileManager = nullptr;
Utils::HealthCheck* healthCheck = nullptr;
Utils::Logger* logger = nullptr;
Utils::CommandMapper* commandMapper = nullptr;
Utils::ConfigManager* configManager = nullptr;

// Initialize the global slave camera data struct
SlaveCameraData slaveCameraData = {
  false,  // dataAvailable
  0,      // width
  0,      // height
  0,      // totalSize
  0,      // totalBlocks
  0,      // blockSize
  0,      // receivedBlocks
  nullptr, // imageData
  nullptr, // blockReceived
  false,   // frameComplete
  0        // dataVersion
};

void setup() {
  psramInit();
  if(psramAddToHeap()) {
    gpio_install_isr_service(ESP_INTR_FLAG_SHARED);
    heap_caps_malloc_extmem_enable(CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL);
  }

  // Initialize Serial
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n\nCozmo System Starting...");

  Serial.printf("Psram: %d\n", ESP.getFreePsram());
  
  // Initialize Logger
  logger = &Utils::Logger::getInstance();
  logger->init(true, true, "/logs.txt");
  logger->setLogLevel(Utils::LogLevel::DEBUG);
  logger->info("Logger initialized");

  fileManager = new Utils::FileManager();
  if (!fileManager->init()) {
    logger->error("SPIFFS initialization failed");
  }

  disableLoopWDT();
  setCpuFrequencyMhz(240);
  
  // Initialize components
  setupPins();
  // setupConfigManager();
  setupSPI(); // Initialize SPI buses and devices
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

unsigned long now = 0;
unsigned long cameraInterval = 1000;
void loop() {
  if (healthCheck)
    healthCheck->update();

  if (screen)
      screen->mutexUpdate();
  
  // Process any pending SPI messages in the queue
  if (spiHandler) {
    // Process up to 10 messages per loop iteration to avoid blocking too long
    int messagesProcessed = 0;
    while (messagesProcessed < 10 && spiHandler->processNextReceive()) {
      messagesProcessed++;
    }
    
    if (messagesProcessed > 0) {
      logger->debug("Processed %d SPI messages", messagesProcessed);
    }
  }

  // if (now == 0) now = millis();
  // if (millis() - now >= cameraInterval) {
  //   requestCameraFrame();
  //   now = millis();
  // }

  vTaskDelay(pdMS_TO_TICKS(33)); // ~30 times per second (1000ms / 30 ≈ 33ms)
}
