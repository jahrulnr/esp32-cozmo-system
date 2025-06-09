# WebSocket and WebServer IoT Framework - Template Implementation

This file provides template code for implementing the WebSocket and WebServer IoT Framework Architecture (IFA) in your ESP32 projects.

## Server-Side Template (ESP32 Arduino)

```cpp
// WebSocketServerTemplate.ino
// A template for implementing WebSocket+WebServer IoT Framework

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// Authentication credentials
const char* AUTH_USERNAME = "admin";
const char* AUTH_PASSWORD = "password";

// Server objects
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Authentication session tracking (simple implementation)
#define MAX_CLIENTS 10
struct {
  bool authenticated;
  unsigned long lastActivity;
} clientSessions[MAX_CLIENTS];

// Forward declarations
void handleWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);
void sendJsonMessage(AsyncWebSocketClient* client, const String& type, const JsonDocument& data);
void sendErrorMessage(AsyncWebSocketClient* client, int code, const String& message);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Starting IoT WebSocket Server");
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Initialize client sessions
  for (int i = 0; i < MAX_CLIENTS; i++) {
    clientSessions[i].authenticated = false;
    clientSessions[i].lastActivity = 0;
  }

  // Set up WebSocket event handler
  ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, 
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", 
                    client->id(), client->remoteIP().toString().c_str());
        break;
        
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        // Clear session data
        clientSessions[client->id() % MAX_CLIENTS].authenticated = false;
        break;
        
      case WS_EVT_DATA:
        handleWebSocketMessage(client, arg, data, len);
        break;
        
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
    }
  });

  // Add WebSocket to server
  server.addHandler(&ws);

  // Define web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Serve static files
  server.serveStatic("/", SPIFFS, "/");

  // Handle not found pages
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not Found");
  });

  // Start server
  server.begin();
}

void loop() {
  // WebSocket is handled by AsyncWebSocket library
  // You can add periodic tasks here:
  
  // Example: Send sensor data every 5 seconds
  static unsigned long lastSensorUpdate = 0;
  if (millis() - lastSensorUpdate > 5000) {
    broadcastSensorData();
    lastSensorUpdate = millis();
  }
  
  // Handle other tasks as needed
  yield();
}

// Process WebSocket messages
void handleWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  
  // Ensure the message is complete and is text format
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    // Null-terminate the data for string processing
    data[len] = 0;
    
    // Parse JSON message
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, (char*)data);
    
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      sendErrorMessage(client, 400, "Invalid JSON");
      return;
    }
    
    // Extract message fields
    const char* type = doc["type"];
    JsonObject jsonData = doc["data"];
    
    // Update client activity timestamp
    clientSessions[client->id() % MAX_CLIENTS].lastActivity = millis();
    
    // Process message based on type
    if (strcmp(type, "login") == 0) {
      handleLogin(client, jsonData);
    } 
    else {
      // Check authentication for all other message types
      if (clientSessions[client->id() % MAX_CLIENTS].authenticated) {
        if (strcmp(type, "get_sensor_data") == 0) {
          handleGetSensorData(client);
        }
        else if (strcmp(type, "set_output") == 0) {
          handleSetOutput(client, jsonData);
        }
        else if (strcmp(type, "system_status") == 0) {
          handleSystemStatus(client);
        }
        else {
          Serial.printf("Unknown message type: %s\n", type);
          sendErrorMessage(client, 400, "Unknown message type");
        }
      } else {
        sendErrorMessage(client, 401, "Authentication required");
      }
    }
  }
}

// Handle login requests
void handleLogin(AsyncWebSocketClient* client, JsonObject& data) {
  const char* username = data["username"];
  const char* password = data["password"];
  
  // Validate credentials (replace with your authentication logic)
  bool authenticated = (strcmp(username, AUTH_USERNAME) == 0 && 
                      strcmp(password, AUTH_PASSWORD) == 0);
  
  // Set session state
  clientSessions[client->id() % MAX_CLIENTS].authenticated = authenticated;
  
  // Send response
  DynamicJsonDocument response(256);
  response["success"] = authenticated;
  
  if (authenticated) {
    // Generate a simple token (in production, use a more secure method)
    char token[64];
    sprintf(token, "auth_%s_%lu", username, millis());
    response["token"] = token;
    
    Serial.printf("Client #%u authenticated as %s\n", client->id(), username);
  } else {
    response["message"] = "Invalid credentials";
    Serial.printf("Client #%u failed authentication attempt\n", client->id());
  }
  
  sendJsonMessage(client, "login_response", response);
}

// Handle sensor data requests
void handleGetSensorData(AsyncWebSocketClient* client) {
  // Read sensor values (replace with your actual sensor code)
  float temperature = readTemperature();
  float humidity = readHumidity();
  
  // Create response
  DynamicJsonDocument response(256);
  response["temperature"] = temperature;
  response["humidity"] = humidity;
  response["timestamp"] = millis();
  
  sendJsonMessage(client, "sensor_data", response);
}

// Handle digital output control
void handleSetOutput(AsyncWebSocketClient* client, JsonObject& data) {
  int pin = data["pin"];
  bool state = data["state"];
  
  // Validate pin number
  if (pin < 0 || pin > 39) {
    sendErrorMessage(client, 400, "Invalid pin number");
    return;
  }
  
  // Set the pin mode and state
  pinMode(pin, OUTPUT);
  digitalWrite(pin, state ? HIGH : LOW);
  
  // Send confirmation
  DynamicJsonDocument response(128);
  response["pin"] = pin;
  response["state"] = state;
  response["success"] = true;
  
  sendJsonMessage(client, "output_state", response);
}

// Handle system status request
void handleSystemStatus(AsyncWebSocketClient* client) {
  DynamicJsonDocument response(512);
  
  // System information
  response["uptime"] = millis() / 1000;
  response["heap"] = ESP.getFreeHeap();
  response["cpu_freq"] = ESP.getCpuFreqMHz();
  response["flash_size"] = ESP.getFlashChipSize();
  
  // WiFi information
  response["wifi_ssid"] = WIFI_SSID;
  response["wifi_strength"] = WiFi.RSSI();
  response["ip"] = WiFi.localIP().toString();
  
  sendJsonMessage(client, "system_status", response);
}

// Send JSON message to client
void sendJsonMessage(AsyncWebSocketClient* client, const String& type, const JsonDocument& data) {
  DynamicJsonDocument doc(1024);
  doc["version"] = "1.0";
  doc["type"] = type;
  doc["data"] = data;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  client->text(jsonString);
}

// Send error message to client
void sendErrorMessage(AsyncWebSocketClient* client, int code, const String& message) {
  DynamicJsonDocument data(256);
  data["code"] = code;
  data["message"] = message;
  
  sendJsonMessage(client, "error", data);
}

// Broadcast sensor data to all clients
void broadcastSensorData() {
  // Only send if there are connected clients
  if (ws.count() > 0) {
    // Read sensor values
    float temperature = readTemperature();
    float humidity = readHumidity();
    
    // Create message
    DynamicJsonDocument data(256);
    data["temperature"] = temperature;
    data["humidity"] = humidity;
    data["timestamp"] = millis();
    
    // Serialize response
    DynamicJsonDocument doc(512);
    doc["version"] = "1.0";
    doc["type"] = "sensor_update";
    doc["data"] = data;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Send to all authenticated clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (clientSessions[i].authenticated) {
        ws.textAll(jsonString);
        break; // Send only once since textAll sends to everyone
      }
    }
  }
}

// Simulated temperature reading (replace with actual sensor code)
float readTemperature() {
  // Simulate temperature reading between 20-30°C
  return random(200, 300) / 10.0;
}

// Simulated humidity reading (replace with actual sensor code)
float readHumidity() {
  // Simulate humidity reading between 30-80%
  return random(300, 800) / 10.0;
}
```

