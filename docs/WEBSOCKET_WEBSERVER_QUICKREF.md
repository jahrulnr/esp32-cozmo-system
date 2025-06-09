# WebSocket and WebServer IoT Framework - Quick Reference Guide

This guide provides a quick reference for implementing the WebSocket and WebServer IoT Framework Architecture in your projects.

## Basic Implementation Flow

```
┌─────────────┐     HTTP     ┌─────────────┐
│             │◄───────────► │             │
│   Browser   │              │  WebServer  │
│    Client   │     WS       │             │
│             │◄───────────► │  WebSocket  │
└─────────────┘              │   Server    │
                             └─────────────┘
```

1. Static files and initial UI served over HTTP
2. Real-time data and commands exchanged over WebSocket
3. Standardized DTO format for all messages
4. Authentication system to secure communications
5. Binary data support for files and video streams

## Server-Side Implementation

### Step 1: Initialize Components

```cpp
// In your setup() function
void setup() {
  // Initialize file system for static content
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed");
    return;
  }
  
  // Initialize WebServer
  webServer = new Communication::WebServer();
  webServer->init(80);
  
  // Configure static routes
  webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  // Serve static files
  webServer->getServer()->serveStatic("/css/", SPIFFS, "/css/");
  webServer->getServer()->serveStatic("/js/", SPIFFS, "/js/");
  
  // Start WebServer
  webServer->begin();
  
  // Initialize WebSocket
  webSocket = new Communication::WebSocketHandler();
  webSocket->init("/ws", webServer->getServer());
  
  // Set WebSocket event handler
  webSocket->onEvent(handleWebSocketEvent);
  
  // Start WebSocket
  webSocket->begin();
}
```

### Step 2: WebSocket Event Handler

```cpp
void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                         AwsEventType type, void* arg, uint8_t* data, size_t len) {
  uint32_t clientId = client->id();

  switch (type) {
    // Handle client connection
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected\n", clientId);
      break;
      
    // Handle client disconnection  
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", clientId);
      // Clean up resources for this client
      break;
      
    // Handle incoming data  
    case WS_EVT_DATA:
      {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        
        // Process text message (JSON commands)
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
          // Parse JSON message
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, data, len);
          
          // Extract message components
          String type = doc["type"].as<String>();
          JsonVariant messageData = doc["data"];
          
          // Process message based on type
          processMessage(clientId, type, messageData);
        }
        // Process binary message (file upload, image data)
        else if (info->opcode == WS_BINARY) {
          processBinaryMessage(clientId, data, len);
        }
      }
      break;
      
    // Handle errors  
    case WS_EVT_ERROR:
      Serial.printf("WebSocket error for client #%u\n", clientId);
      break;
  }
}
```

### Step 3: Message Processing

```cpp
void processMessage(uint32_t clientId, const String& type, const JsonVariant& data) {
  // Process login message
  if (type == "login") {
    String username = data["username"].as<String>();
    String password = data["password"].as<String>();
    
    // Authenticate user
    bool success = (username == "admin" && password == "password");
    
    // Send response
    DynamicJsonDocument response(256);
    response["success"] = success;
    if (success) {
      response["token"] = "auth_token_" + String(millis());
      // Create session for user
      sessions[clientId % MAX_SESSIONS].authenticated = true;
    } else {
      response["message"] = "Authentication failed";
    }
    
    webSocket->sendJsonMessage(clientId, "login_response", response);
  }
  // Process sensor request
  else if (type == "get_sensor_data" && isAuthenticated(clientId)) {
    // Read sensor values
    float temperature = readTemperature();
    float humidity = readHumidity();
    
    // Create response
    DynamicJsonDocument response(256);
    response["temperature"] = temperature;
    response["humidity"] = humidity;
    response["timestamp"] = millis();
    
    webSocket->sendJsonMessage(clientId, "sensor_data", response);
  }
  // Add more message handlers here
}

// Check if client is authenticated
bool isAuthenticated(uint32_t clientId) {
  return sessions[clientId % MAX_SESSIONS].authenticated;
}
```

### Step 4: Sending Messages to Clients

```cpp
// Send sensor data to all connected clients
void broadcastSensorData() {
  if (webSocket->hasClients()) {
    DynamicJsonDocument doc(256);
    doc["temperature"] = readTemperature();
    doc["humidity"] = readHumidity();
    doc["timestamp"] = millis();
    
    webSocket->sendJsonMessage(-1, "sensor_data", doc);
  }
}

// Send error message to specific client
void sendError(uint32_t clientId, int code, const String& message) {
  DynamicJsonDocument doc(256);
  doc["code"] = code;
  doc["message"] = message;
  
  webSocket->sendJsonMessage(clientId, "error", doc);
}
```

## Client-Side Implementation

### Step 1: Connect to WebSocket

