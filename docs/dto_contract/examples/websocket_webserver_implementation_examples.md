# WebSocket and WebServer Implementation Examples

This document provides practical implementation examples for common use cases of the WebSocket and WebServer IoT Framework Architecture.

## 1. Basic Sensor Monitoring

### Server-Side (ESP32)

```cpp
// In setup()
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/sensor-monitor.html", "text/html");
});

// In handleWebSocketMessage()
if (strcmp(type, "get_sensor_data") == 0) {
  // Read sensor values
  float temperature = readDHTTemperature();
  float humidity = readDHTHumidity();
  
  DynamicJsonDocument response(256);
  response["temperature"] = temperature;
  response["humidity"] = humidity;
  response["timestamp"] = millis();
  
  sendJsonMessage(client, "sensor_data", response);
}

// In loop()
static unsigned long lastSensorUpdate = 0;
if (millis() - lastSensorUpdate > 10000) { // Every 10 seconds
  // Read sensor values
  float temperature = readDHTTemperature();
  float humidity = readDHTHumidity();
  
  DynamicJsonDocument data(256);
  data["temperature"] = temperature;
  data["humidity"] = humidity;
  data["timestamp"] = millis();
  
  // Broadcast to all authenticated clients
  broadcastJsonMessage("sensor_update", data);
  
  lastSensorUpdate = millis();
}
```

### Client-Side (JavaScript)

```javascript
// Connect to WebSocket
const ws = new WebSocket(`ws://${window.location.host}/ws`);

// Handle incoming messages
ws.onmessage = (event) => {
  const message = JSON.parse(event.data);
  
  if (message.type === "sensor_data" || message.type === "sensor_update") {
    // Update UI with sensor data
    document.getElementById('temperature').textContent = message.data.temperature.toFixed(1);
    document.getElementById('humidity').textContent = message.data.humidity.toFixed(1);
    document.getElementById('timestamp').textContent = new Date().toLocaleTimeString();
    
    // Update chart if using visualization
    addDataPoint(chart, message.data.temperature, message.data.humidity);
  }
};

// Request sensor data every 30 seconds
setInterval(() => {
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      version: "1.0",
      type: "get_sensor_data",
      data: {}
    }));
  }
}, 30000);
```

## 2. Home Automation Control

### Server-Side (ESP32)

```cpp
// Device state
struct {
  bool lightState;
  int lightBrightness;
  bool fanState;
  int fanSpeed;
} deviceState;

// In handleWebSocketMessage()
if (strcmp(type, "set_light") == 0) {
  bool state = jsonData["state"];
  int brightness = jsonData["brightness"];
  
  // Update device state
  deviceState.lightState = state;
  deviceState.lightBrightness = brightness;
  
  // Control actual hardware
  digitalWrite(LIGHT_PIN, state ? HIGH : LOW);
  if (state) {
    analogWrite(LIGHT_PWM_PIN, map(brightness, 0, 100, 0, 255));
  }
  
  // Send confirmation
  DynamicJsonDocument response(128);
  response["state"] = state;
  response["brightness"] = brightness;
  response["success"] = true;
  
  sendJsonMessage(client, "light_state", response);
  
  // Also broadcast to all clients to keep UIs in sync
  broadcastJsonMessage("light_update", response);
}
else if (strcmp(type, "set_fan") == 0) {
  bool state = jsonData["state"];
  int speed = jsonData["speed"];
  
  // Update device state
  deviceState.fanState = state;
  deviceState.fanSpeed = speed;
  
  // Control actual hardware
  digitalWrite(FAN_PIN, state ? HIGH : LOW);
  if (state) {
    analogWrite(FAN_PWM_PIN, map(speed, 0, 100, 0, 255));
  }
  
  // Send confirmation
  DynamicJsonDocument response(128);
  response["state"] = state;
  response["speed"] = speed;
  response["success"] = true;
  
  sendJsonMessage(client, "fan_state", response);
  
  // Also broadcast to all clients to keep UIs in sync
  broadcastJsonMessage("fan_update", response);
}
else if (strcmp(type, "get_device_states") == 0) {
  DynamicJsonDocument response(256);
  
  JsonObject lightObj = response.createNestedObject("light");
  lightObj["state"] = deviceState.lightState;
  lightObj["brightness"] = deviceState.lightBrightness;
  
  JsonObject fanObj = response.createNestedObject("fan");
  fanObj["state"] = deviceState.fanState;
  fanObj["speed"] = deviceState.fanSpeed;
  
  sendJsonMessage(client, "device_states", response);
}
```

### Client-Side (JavaScript)

```javascript
// Light control
document.getElementById('light-toggle').addEventListener('change', (e) => {
  const state = e.target.checked;
  const brightness = document.getElementById('light-brightness').value;
  
  ws.send(JSON.stringify({
    version: "1.0",
    type: "set_light",
    data: {
      state: state,
      brightness: parseInt(brightness)
    }
  }));
});

document.getElementById('light-brightness').addEventListener('change', (e) => {
  if (document.getElementById('light-toggle').checked) {
    const brightness = e.target.value;
    
    ws.send(JSON.stringify({
      version: "1.0",
      type: "set_light",
      data: {
        state: true,
        brightness: parseInt(brightness)
      }
    }));
  }
});

// Fan control
document.getElementById('fan-toggle').addEventListener('change', (e) => {
  const state = e.target.checked;
  const speed = document.getElementById('fan-speed').value;
  
  ws.send(JSON.stringify({
    version: "1.0",
    type: "set_fan",
    data: {
      state: state,
      speed: parseInt(speed)
    }
  }));
});