## Client-Side Template (HTML/JS)

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>IoT Control Panel</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background-color: #f0f0f0;
    }
    
    .container {
      max-width: 800px;
      margin: 0 auto;
      background-color: #fff;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    }
    
    h1 {
      color: #333;
      margin-top: 0;
    }
    
    .login-container {
      margin-bottom: 20px;
    }
    
    .main-container {
      display: none;
    }
    
    .card {
      background-color: #f9f9f9;
      border-radius: 5px;
      padding: 15px;
      margin-bottom: 15px;
    }
    
    .card h2 {
      margin-top: 0;
      font-size: 18px;
      color: #444;
    }
    
    .status {
      display: flex;
      gap: 10px;
      margin-bottom: 10px;
    }
    
    .status-indicator {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background-color: #ccc;
    }
    
    .status-indicator.connected {
      background-color: #4CAF50;
    }
    
    .status-indicator.disconnected {
      background-color: #F44336;
    }
    
    input, button {
      padding: 8px 12px;
      margin: 5px;
    }
    
    button {
      background-color: #2196F3;
      border: none;
      color: white;
      cursor: pointer;
      border-radius: 4px;
    }
    
    button:hover {
      background-color: #0b7dda;
    }
    
    .error {
      color: #F44336;
      margin: 10px 0;
      display: none;
    }
    
    .controls {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
    }
    
    .switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
    }
    
    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      transition: .4s;
      border-radius: 34px;
    }
    
    .slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }
    
    input:checked + .slider {
      background-color: #2196F3;
    }
    
    input:checked + .slider:before {
      transform: translateX(26px);
    }
    
    .control-item {
      display: flex;
      align-items: center;
      margin-bottom: 10px;
    }
    
    .control-item label {
      margin-left: 10px;
    }
    
    #status-message {
      background-color: #f0f0f0;
      padding: 10px;
      border-left: 4px solid #2196F3;
      margin: 10px 0;
      display: none;
    }
    
    .log {
      max-height: 200px;
      overflow-y: auto;
      background-color: #f5f5f5;
      border: 1px solid #ddd;
      padding: 10px;
      font-family: monospace;
      margin-top: 10px;
    }
    
    .log-item {
      margin-bottom: 5px;
      border-bottom: 1px solid #eee;
      padding-bottom: 5px;
    }
    
    .log-item.info {
      color: #333;
    }
    
    .log-item.success {
      color: #4CAF50;
    }
    
    .log-item.error {
      color: #F44336;
    }
    
    .log-timestamp {
      color: #777;
      margin-right: 5px;
    }
    
    @media (max-width: 600px) {
      body {
        padding: 10px;
      }
      
      .container {
        padding: 15px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="status">
      <span class="status-indicator" id="connection-indicator"></span>
      <span id="connection-status">Disconnected</span>
    </div>
    
    <div id="status-message"></div>
    
    <div id="login-container" class="login-container">
      <h1>IoT Control Panel</h1>
      <form id="login-form">
        <div>
          <input type="text" id="username" placeholder="Username" required>
        </div>
        <div>
          <input type="password" id="password" placeholder="Password" required>
        </div>
        <div id="login-error" class="error"></div>
        <div>
          <button type="submit">Login</button>
        </div>
      </form>
    </div>
    
    <div id="main-container" class="main-container">
      <h1>IoT Control Panel</h1>
      
      <div class="card">
        <h2>Sensor Data</h2>
        <div>Temperature: <span id="temperature">--</span>°C</div>
        <div>Humidity: <span id="humidity">--</span>%</div>
        <div>Last update: <span id="last-update">--</span></div>
        <button id="refresh-sensors">Refresh</button>
      </div>
      
      <div class="card">
        <h2>GPIO Controls</h2>
        <div class="controls">
          <div class="control-item">
            <label class="switch">
              <input type="checkbox" id="gpio-2" data-pin="2">
              <span class="slider"></span>
            </label>
            <label for="gpio-2">GPIO 2</label>
          </div>
          
          <div class="control-item">
            <label class="switch">
              <input type="checkbox" id="gpio-4" data-pin="4">
              <span class="slider"></span>
            </label>
            <label for="gpio-4">GPIO 4</label>
          </div>
          
          <div class="control-item">
            <label class="switch">
              <input type="checkbox" id="gpio-12" data-pin="12">
              <span class="slider"></span>
            </label>
            <label for="gpio-12">GPIO 12</label>
          </div>
          
          <div class="control-item">
            <label class="switch">
              <input type="checkbox" id="gpio-13" data-pin="13">
              <span class="slider"></span>
            </label>
            <label for="gpio-13">GPIO 13</label>
          </div>
        </div>
      </div>
      
      <div class="card">
        <h2>System Information</h2>
        <div>Uptime: <span id="uptime">--</span> seconds</div>
        <div>Free Memory: <span id="free-memory">--</span> bytes</div>
        <div>CPU Frequency: <span id="cpu-freq">--</span> MHz</div>
        <div>IP Address: <span id="ip-address">--</span></div>
        <button id="refresh-system">Refresh</button>
      </div>
      
      <div class="card">
        <h2>Console Log</h2>
        <div id="log" class="log"></div>
        <button id="clear-log">Clear</button>
      </div>
      
      <p>
        <button id="logout-button">Logout</button>
      </p>
    </div>
  </div>

  <script>
    // WebSocket connection
    let websocket = null;
    let reconnectInterval = null;
    let isAuthenticated = false;
    
    // DOM Elements
    const connectionStatus = document.getElementById('connection-status');
    const connectionIndicator = document.getElementById('connection-indicator');
    const loginContainer = document.getElementById('login-container');
    const mainContainer = document.getElementById('main-container');
    const statusMessage = document.getElementById('status-message');
    const loginError = document.getElementById('login-error');
    
    // Initialize the application
    document.addEventListener('DOMContentLoaded', () => {
      // Check for saved authentication
      const savedAuth = localStorage.getItem('iot_auth');
      if (savedAuth) {
        try {
          const authData = JSON.parse(savedAuth);
          if (authData && authData.token) {
            // We have a saved token, connect to WebSocket
            connectWebSocket();
          }
        } catch (e) {
          console.error('Error parsing saved auth:', e);
          localStorage.removeItem('iot_auth');
        }
      }
      
      // Set up event listeners
      setupEventListeners();
    });
    
    // Set up all event listeners
    function setupEventListeners() {
      // Login form submission
      document.getElementById('login-form').addEventListener('submit', (e) => {
        e.preventDefault();
        
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;
        
        if (!username || !password) {
          showError('Please enter username and password');
          return;
        }
        
        // Connect WebSocket if not already connected
        if (!websocket || websocket.readyState !== WebSocket.OPEN) {
          connectWebSocket(() => {
            login(username, password);
          });
        } else {
          login(username, password);
        }
      });
      
      // Sensor refresh button
      document.getElementById('refresh-sensors').addEventListener('click', () => {
        sendJsonMessage('get_sensor_data');
      });
      
      // System refresh button
      document.getElementById('refresh-system').addEventListener('click', () => {
        sendJsonMessage('system_status');
      });
      
      // GPIO control checkboxes
      document.querySelectorAll('input[data-pin]').forEach(checkbox => {
        checkbox.addEventListener('change', (e) => {
          const pin = parseInt(e.target.dataset.pin);
          const state = e.target.checked;
          
          sendJsonMessage('set_output', {
            pin: pin,
            state: state
          });
        });
      });
      
      // Logout button
      document.getElementById('logout-button').addEventListener('click', () => {
        logout();
      });
      
      // Clear log button
      document.getElementById('clear-log').addEventListener('click', () => {
        document.getElementById('log').innerHTML = '';
      });
    }
    
    // Connect to WebSocket server
    function connectWebSocket(callback) {
      // Close existing connection if any
      if (websocket && websocket.readyState !== WebSocket.CLOSED) {
        websocket.close();
      }
      
      // Get current hostname/IP and create WebSocket URI
      const protocol = window.location.protocol === 'https:' ? 'wss://' : 'ws://';
      const wsUri = `${protocol}${window.location.host}/ws`;
      
      console.log(`Connecting to WebSocket at ${wsUri}`);
      updateConnectionStatus('Connecting...');
      
      websocket = new WebSocket(wsUri);
      
      websocket.onopen = (evt) => {
        console.log('WebSocket Connected');
        updateConnectionStatus('Connected', true);
        clearInterval(reconnectInterval);
        reconnectInterval = null;
        
        showStatusMessage('Connected to server', 'success');
        
        // If we have saved authentication, try to auto-login
        const savedAuth = localStorage.getItem('iot_auth');
        if (savedAuth) {
          try {
            const authData = JSON.parse(savedAuth);
            if (authData && authData.token) {
              sendJsonMessage('login', {
                username: authData.username,
                token: authData.token,
                password: 'AUTO_LOGIN_TOKEN' // Special password to indicate token authentication
              });
            }
          } catch (e) {
            console.error('Error parsing saved auth:', e);
          }
        }
        
        // Execute callback if provided (used for login after connection)
        if (typeof callback === 'function') {
          callback();
        }
      };
      
      websocket.onclose = (evt) => {
        console.log('WebSocket Disconnected');
        updateConnectionStatus('Disconnected', false);
        
        if (isAuthenticated) {
          showStatusMessage('Connection lost. Reconnecting...', 'error');
        }
        
        isAuthenticated = false;
        
        // Attempt to reconnect
        if (!reconnectInterval) {
          reconnectInterval = setInterval(() => {
            connectWebSocket();
          }, 5000); // Try every 5 seconds
        }
      };
      
      websocket.onmessage = (evt) => {
        console.log('WebSocket message received');
        handleWebSocketMessage(evt);
      };
      
      websocket.onerror = (evt) => {
        console.error('WebSocket Error:', evt);
        logToConsole('WebSocket error occurred', 'error');
      };
    }
    
    // Handle incoming WebSocket messages
    function handleWebSocketMessage(evt) {
      try {
        // Parse JSON message
        const msg = JSON.parse(evt.data);
        console.log('Received:', msg);
        
        // Process based on message type
        switch (msg.type) {
          case 'login_response':
            handleLoginResponse(msg.data);
            break;
            
          case 'sensor_data':
            updateSensorDisplay(msg.data);
            break;
            
          case 'output_state':
            updateOutputState(msg.data);
            break;
            
          case 'system_status':
            updateSystemStatus(msg.data);
            break;
            
          case 'sensor_update':
            updateSensorDisplay(msg.data);
            break;
            
          case 'error':
            handleError(msg.data);
            break;
            
          default:
            logToConsole(`Unknown message type: ${msg.type}`, 'info');
            break;
        }
      } catch (e) {
        console.error('Error processing message:', e);
        logToConsole(`Error processing message: ${e.message}`, 'error');
      }
    }
    
    // Send JSON message to server
    function sendJsonMessage(type, data = {}) {
      if (!websocket || websocket.readyState !== WebSocket.OPEN) {
        logToConsole('WebSocket not connected', 'error');
        return false;
      }
      
      const message = JSON.stringify({
        version: '1.0',
        type: type,
        data: data
      });
      
      websocket.send(message);
      console.log(`Sent: ${type}`, data);
      return true;
    }
    
    // Login with username and password
    function login(username, password) {
      sendJsonMessage('login', {
        username: username,
        password: password
      });
    }
    
    // Handle login response
    function handleLoginResponse(data) {
      if (data.success) {
        isAuthenticated = true;
        
        // Save authentication data
        localStorage.setItem('iot_auth', JSON.stringify({
          username: document.getElementById('username').value,
          token: data.token
        }));
        
        // Show main interface
        showMainInterface();
        
        // Request initial data
        sendJsonMessage('get_sensor_data');
        sendJsonMessage('system_status');
        
        logToConsole('Logged in successfully', 'success');
      } else {
        loginError.textContent = data.message || 'Login failed';
        loginError.style.display = 'block';
        
        logToConsole(`Login failed: ${data.message || 'Unknown error'}`, 'error');
      }
    }
    
    // Logout function
    function logout() {
      // Clear saved authentication
      localStorage.removeItem('iot_auth');
      isAuthenticated = false;
      
      // Show login interface
      showLoginInterface();
      
      logToConsole('Logged out', 'info');
    }
    
    // Show main interface after login
    function showMainInterface() {
      loginContainer.style.display = 'none';
      mainContainer.style.display = 'block';
    }
    
    // Show login interface after logout
    function showLoginInterface() {
      loginContainer.style.display = 'block';
      mainContainer.style.display = 'none';
      loginError.style.display = 'none';
    }
    
    // Update sensor display with new data
    function updateSensorDisplay(data) {
      document.getElementById('temperature').textContent = data.temperature.toFixed(1);
      document.getElementById('humidity').textContent = data.humidity.toFixed(1);
      document.getElementById('last-update').textContent = new Date().toLocaleTimeString();
      
      logToConsole(`Sensor update: Temp=${data.temperature.toFixed(1)}°C, Humidity=${data.humidity.toFixed(1)}%`, 'info');
    }
    
    // Update output state display
    function updateOutputState(data) {
      if (data.success) {
        const checkbox = document.getElementById(`gpio-${data.pin}`);
        if (checkbox) {
          checkbox.checked = data.state;
        }
        
        logToConsole(`GPIO ${data.pin} set to ${data.state ? 'ON' : 'OFF'}`, 'success');
      }
    }
    
    // Update system status display
    function updateSystemStatus(data) {
      document.getElementById('uptime').textContent = data.uptime;
      document.getElementById('free-memory').textContent = data.heap;
      document.getElementById('cpu-freq').textContent = data.cpu_freq;
      document.getElementById('ip-address').textContent = data.ip;
      
      logToConsole(`System status updated: Uptime=${data.uptime}s, Heap=${data.heap}`, 'info');
    }
    
    // Handle error messages
    function handleError(data) {
      const message = data.message || 'An error occurred';
      const code = data.code || 0;
      
      showStatusMessage(`Error ${code}: ${message}`, 'error');
      logToConsole(`Error ${code}: ${message}`, 'error');
      
      // Handle auth errors
      if (code === 401) {
        // Clear saved auth and show login
        localStorage.removeItem('iot_auth');
        isAuthenticated = false;
        showLoginInterface();
      }
    }
    
    // Update connection status display
    function updateConnectionStatus(status, connected = false) {
      connectionStatus.textContent = status;
      
      if (connected) {
        connectionIndicator.className = 'status-indicator connected';
      } else {
        connectionIndicator.className = 'status-indicator disconnected';
      }
    }
    
    // Show temporary status message
    function showStatusMessage(message, type = 'info') {
      statusMessage.textContent = message;
      statusMessage.style.display = 'block';
      
      // Set color based on type
      switch(type) {
        case 'error':
          statusMessage.style.borderLeftColor = '#F44336';
          break;
        case 'success':
          statusMessage.style.borderLeftColor = '#4CAF50';
          break;
        case 'warning':
          statusMessage.style.borderLeftColor = '#FF9800';
          break;
        default:
          statusMessage.style.borderLeftColor = '#2196F3';
      }
      
      // Auto-hide after 5 seconds
      setTimeout(() => {
        statusMessage.style.display = 'none';
      }, 5000);
    }
    
    // Show error message on login form
    function showError(message) {
      loginError.textContent = message;
      loginError.style.display = 'block';
    }
    
    // Log message to console display
    function logToConsole(message, level = 'info') {
      const log = document.getElementById('log');
      const logItem = document.createElement('div');
      logItem.className = `log-item ${level}`;
      
      const timestamp = document.createElement('span');
      timestamp.className = 'log-timestamp';
      timestamp.textContent = new Date().toLocaleTimeString();
      
      logItem.appendChild(timestamp);
      logItem.appendChild(document.createTextNode(' ' + message));
      
      log.appendChild(logItem);
      log.scrollTop = log.scrollHeight;
      
      // Limit log size
      while (log.children.length > 100) {
        log.removeChild(log.children[0]);
      }
    }
  </script>
</body>
</html>
```

## Build and Deploy Instructions

1. **Prerequisites**
   - Arduino IDE or PlatformIO with ESP32 support
   - Required libraries:
     - AsyncTCP
     - ESPAsyncWebServer
     - ArduinoJson (v6.x)

2. **Server Setup**
   - Copy the ESP32 code to your project
   - Update WiFi credentials and authentication details
   - Create a `data` folder and copy the HTML file as `index.html`
   - Upload the filesystem image to SPIFFS before uploading the main program

3. **File Structure**
   ```
   project/
   ├── WebSocketServerTemplate.ino   # Main Arduino sketch
   └── data/
       └── index.html                # Web interface
   ```

4. **ESP32 Board Configuration**
   - Board: "ESP32 Dev Module" or your specific board
   - Flash Size: "4MB (1MB SPIFFS)"
   - Upload Speed: 921600
   - CPU Frequency: 240MHz

5. **Upload Process**
   - First upload SPIFFS data: Tools > ESP32 Sketch Data Upload
   - Then upload the main program
   - Monitor serial output at 115200 baud

6. **Usage**
   - Connect to the ESP32's IP address using a web browser
   - Login with the default credentials (admin/password)
   - Monitor and control your IoT device

7. **Customization**
   - Add additional sensors by implementing the reading functions
   - Add more controls for different GPIO pins and functions
   - Extend the WebSocket message types for your specific project needs
