#include <Arduino.h>
#include "init.h"
#include <ArduinoJson.h>

// Authentication credentials (should be moved to config file)
const char* AUTH_USERNAME = "admin";
const char* AUTH_PASSWORD = "admin";

// Current session data
struct {
  bool authenticated = false;
} sessions[5]; // Support up to 5 concurrent sessions

// File upload state tracking
struct FileUploadState {
  String path;
  String name;
  size_t size;
  bool inProgress;
};
std::map<uint32_t, FileUploadState> fileUploads;

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

void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                         AwsEventType type, void* arg, uint8_t* data, size_t len) {
  uint32_t clientId = client->id();
  
  switch (type) {
    case WS_EVT_DISCONNECT:
      logger->info("WebSocket client #" + String(clientId) + " disconnected");
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
      break;
      
    case WS_EVT_CONNECT:
      {
        IPAddress ip = client->remoteIP();
        logger->info("WebSocket client #" + String(clientId) + " connected from " + ip.toString());
        sessions[clientId % 5].authenticated = false;
      }
      break;
      
    case WS_EVT_DATA:
      {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        
        // Only process complete text messages
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
          // Parse JSON message
          Utils::SpiJsonDocument doc = Communication::WebSocketHandler::parseJsonMessage(data, len);
          
          if (!doc.isNull()) {
            String type = doc["type"].as<String>();
            JsonVariant data = doc["data"];
            logger->debug("Received DTO message type: " + type + " from client #" + String(clientId));
            
            // Login command doesn't require authentication
            if (type == "login") {
              String username = data["username"] | "";
              String password = data["password"] | "";
              String token = data["token"] | "";
              
              // Check if this is a token-based authentication (for persistent login)
              bool success = false;
              
              if (token.length() > 0 && password == "AUTO_LOGIN_TOKEN") {
                // This is a token-based re-authentication
                // In a production system, you'd validate the token here
                // For simplicity, we're accepting any token with the username matching our allowed user
                success = (username == AUTH_USERNAME);
                if (success) {
                  logger->info("User auto-authenticated with token: " + username);
                }
              } else {
                // Regular password login
                success = (username == AUTH_USERNAME && password == AUTH_PASSWORD);
              }
              
              if (success) {
                sessions[clientId % 5].authenticated = true;
                
                Utils::SpiJsonDocument response;
                response["success"] = true;
                response["token"] = "auth_token_" + username + "_" + String(millis()); // Generate a simple token
                
                webSocket->sendJsonMessage(clientId, "login_response", response);
                logger->info("User logged in: " + username);
              } else {
                Utils::SpiJsonDocument response;
                response["success"] = false;
                response["message"] = "Invalid username or password";
                
                webSocket->sendJsonMessage(clientId, "login_response", response);
                logger->warning("Failed login attempt for user: " + username);
              }
            }
            // Check authentication for all other commands
            else if (sessions[clientId % 5].authenticated) {
              // Get AP-only status at the start of processing commands
              bool apOnlyMode = isApOnlyMode();
              
              // If we're in AP-only mode, allow only WiFi related commands and basic status
              if (apOnlyMode && 
                  type != "system_status" && 
                  type != "get_wifi_networks" && 
                  type != "get_wifi_config" && 
                  type != "update_wifi_config" && 
                  type != "connect_wifi") {
                // Send a friendly message that the command is restricted in AP-only mode
                webSocket->sendError(clientId, 403, "Function restricted in AP mode");
                break;
              }
              
              // System status request
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
                statusData["battery"] = 82; // Sample data
                statusData["memory"] = ESP.getFreeHeap() / 1024;  // Actual free memory in KB
                statusData["cpu"] = 32;     // Sample data
                statusData["spiffs_total"] = SPIFFS.totalBytes() / 1024; // KB
                statusData["spiffs_used"] = (SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024; // KB
                statusData["temperature"] = 42.3; // Sample data
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
              // Camera control
              else if (type == "camera_command") {
                String action = data["action"] | "";
                
                if (action == "start" && camera) {
                  // Set streaming interval if provided
                  if (!data["interval"].isUnbound()) {
                    uint32_t interval = data["interval"] | 200;
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
              // Motor and servo control
              else if (type == "motor_command") {
                float left = data["left"] | 0.0;
                float right = data["right"] | 0.0;
                int duration = data["duration"] | 1000;
                
                if (motors) {
                  // Set motor speeds with duration
                  // motors->setSpeed(left, right, duration);
                  logger->debug("Motor command - Left: " + String(left) + ", Right: " + String(right) + ", Duration: " + String(duration));
                  
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
                  servos->setHand(position);
                  
                  logger->debug("Arm command - Position: " + String(position));
                  webSocket->sendOk(clientId, "Arm position updated");
                } else {
                  webSocket->sendError(clientId, 404, "Servo control not available");
                }
              }
              else if (type == "gyro_request") {
                if (gyro) {
                  gyro->update();
                  
                  Utils::SpiJsonDocument sensorData;
                  sensorData["gyro"]["x"] = gyro->getX();
                  sensorData["gyro"]["y"] = gyro->getY();
                  sensorData["gyro"]["z"] = gyro->getZ();
                  sensorData["accel"]["x"] = gyro->getAccelX();
                  sensorData["accel"]["y"] = gyro->getAccelY();
                  sensorData["accel"]["z"] = gyro->getAccelZ();
                  
                  // Calculate magnitude
                  float magnitude = sqrt(pow(gyro->getAccelX(), 2) + pow(gyro->getAccelY(), 2) + pow(gyro->getAccelZ(), 2));
                  sensorData["accel"]["magnitude"] = magnitude;
                  
                  webSocket->sendJsonMessage(clientId, "sensor_data", sensorData);
                } else {
                  webSocket->sendError(clientId, 404, "Gyroscope not available");
                }
              }
              // Legacy joystick update (convert to appropriate DTO commands)
              else if (type == "joystick_update") {
                String joyType = data["type"] | "";
                int x = data["x"] | 0;
                int y = data["y"] | 0;
                
                if (joyType == "servo" && servos) {
                  // Map joystick values (-100 to 100) to servo angles
                  int servoX = map(x, -100, 100, 0, 180);
                  int servoY = map(y, -100, 100, 0, 180);
                  
                  servos->setHand(servoX); // Assuming channel 0 is X
                  servos->setHead(servoY); // Assuming channel 1 is Y
                  
                  logger->debug("Servo X: " + String(servoX) + ", Y: " + String(servoY));
                  webSocket->sendOk(clientId, "Servo updated");
                } 
                else if (joyType == "motor" && motors) {
                  // Simple differential drive
                  int leftSpeed = y + x;
                  int rightSpeed = y - x;
                  
                  // Scale to motor values
                  leftSpeed = map(constrain(leftSpeed, -100, 100), -100, 100, -255, 255);
                  rightSpeed = map(constrain(rightSpeed, -100, 100), -100, 100, -255, 255);
                  
                  // Set motor speeds
                  // motors->setSpeed(leftSpeed);
                  // motors->setSpeed(rightSpeed);
                  
                  logger->debug("Motors L: " + String(leftSpeed) + ", R: " + String(rightSpeed));
                  
                  // Send motor status response
                  Utils::SpiJsonDocument statusData;
                  statusData["left"] = leftSpeed / 255.0; // Normalize to -1.0 to 1.0
                  statusData["right"] = rightSpeed / 255.0;
                  webSocket->sendJsonMessage(clientId, "motor_status", statusData);
                }
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
                
                // Use FileManager to list files
                static Utils::FileManager fileManager;
                if (!fileManager.init()) {
                  logger->error("Failed to initialize FileManager");
                  webSocket->sendError(clientId, 500, "Failed to initialize file system");
                  break;
                }
                
                Utils::SpiJsonDocument filesData;
                JsonArray files = filesData.to<JsonArray>();
                
                // Get files using the FileManager
                std::vector<Utils::FileManager::FileInfo> fileList = fileManager.listFiles(path);
                for (const auto& file : fileList) {
                  JsonObject fileObj = files.add<JsonObject>();
                  fileObj["name"] = file.name;
                  fileObj["size"] = file.size;
                  fileObj["type"] = file.isDirectory ? "directory" : "file";
                }
                
                webSocket->sendJsonMessage(clientId, "list_files", filesData);
              }
              // Delete file
              else if (type == "delete_file") {
                String path = data["path"] | "";
                
                // Use FileManager for file operations
                static Utils::FileManager fileManager;
                if (!fileManager.init()) {
                  logger->error("Failed to initialize FileManager");
                  webSocket->sendError(clientId, 500, "Failed to initialize file system");
                  break;
                }
                
                bool success = false;
                if (path.length() > 0) {
                  success = fileManager.deleteFile(path);
                  logger->info("File delete " + String(success ? "successful" : "failed") + ": " + path);
                }
                
                Utils::SpiJsonDocument response;
                response["success"] = success;
                response["message"] = success ? "File deleted" : "Failed to delete file";
                
                webSocket->sendJsonMessage(clientId, "file_operation", response);
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
                  break;
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
                    break;
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
                
                if (content.length() > 0) {
                  // Echo message back to client (in a real system, this would be processed)
                  Utils::SpiJsonDocument response;
                  response["sender"] = "System";
                  response["content"] = "Received: " + content;
                  response["timestamp"] = String(millis() / 1000);
                  
                  webSocket->sendJsonMessage(clientId, "chat_message", response);
                }
              }
              // Start sensor updates
              else if (type == "start_sensor_updates" && gyro) {
                Utils::SpiJsonDocument sensorData;
                gyro->update();
                
                // Create nested objects for gyro and accelerometer data
                sensorData["gyro"]["x"] = String(gyro->getX());
                sensorData["gyro"]["y"] = String(gyro->getY());
                sensorData["gyro"]["z"] = String(gyro->getZ());
                
                sensorData["accel"]["x"] = String(gyro->getAccelX());
                sensorData["accel"]["y"] = String(gyro->getAccelY());
                sensorData["accel"]["z"] = String(gyro->getAccelZ());
                sensorData["accel"]["magnitude"] = String(gyro->getAccelMagnitude());
                
                webSocket->sendJsonMessage(clientId, "sensor_data", sensorData);
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
            } else {
              // Not authenticated
              webSocket->sendError(clientId, 401, "Authentication required");
            }
          }
        }
        // Handle binary data
        else if (info->opcode == WS_BINARY) {
          logger->debug("Received binary data from client #" + String(clientId) + ", length: " + String(len));
          
          // Check if client is authenticated
          if (sessions[clientId % 5].authenticated) {
            // Initialize FileManager for file operations
            static Utils::FileManager fileManager;
            if (!fileManager.init()) {
              logger->error("Failed to initialize FileManager");
              webSocket->sendError(clientId, 500, "Failed to initialize file system");
              break;
            }
            
            // Use the global fileUploads map for state tracking
            
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