#include <Arduino.h>
#include "app.h"

TaskHandle_t gptTaskHandle = nullptr;


void setupGPT(){
	gptAdapter = new Communication::GPTAdapter();
	#if GPT_ENABLED
	gptAdapter->init(GPT_API_KEY);
	gptAdapter->setModel(GPT_MODEL);
	gptAdapter->setMaxTokens(GPT_MAX_TOKENS);
	gptAdapter->setTemperature(GPT_TEMPERATURE);
  #endif
  
  // Create directory for data storage if enabled
  #if GPT_LEARNING_ENABLED
  static Utils::FileManager fileManager;
  if (fileManager.init()) {
    // Extract directory part from path
    String dirPath = "/data";
    if (!fileManager.exists(dirPath)) {
      fileManager.createDir(dirPath);
      logger->info("Created directory for GPT learning data");
    }
  }
  #endif
}

// Log a GPT interaction (prompt + response) to a file for learning purposes
bool logGPTInteraction(const String& prompt, const String& response) {
  #if GPT_LEARNING_ENABLED
  static Utils::FileManager fileManager;
  if (!fileManager.init()) {
    logger->error("Failed to initialize FileManager for GPT logging");
    return false;
  }
  
  String logEntry = "{\"timestamp\":" + String(millis()) + 
                   ",\"prompt\":\"" + prompt + 
                   "\",\"response\":\"" + response + "\"}\n";
  
  bool success = fileManager.appendFile(GPT_DATA_LOG_PATH, logEntry);
  if (success) {
    logger->debug("Logged GPT interaction to " + String(GPT_DATA_LOG_PATH));
  } else {
    logger->error("Failed to log GPT interaction");
  }
  return success;
  #else
  return false;
  #endif
}

