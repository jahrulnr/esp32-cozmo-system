#include "routes.h"

void registerWebSocketRoutes(Router* router) {
	router->websocket("/ws")
		.onConnect([](WebSocketRequest& request) {
		  uint32_t clientId = request.clientId();
			Serial.printf("[WebSocket] client %u connected\n", request.clientId());
			String ip = request.clientIP();
			logger->info("WebSocket client #" + String(clientId) + " connected from " + ip);
			sessions[clientId % 5].authenticated = false;
			
			// Send welcome message
			JsonDocument welcome;
			welcome["type"] = "welcome";
			welcome["message"] = "Connected websocket";
			
			String welcomeMsg;
			serializeJson(welcome, welcomeMsg);
			request.send(welcomeMsg);
		})
		.onDisconnect([](WebSocketRequest& request) {
		  uint32_t clientId = request.clientId();
      logger->info("WebSocket client #%d disconnected", clientId);
      // Clean up session data
      sessions[clientId % 5].authenticated = false;

      // Clean up any file upload state
      if (fileUploads.count(clientId) > 0) {
        if (fileUploads[clientId].inProgress) {
          logger->warning("Client #" + String(clientId) + " disconnected during file upload");
        }
        fileUploads.erase(clientId);
      }

      // Unsubscribe from camera frames
      if (webSocket) {
        webSocket->setCameraSubscription(clientId, false);

        // If no more clients want camera streams, stop it
        if (!webSocket->hasClientsForCameraFrames() && isCameraStreaming()) {
          stopCameraStreaming();
          logger->info("Camera streaming stopped (no more subscribers after disconnect)");
        }
      }
		})
		.onMessage([](WebSocketRequest& request, const String& message) {
			JsonDocument doc;
			DeserializationError error = deserializeJson(doc, message);
		  uint32_t clientId = request.clientId();
			
			if (error) {
				Serial.println("[WebSocket] Invalid JSON received");
				return;
			}

			String type = doc["type"].as<String>();
			JsonVariant data = doc["data"];
			String version = doc["version"] | "0.0";

			if (type == "login") {
			}
			else if (sessions[clientId % 5].authenticated) {
				bool apOnlyMode = isApOnlyMode();
				if (apOnlyMode &&
						type != "system_status" &&
						type != "get_wifi_networks" &&
						type != "get_wifi_config" &&
						type != "update_wifi_config" &&
						type != "connect_wifi") {
					// Send a friendly message that the command is restricted in AP-only mode
					webSocket->sendError(clientId, 403, "Function restricted in AP mode");
					return;
				}
				if (type == "system_status" || type == "get_status") {
					Utils::SpiJsonDocument statusData;

					// WiFi status with more detailed information
					if (wifiManager) {
						bool connected = wifiManager->isConnected();
						statusData["wifi"] = connected;
						statusData["wifi_mode"] = isApOnlyMode() ? "ap" : "station";

						if (connected) {
							statusData["ip"] = wifiManager->getIP();
							statusData["rssi"] = wifiManager->getRSSI();
						}

						// Include the AP info if in AP mode
						if (isApOnlyMode()) {
							Communication::WiFiManager::WiFiConfig config = wifiManager->getConfig();
							statusData["ap_ssid"] = config.apSsid;
						}
					}

					// Other system statuses
					statusData["battery"] = -1; // Sample data
					statusData["memory"] = String(ESP.getFreeHeap() / 1024) + " KB";  // Actual free memory in KB
					statusData["cpu"] = String(ESP.getCpuFreqMHz()) + "Mhz";
					statusData["spiffs_total"] = String(SPIFFS.totalBytes() / 1024) + " KB"; // KB
					statusData["spiffs_used"] = String((SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024) + " KB"; // KB
					statusData["temperature"] = temperatureSensor->readTemperature();
		#if MICROPHONE_I2S
					statusData["microphone"]["enabled"] = amicrophone != nullptr;
					if (amicrophone && amicrophone->isInitialized()) {
						statusData["microphone"]["level"] = amicrophone->readLevel();
					}
		#elif MICROPHONE_ANALOG
					statusData["microphone"]["enabled"] = amicrophone != nullptr;
					if (amicrophone && amicrophone->isInitialized()) {
						statusData["microphone"]["level"] = amicrophone->readLevel();
					}
		#endif
					statusData["speaker"]["enabled"] = getSpeakerStatus();
					statusData["speaker"]["type"] = getSpeakerType();
					statusData["speaker"]["playing"] = isSpeakerPlaying();
					statusData["uptime"] = millis() / 1000;

					webSocket->sendJsonMessage(clientId, "system_status", statusData);
				}
				// Get storage information
				else if (type == "storage_info") {
					Utils::SpiJsonDocument storageData;

					// Get SPIFFS information
					size_t totalBytes = SPIFFS.totalBytes();
					size_t usedBytes = SPIFFS.usedBytes();
					size_t freeBytes = totalBytes - usedBytes;
					float percentUsed = (float)usedBytes / (float)totalBytes * 100.0;

					storageData["total"] = totalBytes;
					storageData["used"] = usedBytes;
					storageData["free"] = freeBytes;
					storageData["percent"] = percentUsed;

					webSocket->sendJsonMessage(clientId, "storage_info", storageData);
					logger->debug("Sent storage information to client #" + String(clientId));
				}
				// Get storage status for specific storage type
				else if (type == "get_storage_status") {
					String storageType = data["storage_type"] | "STORAGE_SPIFFS";
					
					Utils::SpiJsonDocument statusData;
					statusData["storage_type"] = storageType;
					
					if (storageType == "STORAGE_SPIFFS") {
						statusData["available"] = true;
						statusData["status"] = "Connected";
						statusData["type"] = "Internal Flash";
					} else if (storageType == "STORAGE_SD_MMC") {
						// Initialize FileManager to check SD/MMC availability
						static Utils::FileManager fileManager;
						bool sdAvailable = fileManager.isSDMMCAvailable();
						
						statusData["available"] = sdAvailable;
						statusData["status"] = sdAvailable ? "Connected" : "Not Available";
						statusData["type"] = "SD/MMC Card";
					}
					
					webSocket->sendJsonMessage(clientId, "storage_status", statusData);
				}
				// Camera control
				else if (type == "camera_command") {
					String action = data["action"] | "";

					if (action == "start" && camera) {
						// Set streaming interval if provided
						if (!data["interval"].isUnbound()) {
							uint32_t interval = data["interval"] | 33;
							camera->setStreamingInterval(interval);
						}

						// Set resolution if provided
						if (!data["resolution"].isUnbound()) {
							String res = data["resolution"] | "vga";
							framesize_t resolution = FRAMESIZE_VGA; // Default

							if (res == "qvga") resolution = FRAMESIZE_QVGA;
							else if (res == "hd") resolution = FRAMESIZE_HD;
							else if (res == "sxga") resolution = FRAMESIZE_SXGA;
							else if (res == "uxga") resolution = FRAMESIZE_UXGA;

							camera->setResolution(resolution);
						}

						// Subscribe this client to camera frames
						webSocket->setCameraSubscription(clientId, true);

						startCameraStreaming();
						logger->info("Camera streaming started for client #" + String(clientId));
						webSocket->sendOk(clientId, "Camera streaming started");
					}
					else if (action == "stop" && camera) {
						// Unsubscribe this client from camera frames
						webSocket->setCameraSubscription(clientId, false);

						// Check if there are any clients still subscribed before stopping the camera
						if (!webSocket->hasClientsForCameraFrames()) {
							stopCameraStreaming();
							logger->info("Camera streaming stopped (no more subscribers)");
						} else {
							logger->info("Client #" + String(clientId) + " unsubscribed from camera stream");
						}
					}
				}
				// Motor 
				else if (type == "motor_command") {
					float left = data["left"] | 0.0;
					float right = data["right"] | 0.0;
					int duration = data["duration"] | 1000;
					String action = data["action"] | "";

					if (motors) {
						if (action == "reset") {
							// Handle reset command
							motors->stop();
							logger->debug("Motor reset command received");
						} else {
							// Determine direction based on motor values
							Motors::MotorControl::Direction direction;

							if (left > 0 && right > 0) {
								direction = Motors::MotorControl::FORWARD;
							} else if (left < 0 && right < 0) {
								direction = Motors::MotorControl::BACKWARD;
							} else if (left < 0 && right > 0) {
								direction = Motors::MotorControl::LEFT;
							} else if (left > 0 && right < 0) {
								direction = Motors::MotorControl::RIGHT;
							} else {
								direction = Motors::MotorControl::STOP;
							}

							// Update manual control time to pause automation
							updateManualControlTime();
							
							// Move motors in the determined direction with optional duration
							motors->move(direction, duration);
							logger->debug("Motor command - Left: " + String(left) + ", Right: " + String(right) +
														", Direction: " + String(direction) + ", Duration: " + String(duration));
						}

						// Send motor status response
						Utils::SpiJsonDocument statusData;
						statusData["left"] = left;
						statusData["right"] = right;
						webSocket->sendJsonMessage(clientId, "motor_status", statusData);
					} else {
						webSocket->sendError(clientId, 404, "Motor control not available");
					}
				}
				else if (type == "head_command") {
					float pan = data["pan"] | 90.0;
					float tilt = data["tilt"] | 90.0;

					if (servos) {
						// Update manual control time to pause automation
						updateManualControlTime();
						
						servos->setHand(pan);  // Set pan servo
						servos->setHead(tilt); // Set tilt servo

						logger->debug("Head command - Pan: " + String(pan) + ", Tilt: " + String(tilt));
						webSocket->sendOk(clientId, "Head position updated");
					} else {
						webSocket->sendError(clientId, 404, "Servo control not available");
					}
				}
				else if (type == "arm_command") {
					float position = data["position"] | 90.0;

					if (servos) {
						// Update manual control time to pause automation
						updateManualControlTime();
						
						servos->setHand(position);

						logger->debug("Arm command - Position: " + String(position));
						webSocket->sendOk(clientId, "Arm position updated");
					} else {
						webSocket->sendError(clientId, 404, "Servo control not available");
					}
				}
				else if (type == "orientation_request") {
					if (orientation) {
						orientation->update();

						Utils::SpiJsonDocument sensorData;
						sensorData["gyro"]["x"] = orientation->getX();
						sensorData["gyro"]["y"] = orientation->getY();
						sensorData["gyro"]["z"] = orientation->getZ();
						sensorData["accel"]["x"] = orientation->getAccelX();
						sensorData["accel"]["y"] = orientation->getAccelY();
						sensorData["accel"]["z"] = orientation->getAccelZ();
						sensorData["accel"]["magnitude"] = orientation->getAccelMagnitude();

						webSocket->sendJsonMessage(clientId, "sensor_data", sensorData);
					} else {
						webSocket->sendError(clientId, 404, "Gyroscope not available");
					}
				}
				else if (type == "distance_request") {
					if (distanceSensor) {
						float distance = distanceSensor->measureDistance();

						Utils::SpiJsonDocument sensorData;
						sensorData["distance"]["value"] = distance;
						sensorData["distance"]["unit"] = "cm";
						sensorData["distance"]["valid"] = (distance >= 0);
						sensorData["distance"]["obstacle"] = distanceSensor->isObstacleDetected();

						webSocket->sendJsonMessage(clientId, "sensor_data", sensorData);
					} else {
						webSocket->sendError(clientId, 404, "Distance sensor not available");
					}
				}
				else if (type == "microphone_request") {
		#if MICROPHONE_I2S
					if (microphone && microphone->isInitialized()) {
						Utils::SpiJsonDocument sensorData;
						sensorData["microphone"]["level"] = microphone->readLevel();
						sensorData["microphone"]["peak"] = microphone->readLevel();
		#elif MICROPHONE_ANALOG
					if (amicrophone && amicrophone->isInitialized()) {
						Utils::SpiJsonDocument sensorData;
						sensorData["microphone"]["level"] = amicrophone->readLevel();
						sensorData["microphone"]["peak"] = amicrophone->readPeakLevel();
		#endif
						webSocket->sendJsonMessage(clientId, "sensor_data", sensorData);
					} else {
						webSocket->sendError(clientId, 404, "Microphone sensor not available");
					}
				}
				else if (type == "speaker_control") {
					String action = data["action"] | "";
					int volume = data["volume"] | 50;
					
					if (action == "beep") {
						playSpeakerBeep(volume);
						webSocket->sendOk(clientId, "Speaker beep played");
					} else if (action == "confirm") {
						playSpeakerConfirmation(volume);
						webSocket->sendOk(clientId, "Confirmation sound played");
					} else if (action == "error") {
						playSpeakerError(volume);
						webSocket->sendOk(clientId, "Error sound played");
					} else if (action == "notify") {
						playSpeakerNotification(volume);
						webSocket->sendOk(clientId, "Notification sound played");
					} else if (action == "tone") {
						int frequency = data["frequency"] | 1000;
						int duration = data["duration"] | 500;
						playSpeakerTone(frequency, duration, volume);
						webSocket->sendOk(clientId, "Tone played");
					} else if (action == "volume") {
						setSpeakerVolume(volume);
						webSocket->sendOk(clientId, "Volume set to " + String(volume));
					} else {
						webSocket->sendError(clientId, 400, "Unknown speaker action: " + action);
					}
				}
				else if (type == "servo_update" && servos) {
					String servoType = data["type"] | "";
					int position = data["position"] | 0;

					// Update manual control time to pause automation
					updateManualControlTime();

					if (servoType == "head") {
						int servoY = map(position, -100, 100, 0, 180);

						servos->setHead(servoY);
						position = servos->getHead();

						logger->debug("Servo Y: " + String(servoY));
						webSocket->sendOk(clientId, "Servo updated. Y="+String(position));
					} 
					else if(servoType == "hand")  {
						int servoX = map(position, -100, 100, 0, 180);
						servos->setHand(servoX);
						position = servos->getHand();

						logger->debug("Servo X: " + String(servoX));
						webSocket->sendOk(clientId, "Servo updated. X="+String(position));
					}
				}
				// Legacy joystick update (convert to appropriate DTO commands)
				else if (type == "joystick_update") {
					String joyType = data["type"] | "";
					int x = data["x"] | 0;
					int y = data["y"] | 0;

					if (joyType == "motor" && motors) {
						// Determine direction based on joystick input
						Motors::MotorControl::Direction direction;
						int directionValue = 0; // For status reporting

						// Use joystick position to determine direction
						if (y > 20) {
							direction = Motors::MotorControl::FORWARD;
							directionValue = 1;
						}
						else if (y < -20) {
							direction = Motors::MotorControl::BACKWARD;
							directionValue = -1;
						}
						else if (x < -20) {
							direction = Motors::MotorControl::LEFT;
							directionValue = 2;
						}
						else if (x > 20) {
							direction = Motors::MotorControl::RIGHT;
							directionValue = 3;
						}
						else {
							direction = Motors::MotorControl::STOP;
							directionValue = 0;
						}

						// Update manual control time to pause automation
						updateManualControlTime();
						
						// Move the motors in the determined direction
						motors->move(direction);

						// Calculate magnitude for status response (for UI feedback)
						int magnitude = sqrt(x*x + y*y);
						magnitude = constrain(magnitude, 0, 100);

						logger->debug("Motors direction: " + String(directionValue) + ", magnitude: " + String(magnitude));

						// Send motor status response
						Utils::SpiJsonDocument statusData;
						statusData["direction"] = directionValue;
						statusData["magnitude"] = magnitude / 100.0; // Normalize to 0.0 to 1.0
						statusData["left"] = (direction == Motors::MotorControl::LEFT || direction == Motors::MotorControl::BACKWARD) ? -magnitude / 100.0 : magnitude / 100.0;
						statusData["right"] = (direction == Motors::MotorControl::RIGHT || direction == Motors::MotorControl::BACKWARD) ? -magnitude / 100.0 : magnitude / 100.0;
						webSocket->sendJsonMessage(clientId, "motor_status", statusData);
					}
				}
				// Set automation state
				else if (type == "automation_control") {
					bool enabled = data["enabled"] | _enableAutomation;  // Default to current state if not specified
					
					// Update automation state
					setAutomationEnabled(enabled);
					
					// Get current status for response
					Utils::SpiJsonDocument statusData;
					statusData["enabled"] = isAutomationEnabled();
					webSocket->sendJsonMessage(clientId, "automation_status", statusData);
					
					logger->info("Automation " + String(enabled ? "enabled" : "disabled") + " by client #" + String(clientId));
				}
				// Get automation status
				else if (type == "get_automation_status") {
					// Get current status for response
					Utils::SpiJsonDocument statusData;
					statusData["enabled"] = isAutomationEnabled();
					webSocket->sendJsonMessage(clientId, "automation_status", statusData);
				}
				// Get WiFi networks
				else if (type == "get_wifi_networks") {
					if (wifiManager) {
						Utils::SpiJsonDocument networksData;
						JsonArray networks = networksData.to<JsonArray>();

						int numNetworks = WiFi.scanNetworks();
						for (int i = 0; i < numNetworks; i++) {
							JsonObject network = networks.add<JsonObject>();
							network["ssid"] = WiFi.SSID(i);
							network["rssi"] = WiFi.RSSI(i);
							network["encryption"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
						}

						webSocket->sendJsonMessage(clientId, "wifi_list", networksData);
					}
				}
				// Get current WiFi configuration
				else if (type == "get_wifi_config") {
					if (wifiManager) {
						Communication::WiFiManager::WiFiConfig config = wifiManager->getConfig();

						Utils::SpiJsonDocument configData;
						configData["ssid"] = config.ssid;
						// Not sending password for security reasons, just indicate if it's set
						configData["password_set"] = config.password.length() > 0;
						configData["ap_ssid"] = config.apSsid;
						configData["ap_password_set"] = config.apPassword.length() > 0;
						configData["is_ap_mode"] = isApOnlyMode();
						configData["connected"] = wifiManager->isConnected();

						webSocket->sendJsonMessage(clientId, "wifi_config", configData);
					}
				}
				// Update WiFi configuration
				else if (type == "update_wifi_config") {
					if (wifiManager) {
						// Get current config as a base
						Communication::WiFiManager::WiFiConfig config = wifiManager->getConfig();

						// Update with new values if provided
						if (!data["ssid"].isUnbound()) {
							config.ssid = data["ssid"].as<String>();
						}

						if (!data["password"].isUnbound()) {
							config.password = data["password"].as<String>();
						}

						if (!data["ap_ssid"].isUnbound()) {
							config.apSsid = data["ap_ssid"].as<String>();
						}

						if (!data["ap_password"].isUnbound()) {
							config.apPassword = data["ap_password"].as<String>();
						}

						// Save the new configuration
						bool saved = wifiManager->updateConfig(config);

						// Respond with success status
						Utils::SpiJsonDocument response;
						response["success"] = saved;
						response["message"] = saved ? "WiFi configuration saved" : "Failed to save WiFi configuration";

						webSocket->sendJsonMessage(clientId, "wifi_config_update", response);

						// If told to reconnect with new settings
						if (data["reconnect"] | false) {
							// Send notification that we'll be reconnecting
							Utils::SpiJsonDocument notificationData;
							notificationData["message"] = "Reconnecting with new WiFi settings...";
							webSocket->sendJsonMessage(clientId, "notification", notificationData);

							// Small delay to allow message to be sent
							delay(500);

							// Reconnect with new settings
							if (wifiManager->connect(config.ssid, config.password)) {
								logger->info("Reconnected to WiFi with new settings: " + config.ssid);
								g_isApOnlyMode = false;
							} else {
								logger->warning("Failed to connect with new settings, starting AP mode");
								wifiManager->startAP(config.apSsid, config.apPassword);
								g_isApOnlyMode = true;
							}
						}
					}
				}
				// Connect to WiFi
				else if (type == "connect_wifi") {
					String ssid = data["ssid"] | "";
					String password = data["password"] | "";

					if (wifiManager && ssid.length() > 0) {
						// Get current config as a base
						Communication::WiFiManager::WiFiConfig config = wifiManager->getConfig();

						// Update with new values
						config.ssid = ssid;
						config.password = password;

						// Try to connect
						bool connected = wifiManager->connect(ssid, password);

						// Update AP-only mode flag
						g_isApOnlyMode = !connected;

						// Save the configuration if connection was successful
						if (connected) {
							wifiManager->updateConfig(config);
						}

						Utils::SpiJsonDocument response;
						response["success"] = connected;
						response["message"] = connected ? "Connected to " + ssid : "Failed to connect to " + ssid;
						response["is_ap_mode"] = isApOnlyMode();

						webSocket->sendJsonMessage(clientId, "wifi_connection", response);
					}
				}
				// File operations
				else if (type == "list_files") {
					String path = data["path"] | "/";
					String storageType = data["storage_type"] | "STORAGE_SPIFFS";

					Utils::SpiJsonDocument filesData;
					JsonArray files = filesData.to<JsonArray>();

					// Convert string storage type to enum
					Utils::FileManager::StorageType storage = Utils::FileManager::STORAGE_SPIFFS;
					if (storageType.equals("STORAGE_SD_MMC")) {
						storage = Utils::FileManager::STORAGE_SD_MMC;
					}

					if (fileManager) {
						// Get files using the FileManager with specified storage
						std::vector<Utils::FileManager::FileInfo> fileList = fileManager->listFiles(path, storage);
						Serial.println("Files in SPIFFS root directory:");
						for (const auto& file : fileList) {
								Serial.print("  ");
								Serial.print(file.name);
								if (file.isDirectory) {
										Serial.println("/");
								} else {
										Serial.print(" (");
										Serial.print(file.size);
										Serial.println(" bytes)");
								}
							JsonObject fileObj = files.add<JsonObject>();
							fileObj["name"] = file.name;
							fileObj["path"] = file.dir;
							fileObj["size"] = file.size;
							fileObj["type"] = file.isDirectory ? "directory" : "file";
						}
					}

					// Add storage info to response
					filesData["files"] = files;
					filesData["storage_type"] = storageType;
					filesData["path"] = path;

					webSocket->sendJsonMessage(clientId, "list_files", filesData);
				}
				// Delete file
				else if (type == "delete_file") {
					String path = data["path"] | "";
					String storageType = data["storage_type"] | "STORAGE_SPIFFS";

					// Use FileManager for file operations
					static Utils::FileManager fileManager;
					if (!fileManager.init()) {
						logger->error("Failed to initialize FileManager");
						webSocket->sendError(clientId, 500, "Failed to initialize file system");
						return;
					}

					bool success = false;
					if (path.length() > 0) {
						// Convert string storage type to enum
						Utils::FileManager::StorageType storage = Utils::FileManager::STORAGE_SPIFFS;
						if (storageType == "STORAGE_SD_MMC") {
							storage = Utils::FileManager::STORAGE_SD_MMC;
						}
						
						success = fileManager.deleteFile(path, storage);
						logger->info("File delete " + String(success ? "successful" : "failed") + 
													": " + path + " from " + storageType);
					}

					Utils::SpiJsonDocument response;
					response["success"] = success;
					response["message"] = success ? "File deleted" : "Failed to delete file";
					response["path"] = path;
					response["storage_type"] = storageType;

					webSocket->sendJsonMessage(clientId, "file_operation", response);
				}
				// Read file content
				else if (type == "read_file") {
					String path = data["path"] | "";

					if (path.length() > 0) {
						// Use FileManager for file operations
						static Utils::FileManager fileManager;
						if (!fileManager.init()) {
							logger->error("Failed to initialize FileManager for file reading");
							webSocket->sendError(clientId, 500, "Failed to initialize file system");
							return;
						}

						// Read file content
						String content = fileManager.readFile(path);

						// Check if file was read successfully
						if (content.length() > 0 || SPIFFS.exists(path)) {
							logger->info("File read: " + path + " (" + String(content.length()) + " bytes)");

							// Send file content to client
							Utils::SpiJsonDocument response;
							response["path"] = path;
							response["content"] = content;
							response["size"] = content.length();
							response["success"] = true;

							// Get file extension for content type hint
							String extension = "";
							int dotIndex = path.lastIndexOf('.');
							if (dotIndex >= 0) {
								extension = path.substring(dotIndex + 1);
								extension.toLowerCase();
							}
							response["type"] = extension;

							webSocket->sendJsonMessage(clientId, "file_content", response);
						} else {
							logger->error("Failed to read file: " + path);
							webSocket->sendError(clientId, 404, "File not found or empty");
						}
					} else {
						webSocket->sendError(clientId, 400, "Missing file path");
					}
				}
				else if (type == "upload_file") {
					String path = data["path"] | "/";
					String name = data["name"] | "";
					String fileData = data["data"] | "";
					size_t size = data["size"] | 0;

					// Initialize FileManager for file operations
					static Utils::FileManager fileManager;
					if (!fileManager.init()) {
						logger->error("Failed to initialize FileManager");
						webSocket->sendError(clientId, 500, "Failed to initialize file system");
						return;
					}

					// Use the global fileUploads map for state tracking
					bool success = false;

					if (name.length() > 0) {
						String filePath = path;
						if (!filePath.endsWith("/")) filePath += "/";
						filePath += name;

						// For small text files, allow direct upload via JSON
						if (fileData.length() > 0) {
							// Use FileManager to write the file
							success = fileManager.writeFile(filePath, fileData);

							if (success) {
								logger->info("Text file uploaded directly: " + filePath);
							} else {
								logger->error("Failed to write text file: " + filePath);
							}
						} else {
							// For binary files, prepare to receive binary data
							// Store upload state for this client
							fileUploads[clientId] = {path, name, size, true};

							// Create an empty file using FileManager
							success = fileManager.writeFile(filePath, "");

							if (success) {
								logger->info("Prepared for binary file upload from client #" + String(clientId) +
														": " + filePath + " (" + String(size) + " bytes)");

								Utils::SpiJsonDocument response;
								response["success"] = true;
								response["message"] = "Ready for binary upload";
								response["path"] = filePath;
								response["expecting_binary"] = true;

								webSocket->sendJsonMessage(clientId, "file_operation", response);
								return; // Don't send the final response yet
							} else {
								logger->error("Failed to create file for binary upload: " + filePath);
							}
						}
					}

					// Only send response for text uploads or failed preparations
					Utils::SpiJsonDocument response;
					response["success"] = success;
					response["message"] = success ? "File uploaded" : "Failed to upload file";

					webSocket->sendJsonMessage(clientId, "file_operation", response);
				}
				// Create folder
				else if (type == "create_folder") {
					String path = data["path"] | "/";
					String name = data["name"] | "";

					if (path.length() > 0 && name.length() > 0) {
						// Use FileManager for file operations
						static Utils::FileManager fileManager;
						if (!fileManager.init()) {
							logger->error("Failed to initialize FileManager");
							webSocket->sendError(clientId, 500, "Failed to initialize file system");
							return;
						}

						// Construct the full folder path
						String folderPath = path;
						if (!folderPath.endsWith("/")) folderPath += "/";
						folderPath += name;

						// Create the directory
						bool success = SPIFFS.mkdir(folderPath);
						logger->info("Folder creation " + String(success ? "successful" : "failed") + ": " + folderPath);

						Utils::SpiJsonDocument response;
						response["success"] = success;
						response["message"] = success ? "Folder created" : "Failed to create folder";
						response["path"] = folderPath;

						webSocket->sendJsonMessage(clientId, "file_operation", response);

						if (success) {
							static Utils::FileManager fileManager;
							if (!fileManager.init()) {
								logger->error("Failed to initialize FileManager for fetchFileList");
								webSocket->sendError(clientId, 500, "Failed to initialize file system");
								return;
							}

							Utils::SpiJsonDocument filesData;
							JsonArray files = filesData.to<JsonArray>();

							std::vector<Utils::FileManager::FileInfo> fileList = fileManager.listFiles(path);
							for (const auto& file : fileList) {
								JsonObject fileObj = files.add<JsonObject>();
								fileObj["name"] = file.name;
								fileObj["size"] = file.size;
								fileObj["type"] = file.isDirectory ? "directory" : "file";
							}

							webSocket->sendJsonMessage(clientId, "list_files", filesData);
						}
					} else {
						webSocket->sendError(clientId, 400, "Missing path or folder name");
					}
				}
				// Chat message
				else if (type == "send_chat") {
					String content = data["content"] | "";
					#if GPT_ENABLED
					if (content.length() > 0 && gptAdapter) {
						// Process with GPTAdapter
						sendGPT(content, [clientId](const String& gptResponse) {
							Utils::SpiJsonDocument response;
							response["sender"] = "Cozmo";
							response["content"] = gptResponse;
							response["timestamp"] = String(millis() / 1000);
							webSocket->sendJsonMessage(clientId, "chat_message", response);
						});
					} else {
						Utils::SpiJsonDocument response;
						response["sender"] = "System";
						response["content"] = "Error: Empty message or GPT not available.";
						response["timestamp"] = String(millis() / 1000);
						webSocket->sendJsonMessage(clientId, "chat_message", response);
					}
					#else
					if (content.length() > 0) {
						// Echo message back to client (legacy fallback)
						Utils::SpiJsonDocument response;
						response["sender"] = "System";
						response["content"] = "Received: " + content;
						response["timestamp"] = String(millis() / 1000);
						webSocket->sendJsonMessage(clientId, "chat_message", response);
					}
					#endif
				}
				// Debug command
				else if (type == "debug_command") {
					String cmd = data["command"] | "";
					logger->debug("Debug command received: " + cmd);

					Utils::SpiJsonDocument response;
					response["message"] = "Command executed: " + cmd;
					response["level"] = "info";

					webSocket->sendJsonMessage(clientId, "log_message", response);
				}
				// Execute text command
				else if (type == "execute_command") {
					String cmdText = data["command"] | "";

					if (cmdText.length() > 0 && commandMapper != nullptr) {
						logger->debug("Processing text command: " + cmdText);

						// Process the command text
						String resultText = processTextCommands(cmdText);

						Utils::SpiJsonDocument response;
						response["success"] = true;
						response["originalText"] = cmdText;
						response["resultText"] = resultText;
						response["containedCommands"] = (resultText != cmdText);

						webSocket->sendJsonMessage(clientId, "command_executed", response);
						logger->info("Text command executed: " + cmdText);
					} else {
						Utils::SpiJsonDocument response;
						response["success"] = false;
						response["message"] = "Empty command or CommandMapper not initialized";

						webSocket->sendJsonMessage(clientId, "command_executed", response);
					}
				}
			} else {
				// Not authenticated
				webSocket->sendError(clientId, 401, "Authentication required");
			}
		})
		.onBinary([](WebSocketRequest& request, uint8_t* data, size_t len){
		  uint32_t clientId = request.clientId();
			if (sessions[clientId % 5].authenticated) {
				static Utils::FileManager fileManager;
				if (!fileManager.init()) {
					logger->error("Failed to initialize FileManager");
					webSocket->sendError(clientId, 500, "Failed to initialize file system");
					return;
				}

				// Check if this client has a pending file upload
				if (fileUploads.count(clientId) > 0 && fileUploads[clientId].inProgress) {
					String filePath = fileUploads[clientId].path;
					if (!filePath.endsWith("/")) filePath += "/";
					filePath += fileUploads[clientId].name;

					logger->info("Writing binary data to " + filePath + " for client #" + String(clientId));

					// Since FileManager doesn't have a direct method for writing binary data,
					// we'll fall back to using SPIFFS directly for this specific case
					File file = SPIFFS.open(filePath, FILE_WRITE);
					if (file) {
						file.write(data, len);
						file.close();

						Utils::SpiJsonDocument response;
						response["success"] = true;
						response["message"] = "File uploaded successfully";
						response["path"] = filePath;
						response["name"] = fileUploads[clientId].name;

						webSocket->sendJsonMessage(clientId, "file_operation", response);

						// Clear upload state
						fileUploads[clientId].inProgress = false;
					} else {
						Utils::SpiJsonDocument response;
						response["success"] = false;
						response["message"] = "Failed to open file for writing";

						webSocket->sendJsonMessage(clientId, "file_operation", response);
						fileUploads[clientId].inProgress = false;
					}
				} else {
					// Default behavior for backward compatibility - assume it's a file upload
					static String uploadPath = "/";
					static String uploadName = "upload.bin";

					String filePath = uploadPath;
					if (!filePath.endsWith("/")) filePath += "/";
					filePath += uploadName;

					logger->warning("Received binary data without file upload context from client #" + String(clientId) +
												", saving to default path: " + filePath);

					// Since FileManager doesn't have a direct method for writing binary data,
					// we'll fall back to using SPIFFS directly for this specific case
					File file = SPIFFS.open(filePath, FILE_WRITE);
					if (file) {
						file.write(data, len);
						file.close();

						Utils::SpiJsonDocument response;
						response["success"] = true;
						response["message"] = "File uploaded successfully (using default path)";
						response["path"] = filePath;

						webSocket->sendJsonMessage(clientId, "file_operation", response);
					} else {
						webSocket->sendError(clientId, 500, "Failed to open file for writing");
					}
				}
			} else {
				webSocket->sendError(clientId, 401, "Authentication required");
			}
		});
}
