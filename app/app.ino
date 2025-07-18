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
Sensors::MicrophoneSensor* microphoneSensor = nullptr;
Motors::MotorControl* motors = nullptr;
Motors::ServoControl* servos = nullptr;
Audio::PWMSpeaker* pwmSpeaker = nullptr;
Audio::I2SSpeaker* i2sSpeaker = nullptr;
Communication::WiFiManager* wifiManager = nullptr;
Communication::WebServer* webServer = nullptr;
Communication::WebSocketHandler* webSocket = nullptr;
Communication::GPTAdapter* gptAdapter = nullptr;
// Communication::SPIHandler* spiHandler = nullptr;
Screen::Screen* screen = nullptr;
Utils::FileManager* fileManager = nullptr;
Utils::Logger* logger = nullptr;
Utils::CommandMapper* commandMapper = nullptr;
Utils::ConfigManager* configManager = nullptr;

void setup() {
  // gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
  heap_caps_malloc_extmem_enable(4096);

  // Initialize Serial
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n\nCozmo System Starting...");
  
  // Initialize Logger
  logger = &Utils::Logger::getInstance();
  logger->init(true, true, "/logs.txt");
  logger->setLogLevel(Utils::LogLevel::INFO);
  logger->info("Logger initialized");

  fileManager = new Utils::FileManager();
  if (!fileManager->init()) {
    logger->error("SPIFFS initialization failed");
  }

  setCpuFrequencyMhz(240);
  
  // Initialize components
  // setupConfigManager();
  // setupSPI(); // Initialize SPI buses and devices
  setupScreen();
  setupExtender();
  setupWiFi();
  setupMotors();
  setupServos();
  setupOrientation();
  setupDistanceSensor();
  setupCliffDetector();
  setupTemperatureSensor();
  setupMicrophone();
  setupSpeakers();
  setupWebServer();
  setupWebSocket();
  setupGPT();
  setupCommandMapper();
  setupAutomation();
  setupCamera();

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
  
  playSpeakerMP3File("/audio/boot.mp3");

  setupTasks();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(2000)); 
  // disableCore0WDT();
  // disableCore1WDT();
  disableLoopWDT();
  vTaskDelete(NULL);
}