void gptChatTask(void * param) {
	gptRequest *data = (gptRequest*) param;
	
	if (data != nullptr){
		// Create additional command with comprehensive hardware and sensor context
		String additionalCommand = "You are the AI brain of a Cozmo IoT Robot. Here's the current hardware status and sensor readings:\n\n";
		
		// ---- SYSTEM INFORMATION ----
		additionalCommand += "=== SYSTEM INFORMATION ===\n";
		additionalCommand += "System version: Cozmo IoT System (June 2025)\n";
		additionalCommand += "Serial baud rate: " + String(SERIAL_BAUD_RATE) + "\n";
		
		#if CONFIG_IDF_TARGET_ESP32
		additionalCommand += "Hardware: ESP32CAM\n";
		#elif CONFIG_IDF_TARGET_ESP32S3
		additionalCommand += "Hardware: ESP32-S3-DevKitC-1\n";
		#else
		additionalCommand += "Hardware: Unknown ESP32 variant\n";
		#endif
		
				// CPU temperature using our TemperatureSensor class
		if (temperatureSensor != nullptr && temperatureSensor->isSupported()) {
			float cpuTemp = temperatureSensor->readTemperature();
			if (!isnan(cpuTemp)) {
				additionalCommand += "CPU temperature: " + String(cpuTemp, 1) + "°C\n";
			} else {
				additionalCommand += "CPU temperature: Not available\n";
			}
		} else {
			additionalCommand += "CPU temperature: Sensor not supported\n";
		}
		
		// ---- SENSOR READINGS ----
		additionalCommand += "\n=== CURRENT SENSOR READINGS ===\n";
		
		// Distance sensor (ultrasonic) readings
		if (distanceSensor != nullptr) {
			float distance = distanceSensor->measureDistance();
			additionalCommand += "Distance sensor: " + String(distance) + " cm\n";
			bool isObstacle = (distance > 0 && distance < ULTRASONIC_OBSTACLE_TRESHOLD);
			additionalCommand += "Obstacle detected: " + String(isObstacle ? "Yes" : "No") + "\n";
		}
		
		// Cliff detector readings
		if (cliffLeftDetector != nullptr) {
			cliffLeftDetector->update(); // Ensure we have fresh data
			bool leftCliffDetected = cliffLeftDetector->isCliffDetected();
			additionalCommand += "Left cliff detector: " + String(leftCliffDetected ? "CLIFF DETECTED" : "No cliff") + "\n";
		}
		
		if (cliffRightDetector != nullptr) {
			cliffRightDetector->update(); // Ensure we have fresh data
			bool rightCliffDetected = cliffRightDetector->isCliffDetected();
			additionalCommand += "Right cliff detector: " + String(rightCliffDetected ? "CLIFF DETECTED" : "No cliff") + "\n";
		}
		
		// Orientation sensor readings
		if (orientation != nullptr) {
			// Add more specific orientation data if available
			additionalCommand += "Orientation sensors: Active\n";
			// If methods available, add specific values:
			additionalCommand += "Gyro X: " + String(orientation->getX()) + ", Y: " + String(orientation->getY()) + ", Z: " + String(orientation->getZ()) + "\n";
			additionalCommand += "Accel X: " + String(orientation->getAccelX()) + ", Y: " + String(orientation->getAccelY()) + ", Z: " + String(orientation->getAccelZ()) + "\n";
		}
		
		// ---- HARDWARE CONFIGURATION ----
		additionalCommand += "\n=== HARDWARE CONFIGURATION ===\n";
		
		// Motors configuration
		if (motors != nullptr) {
			additionalCommand += "Motors: Enabled \n";
			additionalCommand += "- Left Motor: PIN1=" + String(LEFT_MOTOR_PIN1) + ", PIN2=" + String(LEFT_MOTOR_PIN2) + "\n";
			additionalCommand += "- Right Motor: PIN1=" + String(RIGHT_MOTOR_PIN1) + ", PIN2=" + String(RIGHT_MOTOR_PIN2) + "\n";
		} else {
			additionalCommand += "Motors: Disabled\n";
		}
		
		// Servo information
		#if SERVO_ENABLED
		additionalCommand += "Servos: Enabled\n";
		additionalCommand += "- Head servo: Pin=" + String(HEAD_SERVO_PIN) + ", Default angle=" + String(DEFAULT_HEAD_ANGLE) + "°\n";
		additionalCommand += "- Hand servo: Pin=" + String(HAND_SERVO_PIN) + ", Default angle=" + String(DEFAULT_HAND_ANGLE) + "°\n";
		#else
		additionalCommand += "Servos: Disabled\n";
		#endif
		
		// Sensors configuration
		additionalCommand += "\nSensor Configuration:\n";
		
		// Ultrasonic sensor
		#if ULTRASONIC_ENABLED
		additionalCommand += "- Ultrasonic: Enabled (Trigger Pin=" + String(ULTRASONIC_TRIGGER_PIN) + ", Echo Pin=" + String(ULTRASONIC_ECHO_PIN) + ")\n";
		additionalCommand += "  Range: 0-" + String(ULTRASONIC_MAX_DISTANCE) + " cm, Obstacle threshold: " + String(ULTRASONIC_OBSTACLE_TRESHOLD) + " cm\n";
		#else
		additionalCommand += "- Ultrasonic: Disabled\n";
		#endif
		
		// Cliff detectors
		#if CLIFF_DETECTOR_ENABLED
		additionalCommand += "- Cliff detectors: Enabled (Digital sensors)\n";
		additionalCommand += "  Left detector pin: " + String(CLIFF_LEFT_DETECTOR_PIN) + " (1=cliff detected)\n";
		additionalCommand += "  Right detector pin: " + String(CLIFF_RIGHT_DETECTOR_PIN) + " (1=cliff detected)\n";
		#else
		additionalCommand += "- Cliff detectors: Disabled\n";
		#endif
		
		// Orientation sensors
		#if ORIENTATION_ENABLED
		additionalCommand += "- Orientation sensors: Enabled (I2C: SDA=" + String(ORIENTATION_SDA_PIN) + ", SCL=" + String(ORIENTATION_SCL_PIN) + ")\n";
		#else
		additionalCommand += "- Orientation sensors: Disabled\n";
		#endif
		
		// Camera configuration
		#if CAMERA_ENABLED
		#if CONFIG_IDF_TARGET_ESP32
		additionalCommand += "- Camera: Enabled (Model: AI-THINKER ESP32-CAM)\n";
		#elif CONFIG_IDF_TARGET_ESP32S3
		additionalCommand += "- Camera: Enabled (Model: ESP32-S3 OV2640)\n";
		#else
		additionalCommand += "- Camera: Enabled (Unknown model)\n";
		#endif
		additionalCommand += "  Resolution: " + String(CAMERA_FRAME_SIZE) + ", Quality: " + String(CAMERA_QUALITY) + ", FPS: " + String(CAMERA_FPS) + "\n";
		#else
		additionalCommand += "- Camera: Disabled\n";
		#endif
		
		// Display
		#if SCREEN_ENABLED
		additionalCommand += "- Screen: Enabled (OLED " + String(SCREEN_WIDTH) + "x" + String(SCREEN_HEIGHT) + " pixels)\n";
		additionalCommand += "  I2C pins: SDA=" + String(SCREEN_SDA_PIN) + ", SCL=" + String(SCREEN_SCL_PIN) + "\n";
		#else
		additionalCommand += "- Screen: Disabled\n";
		#endif
		
		// Networking configuration
		additionalCommand += "\nNetworking:\n";
		#if WIFI_ENABLED
		additionalCommand += "- WiFi: Enabled\n";
		additionalCommand += "  Access Point: SSID=\"" + String(WIFI_AP_SSID) + "\"\n";
		#else
		additionalCommand += "- WiFi: Disabled\n";
		#endif
		
		#if WEBSERVER_ENABLED
		additionalCommand += "- Web Server: Enabled on port " + String(WEBSERVER_PORT) + "\n";
		#else
		additionalCommand += "- Web Server: Disabled\n";
		#endif
		
		#if WEBSOCKET_ENABLED
		additionalCommand += "- WebSocket: Enabled (for real-time communication)\n";
		#else
		additionalCommand += "- WebSocket: Disabled\n";
		#endif
		
		// Add tips for responding to the robot
		additionalCommand += "\n=== RESPONSE GUIDELINES ===\n";
		additionalCommand += "1. Format your commands using exact syntax: [COMMAND] or [COMMAND=duration]\n";
		additionalCommand += "   - Duration format examples: 5s, 10s, 1m (minimum 3 seconds)\n";
		additionalCommand += "2. Available face expressions: [FACE_NORMAL], [FACE_HAPPY], [FACE_SAD], [FACE_ANGRY], [FACE_SURPRISED], \n";
		additionalCommand += "   [FACE_WORRIED], [FACE_FOCUSED], [FACE_ANNOYED], [FACE_SKEPTIC], [FACE_FRUSTRATED], [FACE_UNIMPRESSED],\n";
		additionalCommand += "   [FACE_SLEEPY], [FACE_SUSPICIOUS], [FACE_SQUINT], [FACE_FURIOUS], [FACE_SCARED], [FACE_AWE], [FACE_GLEE]\n";
		additionalCommand += "3. Look direction commands: [LOOK_LEFT], [LOOK_RIGHT], [LOOK_FRONT], [LOOK_TOP], [LOOK_BOTTOM], [BLINK]\n";
		additionalCommand += "4. Movement commands: [MOVE_FORWARD=5s], [MOVE_BACKWARD=5s], [TURN_LEFT=3s], [TURN_RIGHT=3s], [STOP]\n";
		additionalCommand += "5. Servo commands: [HEAD_UP], [HEAD_DOWN], [HEAD_CENTER], [ARM_OPEN], [ARM_CLOSE], [ARM_CENTER]\n";
		additionalCommand += "6. Consider sensor readings when responding (avoid cliffs, obstacles, etc)\n";
		additionalCommand += "7. Be concise but helpful in your responses\n";
		additionalCommand += "8. If asked about hardware capabilities, use this context to provide accurate information\n\n";
		
		// Send the prompt with enhanced context
		gptAdapter->sendPrompt(data->prompt, additionalCommand, [data](const String& gptResponse){
			// Process commands in the response if CommandMapper is available
			String processedResponse = gptResponse;
			
			if (commandMapper != nullptr) {
				// Extract and execute any commands in the response
				logger->debug("Processing commands in GPT response");
				int commandCount = commandMapper->executeCommandString(gptResponse);
				
				if (commandCount > 0) {
					logger->debug("Executed " + String(commandCount) + " commands from GPT response");
					
					// Get just the text without commands
					processedResponse = commandMapper->extractText(gptResponse);
				}
			}

			// Call the callback with the processed response
			data->callback(processedResponse);

			if (screen){
				screen->mutexClear();
				screen->drawCenteredText(20, processedResponse);
				screen->mutexUpdate();
			}
			
			// Log the interaction if requested
			if (data->saveToLog) {
				logGPTInteraction(data->prompt, gptResponse);
			}
		});
	}

	gptTaskHandle = nullptr;
	vTaskDelete(NULL);
}

void sendGPT(const String &prompt, Communication::GPTAdapter::ResponseCallback callback){
	gptRequest *data = new gptRequest{
		prompt: prompt,
		callback: callback,
		saveToLog: GPT_LEARNING_ENABLED
	};

	xTaskCreate(gptChatTask, "gptChatTask", 20 * 1024, data, 10, &gptTaskHandle);
}

// Clear learning data (useful for testing or resetting learning)
bool clearGPTLearningData() {
  #if GPT_LEARNING_ENABLED
  static Utils::FileManager fileManager;
  if (!fileManager.init()) {
    logger->error("Failed to initialize FileManager for GPT data clearing");
    return false;
  }
  
  if (!fileManager.exists(GPT_DATA_LOG_PATH)) {
    return true; // File doesn't exist, consider it "cleared"
  }
  
  bool success = fileManager.deleteFile(GPT_DATA_LOG_PATH);
  if (success) {
    logger->info("GPT learning data cleared");
  }
  return success;
  #else
  return false;
  #endif
}