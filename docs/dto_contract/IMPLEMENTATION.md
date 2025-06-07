# Cozmo-System: Implementasi Kontrak DTO

Dokumen ini menjelaskan cara mengimplementasikan Kontrak DTO dalam kode Server (Go) dan Mikrokontroler (C++/Arduino).

## Server (Go) Implementasi

### 1. Struktur DTO Dasar

```go
// internal/models/dto.go

package models

// BaseDTO adalah struktur dasar untuk semua pesan DTO
type BaseDTO struct {
    Version string      `json:"version"`
    Type    string      `json:"type"`
    Data    interface{} `json:"data"`
}

// NewBaseDTO membuat instance baru BaseDTO dengan versi default
func NewBaseDTO(msgType string, data interface{}) *BaseDTO {
    return &BaseDTO{
        Version: "1.0",
        Type:    msgType,
        Data:    data,
    }
}
```

### 2. DTO Autentikasi

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

### 3. Handler WebSocket

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

## Mikrokontroler (C++/Arduino) Implementasi

### 1. DTO Parsing dan Serialisasi

```cpp
// lib/Communication/DTOParser.h

#pragma once

#include <ArduinoJson.h>
#include "Utils/SpiAllocator.h"

namespace Communication {

class DTOParser {
public:
    // Parse pesan WebSocket menjadi DTO
    static bool parse(const String& jsonString, Utils::SpiJsonDocument& doc) {
        DeserializationError error = deserializeJson(doc, jsonString);
        return !error;
    }
    
    // Ambil tipe pesan dari DTO
    static String getType(const Utils::SpiJsonDocument& doc) {
        if (!doc.containsKey("type") || !doc.containsKey("version")) {
            return "";
        }
        return doc["type"].as<String>();
    }
    
    // Buat DTO baru dengan tipe dan data tertentu
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

### 2. Data Classes

```cpp
// lib/Communication/DTOData.h

#pragma once

#include <ArduinoJson.h>

namespace Communication {

// Base class untuk semua data DTO
class DTOData {
public:
    virtual ~DTOData() {}
    virtual void serialize(JsonObject& obj) const = 0;
};

// Kelas untuk data status sistem
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

// Kelas untuk data gyroscope
class GyroData : public DTOData {
public:
    float x;
    float y;
    float z;
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

### 3. WebSocket Handler

```cpp
// app/websocket.cpp

#include "app.h"
#include "Communication/DTOParser.h"
#include "Utils/SpiAllocator.h"

// Handler untuk pesan WebSocket dari klien
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

// Contoh handler status sistem
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

## Penggunaan pada Frontend (JavaScript)

```javascript
// data/js/dto.js

// Utilitas untuk membuat dan memproses DTO
class DTOUtil {
    // Buat DTO baru dengan tipe dan data tertentu
    static createDTO(type, data = {}) {
        return {
            version: "1.0",
            type: type,
            data: data
        };
    }
    
    // Validasi DTO yang diterima
    static validateDTO(dto) {
        return dto && dto.version && dto.type && dto.data !== undefined;
    }
}

// Contoh penggunaan:
const loginDTO = DTOUtil.createDTO("auth_login", {
    username: "admin",
    password: "password123"
});

websocket.send(JSON.stringify(loginDTO));
```

## Contoh Alur Komunikasi Lengkap

Berikut adalah contoh alur komunikasi lengkap antara frontend, server, dan mikrokontroler:

1. **Pengguna membuat permintaan login dari browser**:
   ```javascript
   // Browser
   const loginDTO = {
       version: "1.0",
       type: "auth_login",
       data: {
           username: "admin",
           password: "password123"
       }
   };
   websocket.send(JSON.stringify(loginDTO));
   ```

2. **Server menerima dan memproses permintaan login**:
   ```go
   // Server (Go)
   func (h *WebSocketHandler) handleAuthLogin(data interface{}, conn *websocket.Conn) {
       // Parse data login
       // Autentikasi user
       // Buat token
       
       // Kirim respons
       response := models.AuthLoginResponse(true, "generated-token", "")
       jsonResponse, _ := json.Marshal(response)
       conn.WriteMessage(websocket.TextMessage, jsonResponse)
   }
   ```

3. **Browser menerima respons dan menyimpan token**:
   ```javascript
   // Browser
   websocket.onmessage = (event) => {
       const response = JSON.parse(event.data);
       
       if (response.type === "auth_login_response") {
           if (response.data.success) {
               localStorage.setItem("auth_token", response.data.token);
               showDashboard();
           } else {
               showError(response.data.message);
           }
       }
   };
   ```

4. **Pengguna mengirim perintah motor ke robot**:
   ```javascript
   // Browser
   const motorCommand = {
       version: "1.0",
       type: "robot_motor",
       data: {
           left: 75,
           right: 75,
           duration: 2000
       }
   };
   websocket.send(JSON.stringify(motorCommand));
   ```

5. **Server meneruskan perintah ke robot**:
   ```go
   // Server (Go)
   func (h *WebSocketHandler) handleRobotMotorCommand(data interface{}, conn *websocket.Conn) {
       // Parse data perintah motor
       // Teruskan ke robot yang terhubung
       robotMsg := models.NewBaseDTO("robot_motor", data)
       jsonMsg, _ := json.Marshal(robotMsg)
       h.service.BroadcastToRobots(jsonMsg)
   }
   ```

6. **Robot menerima dan menjalankan perintah motor**:
   ```cpp
   // Mikrokontroler (ESP32)
   void handleMotorCommand(AsyncWebSocketClient *client, const Utils::SpiJsonDocument& doc) {
       JsonObject data = doc["data"];
       
       int leftPower = data["left"];
       int rightPower = data["right"];
       int duration = data["duration"];
       
       // Jalankan motor dengan parameter yang diberikan
       motorController.setMotorPower(leftPower, rightPower, duration);
       
       // Kirim konfirmasi
       client->text("{\"version\":\"1.0\",\"type\":\"command_result\",\"data\":{\"success\":true}}");
   }
   ```

Implementasi ini memastikan bahwa semua komponen sistem berkomunikasi dengan format yang konsisten dan sesuai dengan kontrak DTO yang telah didefinisikan.