```javascript
// WebSocket connection
let websocket = null;
let reconnectInterval = null;

function connectWebSocket() {
  // Close any existing connection
  if (websocket) {
    websocket.close();
    websocket = null;
  }
  
  // Create new WebSocket connection
  const wsUri = `ws://${window.location.hostname}/ws`;
  websocket = new WebSocket(wsUri);
  
  // Connection opened
  websocket.onopen = (evt) => {
    console.log('WebSocket Connected');
    clearInterval(reconnectInterval);
    
    // Authenticate if token exists
    const authToken = localStorage.getItem('auth_token');
    if (authToken) {
      sendJsonMessage('login', {
        token: authToken,
        password: 'AUTO_LOGIN_TOKEN'
      });
    }
  };
  
  // Connection closed
  websocket.onclose = (evt) => {
    console.log('WebSocket Disconnected');
    
    // Attempt to reconnect
    if (!reconnectInterval) {
      reconnectInterval = setInterval(connectWebSocket, 3000);
    }
  };
  
  // Message received
  websocket.onmessage = (evt) => {
    handleWebSocketMessage(evt);
  };
  
  // Error occurred
  websocket.onerror = (evt) => {
    console.error('WebSocket Error:', evt);
  };
}
```

### Step 2: Send and Receive Messages

```javascript
// Send message to server
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

// Handle received message
function handleWebSocketMessage(evt) {
  // Handle binary data (camera frames, file downloads)
  if (evt.data instanceof ArrayBuffer || evt.data instanceof Blob) {
    handleBinaryMessage(evt.data);
    return;
  }
  
  // Handle JSON messages
  try {
    const msg = JSON.parse(evt.data);
    const type = msg.type;
    const data = msg.data;
    
    // Process based on message type
    switch (type) {
      case 'login_response':
        handleLoginResponse(data);
        break;
        
      case 'sensor_data':
        updateSensorUI(data);
        break;
        
      case 'error':
        showError(data);
        break;
        
      // Add more handlers as needed
    }
  } catch (e) {
    console.error('Error parsing WebSocket message:', e);
  }
}
```

### Step 3: Specific Message Handling

```javascript
// Handle login response
function handleLoginResponse(data) {
  if (data.success) {
    // Store authentication token
    localStorage.setItem('auth_token', data.token);
    
    // Show main application
    showApp();
    
    // Request initial data
    sendJsonMessage('get_sensor_data');
  } else {
    // Show error message
    showError({
      message: data.message || 'Login failed'
    });
  }
}

// Update sensor display
function updateSensorUI(data) {
  document.getElementById('temperature').textContent = data.temperature + '°C';
  document.getElementById('humidity').textContent = data.humidity + '%';
  document.getElementById('last-update').textContent = new Date().toLocaleTimeString();
}

// Show error message
function showError(data) {
  const errorDiv = document.getElementById('error-message');
  errorDiv.textContent = data.message || 'An error occurred';
  errorDiv.style.display = 'block';
  
  // Hide error after 5 seconds
  setTimeout(() => {
    errorDiv.style.display = 'none';
  }, 5000);
}
```

### Step 4: Handle Binary Data

```javascript
// Handle binary message (e.g., camera frames)
function handleBinaryMessage(data) {
  // Convert binary data to blob
  const blob = data instanceof Blob ? data : new Blob([data]);
  
  // Create URL for blob
  const url = URL.createObjectURL(blob);
  
  // Update image source
  const imageElement = document.getElementById('camera-image');
  if (imageElement) {
    // Release previous URL to avoid memory leaks
    if (imageElement.src && imageElement.src.startsWith('blob:')) {
      URL.revokeObjectURL(imageElement.src);
    }
    
    imageElement.src = url;
  }
}
```

## Standard Message Types

| Type | Direction | Purpose | Sample Data |
|------|-----------|---------|-------------|
| `login` | C → S | Authenticate user | `{"username": "admin", "password": "pwd"}` |
| `login_response` | S → C | Auth result | `{"success": true, "token": "auth_xyz"}` |
| `system_status` | S → C | System info | `{"memory": "32KB", "uptime": 3600}` |
| `sensor_data` | S → C | Sensor readings | `{"temperature": 23.5, "humidity": 60}` |
| `device_control` | C → S | Control device | `{"id": "light1", "state": true}` |
| `error` | S → C | Error information | `{"code": 403, "message": "Access denied"}` |

## Best Practices

1. **Authentication**: Always authenticate users before allowing actions
2. **Error Handling**: Provide clear error messages for all failure cases
3. **Reconnection**: Implement automatic WebSocket reconnection
4. **Memory Management**: Minimize JSON document size for ESP32
5. **Security**: Use HTTPS and WSS in production environments
6. **Versioning**: Include version in all messages for future compatibility
7. **Throttling**: Limit message frequency to avoid overloading devices
8. **Session Cleanup**: Remove disconnected client sessions

## Example Implementation

See the full implementation and detailed documentation in:
- [WebSocket WebServer IFA Documentation](WEBSOCKET_WEBSERVER_IFA.md)
- [DTO Format Documentation](DTO_FORMAT.md)
- [WebSocketHandler Implementation](../app/lib/Communication/WebSocketHandler.cpp)
- [WebServer Implementation](../app/lib/Communication/WebServer.cpp)