// Update UI based on server messages
ws.onmessage = (event) => {
  const message = JSON.parse(event.data);
  
  switch (message.type) {
    case "light_state":
    case "light_update":
      document.getElementById('light-toggle').checked = message.data.state;
      document.getElementById('light-brightness').value = message.data.brightness;
      document.getElementById('brightness-display').textContent = message.data.brightness + '%';
      break;
      
    case "fan_state":
    case "fan_update":
      document.getElementById('fan-toggle').checked = message.data.state;
      document.getElementById('fan-speed').value = message.data.speed;
      document.getElementById('speed-display').textContent = message.data.speed + '%';
      break;
      
    case "device_states":
      // Update light controls
      document.getElementById('light-toggle').checked = message.data.light.state;
      document.getElementById('light-brightness').value = message.data.light.brightness;
      document.getElementById('brightness-display').textContent = message.data.light.brightness + '%';
      
      // Update fan controls
      document.getElementById('fan-toggle').checked = message.data.fan.state;
      document.getElementById('fan-speed').value = message.data.fan.speed;
      document.getElementById('speed-display').textContent = message.data.fan.speed + '%';
      break;
  }
};

// Request initial states when page loads
ws.onopen = () => {
  ws.send(JSON.stringify({
    version: "1.0",
    type: "get_device_states",
    data: {}
  }));
};
```

## 3. File Upload and Management

### Server-Side (ESP32)

```cpp
// In setup() - Add HTTP endpoints
server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "File uploaded successfully");
}, handleUpload);

server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(4096);
  JsonArray files = doc.createNestedArray("files");
  
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  while (file) {
    JsonObject fileObj = files.createNestedObject();
    fileObj["name"] = file.name();
    fileObj["size"] = file.size();
    file = root.openNextFile();
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
});

// Handle file uploads
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  static File uploadFile;
  
  if (!index) {
    // Start of upload
    Serial.printf("Upload started: %s\n", filename.c_str());
    
    // Open the file for writing
    String path = "/uploads/" + filename;
    uploadFile = SPIFFS.open(path, FILE_WRITE);
    
    // Create a WebSocket notification
    DynamicJsonDocument notif(256);
    notif["filename"] = filename;
    notif["status"] = "started";
    broadcastJsonMessage("upload_status", notif);
  }
  
  // Write data
  if (uploadFile) {
    uploadFile.write(data, len);
  }
  
  if (final) {
    // Upload complete
    if (uploadFile) {
      uploadFile.close();
    }
    
    Serial.printf("Upload complete: %s, size: %u bytes\n", filename.c_str(), index + len);
    
    // Send WebSocket notification
    DynamicJsonDocument notif(256);
    notif["filename"] = filename;
    notif["status"] = "complete";
    notif["size"] = index + len;
    broadcastJsonMessage("upload_status", notif);
  }
}

// Add file list request handling to WebSocket
if (strcmp(type, "get_file_list") == 0) {
  DynamicJsonDocument response(4096);
  JsonArray files = response.createNestedArray("files");
  
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  while (file) {
    JsonObject fileObj = files.createNestedObject();
    fileObj["name"] = file.name();
    fileObj["size"] = file.size();
    file = root.openNextFile();
  }
  
  sendJsonMessage(client, "file_list", response);
}
```

### Client-Side (JavaScript)

```javascript
// File upload form
document.getElementById('upload-form').addEventListener('submit', (e) => {
  e.preventDefault();
  
  const fileInput = document.getElementById('file-input');
  const formData = new FormData();
  
  if (fileInput.files.length === 0) {
    alert('Please select a file to upload');
    return;
  }
  
  formData.append('file', fileInput.files[0]);
  
  fetch('/upload', {
    method: 'POST',
    body: formData
  })
  .then(response => response.text())
  .then(result => {
    console.log(result);
    // Request updated file list
    refreshFileList();
  })
  .catch(error => {
    console.error('Error:', error);
  });
});

// Refresh file list via WebSocket
function refreshFileList() {
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      version: "1.0",
      type: "get_file_list",
      data: {}
    }));
  }
}

// Handle file list updates via WebSocket
ws.onmessage = (event) => {
  const message = JSON.parse(event.data);
  
  if (message.type === "file_list") {
    const fileList = document.getElementById('file-list');
    fileList.innerHTML = '';
    
    message.data.files.forEach(file => {
      const fileItem = document.createElement('div');
      fileItem.className = 'file-item';
      
      const fileName = document.createElement('span');
      fileName.textContent = file.name;
      
      const fileSize = document.createElement('span');
      fileSize.className = 'file-size';
      fileSize.textContent = `${Math.round(file.size / 1024 * 100) / 100} KB`;
      
      const downloadLink = document.createElement('a');
      downloadLink.href = file.name;
      downloadLink.textContent = 'Download';
      downloadLink.target = '_blank';
      
      fileItem.appendChild(fileName);
      fileItem.appendChild(fileSize);
      fileItem.appendChild(downloadLink);
      
      fileList.appendChild(fileItem);
    });
  }
  else if (message.type === "upload_status") {
    const status = message.data.status;
    const filename = message.data.filename;
    
    if (status === "started") {
      showNotification(`Upload started: ${filename}`);
    }
    else if (status === "complete") {
      showNotification(`Upload complete: ${filename} (${Math.round(message.data.size / 1024 * 100) / 100} KB)`);
      refreshFileList();
    }
  }
};

// Initial file list request when page loads
ws.onopen = () => {
  refreshFileList();
};
```
