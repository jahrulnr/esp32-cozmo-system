# WebSocket and WebServer IoT Framework Architecture (IFA)

This document provides a comprehensive overview of the WebSocket and WebServer implementation in the Cozmo-System project, which can be reused as an IoT Framework Architecture (IFA) for future projects.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Components](#components)
3. [Communication Protocol](#communication-protocol)
4. [Server-Side Implementation](#server-side-implementation)
5. [Client-Side Implementation](#client-side-implementation)
6. [Data Transfer Object (DTO) Format](#data-transfer-object-dto-format)
7. [Authentication and Security](#authentication-and-security)
8. [Binary Data Handling](#binary-data-handling)
9. [Practical Use Cases](#practical-use-cases)
10. [Implementation Guide for New Projects](#implementation-guide-for-new-projects)

## Architecture Overview

The IFA (IoT Framework Architecture) is designed around a bidirectional WebSocket communication layer with a supporting HTTP WebServer for static file serving and RESTful endpoints. This architecture provides:

- **Real-time communication** between IoT devices and clients
- **Efficient data transfer** using standardized DTO format
- **Mixed content delivery** (static files via HTTP, dynamic data via WebSockets)
- **Authentication and session management**
- **Binary data handling** for image streaming and file transfers
- **Modular design** for easy extension and reuse

The framework is built on ESP32 using the Arduino framework but can be adapted to other platforms.

## Components

### Server-Side

1. **WebServer** - Handles HTTP requests, serves static content
   - Based on ESPAsyncWebServer for non-blocking operation
   - Configurable routes and content types
   - File serving from SPIFFS

2. **WebSocketHandler** - Manages WebSocket connections and message passing
   - Supports both text and binary WebSocket frames
   - Standard JSON-based DTO format
   - Event-based architecture with callbacks
   - Client session tracking

3. **Authentication System** - Manages user sessions and access control
   - Username/password authentication
   - Token-based persistent sessions
   - Access control for WebSocket commands

4. **File Manager** - Handles file operations through WebSocket interface
   - File listing, reading, writing
   - Directory creation and navigation
   - Binary file uploads

### Client-Side

1. **WebSocket Client** - Establishes and maintains WebSocket connection
   - Automatic reconnection
   - Message parsing and handling
   - Binary data processing

2. **DTO System** - Standardized message format for all communications
   - Versioned message format
   - Type-based message routing
   - Consistent error handling

3. **UI Components** - Web-based interface for IoT device control
   - Dashboard panels
   - Sensor visualization
   - Device control interfaces

## Communication Protocol

### Message Format

All communications use a standardized DTO (Data Transfer Object) format:

```json
{
  "version": "1.0",
  "type": "command_type",
  "data": {
    // Command-specific payload
  }
}
```

- **version**: String identifying the protocol version (currently "1.0")
- **type**: String specifying the message type (e.g., "motor_command", "system_status")
- **data**: Object containing the actual payload relevant to the message type

### Message Flow

1. **Client to Server**:
   - Commands sent with specific action type and parameters
   - Authentication messages for establishing session
   - File operation requests

2. **Server to Client**:
   - Status updates and command responses
   - Sensor data streams
   - Error messages
   - Binary data with JSON header metadata

## Server-Side Implementation

### WebServer Setup

```cpp
// Initialize WebServer
webServer = new Communication::WebServer();
if (webServer->init(WEBSERVER_PORT)) {
  // Setup routes
  webServer->on("/", [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  // Serve static files from SPIFFS
  webServer->getServer()->serveStatic("/css/", SPIFFS, "/css/");
  webServer->getServer()->serveStatic("/js/", SPIFFS, "/js/");
  
  webServer->begin();
}
```

### WebSocket Setup

```cpp
// Initialize WebSocket handler
webSocket = new Communication::WebSocketHandler();

// Initialize WebSocket with path and existing web server
if (webSocket->init("/ws", webServer->getServer())) {
  // Set the event handler
  webSocket->onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client,
                       AwsEventType type, void* arg, uint8_t* data, size_t len) {
    handleWebSocketEvent(server, client, type, arg, data, len);
  });
  
  webSocket->begin();
}
```

### WebSocket Event Handling

The event handler processes different types of WebSocket events:

```cpp
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                         AwsEventType type, void* arg, uint8_t* data, size_t len) {
  uint32_t clientId = client->id();

  switch (type) {
    case WS_EVT_CONNECT:
      // Handle client connection
      break;
      
    case WS_EVT_DISCONNECT:
      // Handle client disconnection
      // Clean up session and subscription data
      break;
      
    case WS_EVT_DATA:
      // Process incoming data (text or binary)
      // For text: Parse JSON and route to appropriate handler
      // For binary: Process as file upload or image data
      break;
      
    case WS_EVT_ERROR:
      // Handle WebSocket errors
      break;
  }
}
```

### Sending Messages to Clients

```cpp
// Send JSON message to a client
void sendJsonMessage(int clientId, const String& type, const JsonVariant& data) {
  Utils::SpiJsonDocument doc;
  doc["version"] = "1.0";
  doc["type"] = type;
  doc["data"] = data;
  
  String jsonString;
  serializeJson(doc, jsonString);
  sendText(clientId, jsonString);
}

// Send error message
void sendError(int clientId, int code, const String& message) {
  Utils::SpiJsonDocument doc;
  doc["version"] = "1.0";
  doc["type"] = "error";
  doc["data"]["code"] = code;
  doc["data"]["message"] = message;
  
  String jsonString;
  serializeJson(doc, jsonString);
  sendText(clientId, jsonString);
}
```

## Client-Side Implementation

### Establishing WebSocket Connection

```javascript
// Connect WebSocket
function connectWebSocket() {
  const wsUri = `ws://${window.location.hostname}/ws`;
  websocket = new WebSocket(wsUri);

  websocket.onopen = (evt) => {
    console.log('WebSocket Connected');
    // Handle successful connection
    // Send authentication if needed
  };

  websocket.onclose = (evt) => {
    console.log('WebSocket Disconnected');
    // Handle disconnection
    // Set up reconnection if needed
  };

  websocket.onmessage = (evt) => {
    handleWebSocketMessage(evt);
  };

  websocket.onerror = (evt) => {
    console.error('WebSocket Error:', evt);
    // Handle error
  };
}
```

### Sending Commands

```javascript
// Send Command to server using the DTO contract format (v1.0)
function sendJsonMessage(type, data = {}) {
  if (websocket && websocket.readyState === WebSocket.OPEN) {
    const message = JSON.stringify({
      version: "1.0",
      type: type,
      data: data
    });
    websocket.send(message);
  } else {
    console.error('WebSocket not connected');
  }
}

// Example: Sending a motor command
function sendMotorCommand(left, right, duration = 0) {
  sendJsonMessage('motor_command', {
    left: left,
    right: right,
    duration: duration
  });
}
```

### Handling Responses

```javascript
// Handle WebSocket messages
function handleWebSocketMessage(evt) {
  // Handle binary data (camera frames, file downloads)
  if (evt.data instanceof ArrayBuffer || evt.data instanceof Blob) {
    handleBinaryMessage(evt.data);
    return;
  }
  
  // Handle text data (JSON messages)
  try {
    const msg = JSON.parse(evt.data);
    
    // Route message to appropriate handler based on type
    switch (msg.type) {
      case 'system_status':
        updateStatus(msg.data);
        break;
      case 'sensor_data':
        updateSensors(msg.data);
        break;
      case 'error':
        handleError(msg.data);
        break;
      // Additional message types...
    }
  } catch (e) {
    console.error('Error parsing WebSocket message:', e);
  }
}
```

## Data Transfer Object (DTO) Format

The DTO format is a standardized structure for all messages exchanged between client and server:

### Command Messages (Client → Server)

```json
{
  "version": "1.0",
  "type": "command_type",
  "data": {
    "parameter1": "value1",
    "parameter2": "value2"
  }
}
```

### Response Messages (Server → Client)

```json
{
  "version": "1.0",
  "type": "response_type",
  "data": {
    "result": "value",
    "status": "success"
  }
}
```

### Error Messages (Server → Client)

```json
{
  "version": "1.0",
  "type": "error",
  "data": {
    "code": 403,
    "message": "Access denied"
  }
}
```

## Authentication and Security

The framework implements a basic authentication system:

1. **Login Process**:
   - Client sends login credentials via WebSocket
   - Server validates credentials and creates a session
   - Server returns authentication token for persistent sessions

```javascript
// Client-side login
function login(username, password) {
  sendJsonMessage('login', {
    username: username,
    password: password
  });
}

// Auto-login with saved token
function autoLogin() {
  const savedAuth = localStorage.getItem('auth_token_key');
  if (savedAuth) {
    const authData = JSON.parse(savedAuth);
    sendJsonMessage('login', {
      username: authData.username,
      password: 'AUTO_LOGIN_TOKEN',
      token: authData.token
    });
  }
}
```

2. **Session Management**:
   - Server maintains client sessions with authentication state
   - Commands are checked against authentication status
   - Sessions expire on disconnect or explicit logout

```cpp
// Server-side authentication check
if (sessions[clientId % 5].authenticated) {
  // Process authenticated command
} else {
  // Return authentication error
  webSocket->sendError(clientId, 401, "Authentication required");
}
```

## Binary Data Handling

The framework supports binary data transmission for file transfers and camera streaming:

### Camera Streaming

1. **Camera Frame Header**:
   - JSON message with frame metadata (dimensions, format, etc.)
   - Followed by binary frame data

2. **Binary Processing**:
   ```javascript
   // Handle binary message (camera frame)
   function handleBinaryMessage(data) {
     // If we have frame header metadata from previous message
     if (frameHeader) {
       // Add frame to queue
       frameQueue.push({
         data: data,
         header: frameHeader
       });
       
       // Process frame (convert to blob, create URL, update UI)
       processNextFrame();
       
       // Clear the frame header
       frameHeader = null;
     }
   }
   ```

### File Uploads

1. **File Upload Preparation**:
   - Client sends file metadata via JSON
   - Server prepares to receive binary data

2. **Binary Upload**:
   - Client sends binary file data
   - Server writes to file system
   - Server confirms successful upload

## Practical Use Cases

### 1. Sensor Data Monitoring

```javascript
// Client sends request for sensor updates
sendJsonMessage('sensor_request', { interval: 1000 });

// Server sends periodic updates
function sendSensorData() {
  Utils::SpiJsonDocument sensorData;
  sensorData["temperature"] = temperatureSensor->readTemperature();
  sensorData["humidity"] = humiditySensor->readHumidity();
  sensorData["pressure"] = pressureSensor->readPressure();
  
  webSocket->sendJsonMessage(clientId, "sensor_data", sensorData);
}
```

### 2. Device Control

```javascript
// Client sends control command
function setDeviceState(deviceId, state) {
  sendJsonMessage('device_control', {
    id: deviceId,
    state: state
  });
}

// Server processes command
if (type == "device_control") {
  String deviceId = data["id"];
  bool state = data["state"];
  
  // Control the physical device
  controlDevice(deviceId, state);
  
  // Send acknowledgement
  Utils::SpiJsonDocument response;
  response["success"] = true;
  response["device_id"] = deviceId;
  response["state"] = state;
  
  webSocket->sendJsonMessage(clientId, "device_status", response);
}
```

### 3. File Management

```javascript
// Client requests file listing
function fetchFiles(path) {
  sendJsonMessage('list_files', { path: path });
}

// Server response
void handleListFiles(uint32_t clientId, const JsonVariant& data) {
  String path = data["path"] | "/";
  
  Utils::SpiJsonDocument response;
  JsonArray files = response.createNestedArray();
  
  // Populate file list from SPIFFS
  File root = SPIFFS.open(path);
  if (root && root.isDirectory()) {
    File file = root.openNextFile();
    while (file) {
      JsonObject fileObj = files.createNestedObject();
      fileObj["name"] = file.name();
      fileObj["size"] = file.size();
      fileObj["type"] = file.isDirectory() ? "directory" : "file";
      
      file = root.openNextFile();
    }
  }
  
  webSocket->sendJsonMessage(clientId, "list_files", response);
}
```

## Implementation Guide for New Projects

### 1. Set up the WebServer and WebSocket Handler

```cpp
#include <WebServer.h>
#include <WebSocketHandler.h>

Communication::WebServer* webServer = nullptr;
Communication::WebSocketHandler* webSocket = nullptr;

void setup() {
  // Initialize file system
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed");
    return;
  }
  
  // Initialize WebServer
  webServer = new Communication::WebServer();
  if (webServer->init(80)) {
    // Configure routes for static content
    webServer->on("/", [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html");
    });
    
    webServer->getServer()->serveStatic("/", SPIFFS, "/");
    
    // Start server
    webServer->begin();
    Serial.println("Web server started");
  }
  
  // Initialize WebSocket handler
  webSocket = new Communication::WebSocketHandler();
  if (webSocket->init("/ws", webServer->getServer())) {
    webSocket->onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client,
                       AwsEventType type, void* arg, uint8_t* data, size_t len) {
      handleWebSocketEvent(server, client, type, arg, data, len);
    });
    
    webSocket->begin();
    Serial.println("WebSocket server started");
  }
}
```

### 2. Implement WebSocket Event Handler

```cpp
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                         AwsEventType type, void* arg, uint8_t* data, size_t len) {
  uint32_t clientId = client->id();

  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected\n", clientId);
      break;
      
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", clientId);
      break;
      
    case WS_EVT_DATA:
      {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        
        // Handle text messages (commands)
        if (info->opcode == WS_TEXT && info->final && info->index == 0 && info->len == len) {
          // Parse JSON message
          DeserializationError error;
          DynamicJsonDocument doc(4096);
          
          error = deserializeJson(doc, data, len);
          
          if (error) {
            Serial.printf("JSON parsing error: %s\n", error.c_str());
            return;
          }
          
          String type = doc["type"];
          JsonVariant messageData = doc["data"];
          
          // Process message based on type
          if (type == "ping") {
            // Send pong response
            DynamicJsonDocument response(128);
            response["time"] = millis();
            
            String jsonResponse;
            serializeJson(response, jsonResponse);
            
            webSocket->sendJsonMessage(clientId, "pong", jsonResponse);
          }
          // Add more message type handlers here
        }
        // Handle binary data
        else if (info->opcode == WS_BINARY) {
          // Process binary data
          Serial.printf("Received binary data from client #%u, length: %u\n", clientId, len);
        }
      }
      break;
      
    case WS_EVT_ERROR:
      Serial.printf("WebSocket client #%u error\n", clientId);
      break;
  }
}
```

### 3. Create Client-Side Implementation

```html
<!DOCTYPE html>
<html>
<head>
  <title>IoT Device Control</title>
  <script>
    let websocket = null;
    
    function initWebSocket() {
      const wsUri = `ws://${window.location.hostname}/ws`;
      websocket = new WebSocket(wsUri);
      
      websocket.onopen = (evt) => {
        console.log('WebSocket Connected');
        document.getElementById('status').innerHTML = 'Connected';
      };
      
      websocket.onclose = (evt) => {
        console.log('WebSocket Disconnected');
        document.getElementById('status').innerHTML = 'Disconnected';
        // Try to reconnect after 2 seconds
        setTimeout(initWebSocket, 2000);
      };
      
      websocket.onmessage = (evt) => {
        try {
          const msg = JSON.parse(evt.data);
          console.log('Received:', msg);
          
          if (msg.type === 'sensor_data') {
            updateSensorDisplay(msg.data);
          } else if (msg.type === 'device_status') {
            updateDeviceStatus(msg.data);
          }
        } catch (e) {
          console.error('Error parsing WebSocket message:', e);
        }
      };
      
      websocket.onerror = (evt) => {
        console.error('WebSocket Error:', evt);
      };
    }
    
    function sendCommand(type, data) {
      if (websocket && websocket.readyState === WebSocket.OPEN) {
        const message = JSON.stringify({
          version: "1.0",
          type: type,
          data: data
        });
        websocket.send(message);
      } else {
        console.error('WebSocket not connected');
      }
    }
    
    function toggleDevice(deviceId) {
      sendCommand('device_control', {
        id: deviceId,
        state: document.getElementById('device-' + deviceId).checked
      });
    }
    
    function updateSensorDisplay(data) {
      // Update UI with sensor data
      if (data.temperature) {
        document.getElementById('temperature').innerText = data.temperature + '°C';
      }
      if (data.humidity) {
        document.getElementById('humidity').innerText = data.humidity + '%';
      }
    }
    
    function updateDeviceStatus(data) {
      // Update UI with device status
      const deviceElement = document.getElementById('device-' + data.device_id);
      if (deviceElement) {
        deviceElement.checked = data.state;
      }
    }
    
    window.addEventListener('load', (e) => {
      initWebSocket();
    });
  </script>
</head>
<body>
  <h1>IoT Device Control</h1>
  <div>Status: <span id="status">Disconnected</span></div>
  
  <h2>Sensors</h2>
  <div>Temperature: <span id="temperature">--</span></div>
  <div>Humidity: <span id="humidity">--</span></div>
  
  <h2>Device Control</h2>
  <div>
    <label>Device 1: 
      <input type="checkbox" id="device-1" onchange="toggleDevice('1')">
    </label>
  </div>
  <div>
    <label>Device 2: 
      <input type="checkbox" id="device-2" onchange="toggleDevice('2')">
    </label>
  </div>
</body>
</html>
```

### 4. Data Serialization and Memory Management

For ESP32 with limited memory, consider using efficient serialization:

- Use the ArduinoJson library with its zero-copy mode
- Pre-calculate document size to avoid memory fragmentation
- Use external PSRAM if available for larger JSON documents
- Implement pagination for large datasets (files, logs)

```cpp
// Example of efficient JSON processing
void createSensorResponseJson(JsonDocument& doc, float temperature, float humidity) {
  doc.clear();
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
}

void sendSensorData(uint32_t clientId) {
  // Calculate JSON capacity in advance
  const size_t capacity = JSON_OBJECT_SIZE(2) + 30;
  
  // Create document with calculated capacity
  StaticJsonDocument<capacity> doc;
  
  // Populate with data
  createSensorResponseJson(doc, 
    temperatureSensor->readTemperature(),
    humiditySensor->readHumidity()
  );
  
  // Send to client
  webSocket->sendJsonMessage(clientId, "sensor_data", doc);
}
```

### 5. Adding Authentication

```cpp
// Simple session store
struct Session {
  bool authenticated = false;
  String username = "";
  unsigned long lastActivity = 0;
};

// Max 10 simultaneous sessions
Session sessions[10];

// Process login message
void handleLogin(uint32_t clientId, const JsonVariant& data) {
  String username = data["username"] | "";
  String password = data["password"] | "";
  
  // Validate credentials (replace with your authentication logic)
  bool isValid = (username == "admin" && password == "password");
  
  if (isValid) {
    // Set up session
    sessions[clientId % 10].authenticated = true;
    sessions[clientId % 10].username = username;
    sessions[clientId % 10].lastActivity = millis();
    
    DynamicJsonDocument response(256);
    response["success"] = true;
    response["token"] = "auth_" + username + "_" + String(millis());
    
    webSocket->sendJsonMessage(clientId, "login_response", response);
    Serial.printf("User logged in: %s\n", username.c_str());
  } else {
    DynamicJsonDocument response(128);
    response["success"] = false;
    response["message"] = "Invalid credentials";
    
    webSocket->sendJsonMessage(clientId, "login_response", response);
    Serial.printf("Failed login attempt for user: %s\n", username.c_str());
  }
}

// Check if client is authenticated
bool isAuthenticated(uint32_t clientId) {
  return sessions[clientId % 10].authenticated;
}
```

By following this guide, you'll be able to implement a complete WebSocket and WebServer-based IoT Framework Architecture for your future projects.
