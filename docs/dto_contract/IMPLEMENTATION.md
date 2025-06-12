# Cozmo-System: DTO Contract Implementation

This document describes the implementation of the DTO Contract in the Server (Go) and Microcontroller (C++/Arduino) components.

## Server (Go) Implementation

### 1. Base DTO Structure

```go
// internal/models/dto.go

package models

// BaseDTO defines the foundation structure for all DTO messages
type BaseDTO struct {
    Version string      `json:"version"`
    Type    string      `json:"type"`
    Data    interface{} `json:"data"`
}

// NewBaseDTO creates a new BaseDTO instance with default version
func NewBaseDTO(msgType string, data interface{}) *BaseDTO {
    return &BaseDTO{
        Version: "1.0",
        Type:    msgType,
        Data:    data,
    }
}
```

### 2. Authentication DTO

```go
// internal/models/auth_dto.go

package models

// AuthLoginRequestData adalah struktur untuk data login request
type AuthLoginRequestData struct {
    Username string `json:"username"`
    Password string `json:"password"`
}

// AuthLoginResponseData adalah struktur untuk data login response
type AuthLoginResponseData struct {
    Success bool   `json:"success"`
    Token   string `json:"token,omitempty"`
    Message string `json:"message,omitempty"`
}

// AuthLoginRequest membuat DTO untuk login request
func AuthLoginRequest(username, password string) *BaseDTO {
    return NewBaseDTO("auth_login", AuthLoginRequestData{
        Username: username,
        Password: password,
    })
}

// AuthLoginResponse membuat DTO untuk login response
func AuthLoginResponse(success bool, token, message string) *BaseDTO {
    return NewBaseDTO("auth_login_response", AuthLoginResponseData{
        Success: success,
        Token:   token,
        Message: message,
    })
}
```

### 3. WebSocket Handler Implementation

```go
// internal/handlers/websocket_handler.go

package handlers

import (
    "encoding/json"
    "log"
    
    "cozmo-clouds/internal/models"
    "cozmo-clouds/internal/services"
)

// handleMessage menangani pesan WebSocket berdasarkan kontrak DTO
func (h *WebSocketHandler) handleMessage(message []byte, conn *websocket.Conn) {
    // Parse pesan sebagai BaseDTO
    var baseDTO models.BaseDTO
    if err := json.Unmarshal(message, &baseDTO); err != nil {
        log.Printf("Error parsing WebSocket message: %v", err)
        return
    }
    
    // Periksa versi DTO
    if baseDTO.Version != "1.0" {
        log.Printf("Unsupported DTO version: %s", baseDTO.Version)
        return
    }
    
    // Tangani berbagai tipe pesan
    switch baseDTO.Type {
    case "auth_login":
        h.handleAuthLogin(baseDTO.Data, conn)
    case "robot_motor":
        h.handleRobotMotorCommand(baseDTO.Data, conn)
    case "camera_start":
        h.handleCameraStartCommand(baseDTO.Data, conn)
    case "system_status_request":
        h.handleSystemStatusRequest(conn)
    case "wifi_scan":
        h.handleWifiScanRequest(conn)
    // Tambahkan handler untuk tipe pesan lainnya
    default:
        log.Printf("Unknown message type: %s", baseDTO.Type)
    }
}
```

### 4. Contoh Handler Spesifik

```go
// Handler untuk perintah motor
func (h *WebSocketHandler) handleRobotMotorCommand(data interface{}, conn *websocket.Conn) {
    // Parse data spesifik dari interface{}
    jsonData, _ := json.Marshal(data)
    var motorData struct {
        Left     int `json:"left"`
        Right    int `json:"right"`
        Duration int `json:"duration"`
    }
    
    if err := json.Unmarshal(jsonData, &motorData); err != nil {
        log.Printf("Error parsing motor command data: %v", err)
        return
    }
    
    // Teruskan perintah ke robot yang terhubung
    robotMsg := models.NewBaseDTO("robot_motor", motorData)
    jsonMsg, _ := json.Marshal(robotMsg)
    h.service.BroadcastToRobots(jsonMsg)
}
```

## Microcontroller (C++/Arduino) Implementation

### 1. DTO Parsing and Serialization

```cpp
// lib/Communication/DTOParser.h

#pragma once

#include <ArduinoJson.h>
#include "Utils/SpiAllocator.h"

namespace Communication {

class DTOParser {
public:
    // Parse WebSocket message into DTO
    static bool parse(const String& jsonString, Utils::SpiJsonDocument& doc) {
        DeserializationError error = deserializeJson(doc, jsonString);
        return !error;
    }
    
    // Extract message type from DTO
    static String getType(const Utils::SpiJsonDocument& doc) {
        if (!doc.containsKey("type") || !doc.containsKey("version")) {
            return "";
        }
        return doc["type"].as<String>();
    }
    
    // Create new DTO with specified type and data
    template<typename T>
    static String createDTO(const String& type, const T& data) {
        Utils::SpiJsonDocument doc;
        doc["version"] = "1.0";
        doc["type"] = type;
        
        JsonObject dataObj = doc.createNestedObject("data");
        data.serialize(dataObj);
        
        String result;
        serializeJson(doc, result);
        return result;
    }
};

} // namespace Communication
```

### 2. DTO Data Classes

