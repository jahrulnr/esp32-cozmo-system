# WebSocket Data Transfer Object (DTO) Format

> **PERHATIAN**: Format DTO yang didokumentasikan di sini adalah referensi untuk implementasi saat ini. Untuk format baru yang akan digunakan dalam integrasi server Go dan mikrokontroler, silakan merujuk ke [Kontrak DTO](/docs/dto_contract/README.md).

## Message Format

Semua pesan mengikuti format JSON ini:

```json
{
  "type": "command_type",
  "data": any
}
```

Where:
- `type`: A string identifying the type of command or response
- `data`: Any valid JSON data structure containing the payload

## Message Types

### Commands (Client to Robot)

| Type | Description | Data Structure |
|------|-------------|----------------|
| `motor_command` | Control robot motors | `{ "left": float, "right": float, "duration": int }` |
| `head_command` | Control robot head position | `{ "pan": float, "tilt": float }` |
| `arm_command` | Control robot arm position | `{ "position": float }` |
| `action_command` | Perform predefined actions | `{ "action": string }` |
| `expression_command` | Change robot facial expression | `{ "expression": string }` |
| `gyro_request` | Request gyroscope data | `{}` |
| `camera_command` | Control camera settings | `{ "resolution": string, "quality": int }` |
| `system_command` | System level commands | `{ "action": string }` |
| `ping` | Keep-alive message | `{ "timestamp": int }` |

### Responses (Robot to Client)

| Type | Description | Data Structure |
|------|-------------|----------------|
| `motor_status` | Motor status information | `{ "left": float, "right": float }` |
| `sensor_data` | Combined sensor data | `{ "gyro": { "x": float, "y": float, "z": float }, "accel": { "x": float, "y": float, "z": float, "magnitude": float } }` |
| `camera_frame` | Camera frame data | `{ "format": string, "data": string }` |
| `system_status` | System status information | `{ "battery": float, "temperature": float, "uptime": int }` |
| `error` | Error information | `{ "code": int, "message": string }` |
| `ok` | Success confirmation | `{ "message": string }` |

## Examples

### Command Examples

#### Motor Command
```json
{
  "type": "motor_command",
  "data": {
    "left": 0.75,
    "right": 0.75,
    "duration": 1000
  }
}
```

#### Head Command
```json
{
  "type": "head_command",
  "data": {
    "pan": 45,
    "tilt": 30
  }
}
```

#### Gyroscope Data Request
```json
{
  "type": "gyro_request",
  "data": {}
}
```

### Response Examples

#### Sensor Data (Gyroscope and Accelerometer)
```json
{
  "type": "sensor_data",
  "data": {
    "gyro": {
      "x": 0.01,
      "y": -0.03,
      "z": 0.15
    },
    "accel": {
      "x": 0.03,
      "y": -0.12,
      "z": 0.98,
      "magnitude": 0.99
    }
  }
}
```

#### System Status
```json
{
  "type": "system_status",
  "data": {
    "battery": 85.2,
    "temperature": 42.3,
    "uptime": 3600
  }
}
```

## Error Handling

Error responses use the following format:
```json
{
  "type": "error",
  "data": {
    "code": 404,
    "message": "Command not recognized"
  }
}
```

Common error codes:
- 400: Bad request
- 404: Command not found
- 500: Internal error

## Client-Side Implementation

### JavaScript Example

```javascript
// Send a standardized message
function sendJsonMessage(type, data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        const message = JSON.stringify({
            type: type,
            data: data
        });
        websocket.send(message);
    }
}

// Handle received messages
websocket.onmessage = function(event) {
    // Skip binary messages (like camera frames)
    if (event.data instanceof ArrayBuffer) return;
    
    try {
        const message = JSON.parse(event.data);
        
        // Process based on message type
        switch (message.type) {
            case 'sensor_data':
                updateSensorDisplay(message.data);
                break;
            case 'error':
                showError(message.data.message);
                break;
            // ... handle other types
        }
    } catch (error) {
        console.error('Error parsing WebSocket message:', error);
    }
};

// Example usage
sendJsonMessage('motor_command', { left: 0.5, right: 0.5, duration: 2000 });
```

## C++ Implementation

```cpp
// Send a JSON message using WebSocketHandler
void sendRobotStatus(WebSocketHandler& ws, int clientId) {
    DynamicJsonDocument doc(1024);
    doc["battery"] = getBatteryLevel();
    doc["temperature"] = getProcessorTemp();
    doc["uptime"] = millis() / 1000;
    
    ws.sendJsonMessage(clientId, "system_status", doc.as<JsonVariant>());
}

// Handle received message
void handleWebSocketMessage(uint8_t* data, size_t len) {
    DynamicJsonDocument doc = WebSocketHandler::parseJsonMessage(data, len);
    
    if (doc.isNull()) {
        // Invalid JSON format
        return;
    }
    
    String type = doc["type"];
    JsonVariant messageData = doc["data"];
    
    if (type == "motor_command") {
        float left = messageData["left"];
        float right = messageData["right"];
        int duration = messageData["duration"];
        
        // Execute motor command
        setMotorSpeeds(left, right, duration);
    }
}
```

## API Extensions

When adding new command types, follow these guidelines:
1. Use descriptive type names in snake_case
2. Document the new type and its data structure in this file
3. Include appropriate validation in the implementation
4. Make sure it is properly handled in both the client and server code

## New DTO Contract Format (v1.0)

Untuk standardisasi komunikasi antara server Go dan mikrokontroler ESP32-CAM, kami telah mengembangkan format DTO baru yang mencakup versioning:

```json
{
  "version": "1.0",
  "type": "command_type",
  "data": {
    // Command-specific payload
  }
}
```

Format baru ini akan menggantikan format lama secara bertahap. Dokumentasi lengkap tersedia di:
- [Dokumentasi Kontrak DTO](/docs/dto_contract/README.md)
- [Skema JSON](/docs/dto_contract/schemas/)
- [Contoh Pesan](/docs/dto_contract/examples/)
- [Panduan Implementasi](/docs/dto_contract/IMPLEMENTATION.md)