```cpp
// lib/Communication/DTOData.h

#pragma once

#include <ArduinoJson.h>

namespace Communication {

// Base class for all DTO data structures
class DTOData {
public:
    virtual ~DTOData() {}
    virtual void serialize(JsonObject& obj) const = 0;
};

// System status data structure
class SystemStatusData : public DTOData {
public:
    uint32_t uptime;
    uint32_t freeHeap;
    uint32_t totalHeap;
    uint32_t freePsram;
    uint32_t totalPsram;
    uint16_t cpuFreq;
    float cpuTemp;
    String sdkVersion;
    String firmwareVersion;
    String ipAddress;
    String macAddress;
    int8_t rssi;
    
    void serialize(JsonObject& obj) const override {
        obj["uptime"] = uptime;
        obj["freeHeap"] = freeHeap;
        obj["totalHeap"] = totalHeap;
        obj["freePsram"] = freePsram;
        obj["totalPsram"] = totalPsram;
        obj["cpuFreq"] = cpuFreq;
        obj["cpuTemp"] = cpuTemp;
        obj["sdkVersion"] = sdkVersion;
        obj["firmwareVersion"] = firmwareVersion;
        obj["ipAddress"] = ipAddress;
        obj["macAddress"] = macAddress;
        obj["rssi"] = rssi;
    }
};

// Motion sensor data structure
class MotionSensorData : public DTOData {
public:
    int16_t x;
    int16_t y;
    int16_t z;
    uint64_t timestamp;
    
    void serialize(JsonObject& obj) const override {
        obj["x"] = x;
        obj["y"] = y;
        obj["z"] = z;
        obj["timestamp"] = timestamp;
    }
};

// Tambahkan kelas lain untuk jenis data yang berbeda...

} // namespace Communication
```

### 3. WebSocket Message Handler

```cpp
// app/websocket.cpp

#include "app.h"
#include "Communication/DTOParser.h"
#include "Utils/SpiAllocator.h"

// Handler for WebSocket messages from clients
void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    
    // Hanya memproses pesan teks lengkap
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        // Parse data ke string
        data[len] = 0;
        String jsonString = String((char*)data);
        
        // Parse ke JSON menggunakan DTOParser
        Utils::SpiJsonDocument doc;
        if (!Communication::DTOParser::parse(jsonString, doc)) {
            Serial.println("Failed to parse WebSocket message");
            return;
        }
        
        // Ambil tipe pesan
        String messageType = Communication::DTOParser::getType(doc);
        if (messageType.length() == 0) {
            Serial.println("Invalid DTO format: missing type or version");
            return;
        }
        
        // Tangani berbagai tipe pesan
        if (messageType == "auth_login") {
            handleAuthLogin(client, doc);
        } 
        else if (messageType == "robot_motor") {
            handleMotorCommand(client, doc);
        }
        else if (messageType == "camera_start") {
            handleCameraStart(client, doc);
        }
        else if (messageType == "system_status_request") {
            handleSystemStatusRequest(client);
        }
        else if (messageType == "wifi_scan") {
            handleWifiScan(client);
        }
        // Tambahkan handler untuk tipe pesan lainnya
        else {
            Serial.printf("Unknown message type: %s\n", messageType.c_str());
        }
    }
}

// System status request handler
void handleSystemStatusRequest(AsyncWebSocketClient *client) {
    // Buat data status sistem
    Communication::SystemStatusData statusData;
    statusData.uptime = millis() / 1000;
    statusData.freeHeap = ESP.getFreeHeap();
    statusData.totalHeap = ESP.getHeapSize();
    statusData.freePsram = ESP.getFreePsram();
    statusData.totalPsram = ESP.getPsramSize();
    statusData.cpuFreq = ESP.getCpuFreqMHz();
    statusData.cpuTemp = getCPUTemperature();
    statusData.sdkVersion = ESP.getSdkVersion();
    statusData.firmwareVersion = FIRMWARE_VERSION;
    statusData.ipAddress = WiFi.localIP().toString();
    statusData.macAddress = WiFi.macAddress();
    statusData.rssi = WiFi.RSSI();
    
    // Kirim respons
    String response = Communication::DTOParser::createDTO("system_status", statusData);
    client->text(response);
}
```

## Web Interface Implementation

### 1. DTO Utilities

```javascript
// data/js/dto.js

// Utilities for creating and processing DTOs
class DTOUtil {
    // Create new DTO with specified type and data
    static createDTO(type, data = {}) {
        return {
            version: "1.0",
            type: type,
            data: data
        };
    }
    
    // Validate received DTO
    static validateDTO(dto) {
        return dto && dto.version && dto.type && dto.data !== undefined;
    }
}
```

## Communication Flow Examples

### 1. Authentication Flow

```javascript
// Client initiates login
const loginDTO = DTOUtil.createDTO("auth_login", {
    username: "admin",
    password: "password123"
});

websocket.send(JSON.stringify(loginDTO));
```

### 2. Motion Control Flow

```javascript
// Client sends motion command
const motionCommand = DTOUtil.createDTO("motion_control", {
    left: 75,
    right: 75,
    duration: 2000
});

websocket.send(JSON.stringify(motionCommand));
```

### 3. System Status Flow

```go
// Server handles system status request
func (h *WebSocketHandler) handleSystemStatusRequest(conn *websocket.Conn) {
    statusData := collectSystemStatus()
    response := models.NewBaseDTO("system_status", statusData)
    jsonResponse, _ := json.Marshal(response)
    conn.WriteMessage(websocket.TextMessage, jsonResponse)
}
```

This implementation ensures consistent communication between all system components according to the defined DTO contract. All message handling follows the standardized format and maintains type safety across the platform.

---

For detailed message type specifications, see [MESSAGE_TYPE_MAPPING.md](MESSAGE_TYPE_MAPPING.md).
