# Interface API Documentation (IFA)

## Overview

This document describes the HTTP REST API and WebSocket API for the ESP32 Cozmo Robot System. The system provides both traditional HTTP endpoints for file management and configuration, as well as real-time WebSocket communication for robot control and status monitoring.

## Authentication

Most endpoints require authentication. The system uses a simple session-based authentication:

### Login Process
1. Connect to WebSocket endpoint `/ws`
2. Send login message with credentials
3. Receive authentication token for subsequent requests

**Login Message Format:**
```json
{
  "version": "1.0",
  "type": "login",
  "data": {
    "username": "admin",
    "password": "admin"
  }
}
```

**Login Response:**
```json
{
  "version": "1.0",
  "type": "login_response",
  "data": {
    "success": true,
    "token": "auth_token_admin_123456789"
  }
}
```

## HTTP REST API

### Base URL
```
http://<robot-ip>/
```

### Endpoints

#### GET `/`
**Description:** Serves the main web interface  
**Response:** HTML page from SPIFFS (`/index.html`)  
**Content-Type:** `text/html`

#### GET `/css/*` and `/js/*`
**Description:** Serves static assets (CSS and JavaScript files)  
**Response:** Static files from SPIFFS  
**Content-Type:** Varies by file type

#### GET `/api/config`
**Description:** Retrieves current robot configuration  
**Authentication:** Required  
**Response Format:**
```json
{
  "version": "1.0",
  "type": "config",
  "data": {
    // Configuration object with all system settings
  }
}
```

#### POST `/api/config`
**Description:** Updates robot configuration  
**Authentication:** Required  
**Content-Type:** `application/json`  
**Request Body:** JSON configuration object  
**Response:**
```json
{
  "version": "1.0",
  "type": "ok",
  "data": {
    "message": "Configuration updated successfully"
  }
}
```

#### GET `/download?path=<file_path>`
**Description:** Downloads a file from SPIFFS  
**Parameters:**
- `path` (required): Full path to file in SPIFFS  
**Response:** File content with appropriate headers for download  
**Example:** `/download?path=/audio/boot.mp3`

#### POST `/upload`
**Description:** Uploads a file to SPIFFS  
**Authentication:** Required  
**Content-Type:** `multipart/form-data`  
**Parameters:**
- `path` (optional): Target directory path (default: `/`)  
**Response:**
```json
{
  "version": "1.0",
  "type": "ok",
  "data": {
    "message": "File uploaded successfully",
    "filename": "example.mp3",
    "size": 1024
  }
}
```

## WebSocket API

### Connection
```
ws://<robot-ip>/ws
```

### Message Format

All WebSocket messages follow the standardized DTO (Data Transfer Object) format:

```json
{
  "version": "1.0",
  "type": "message_type",
  "data": {
    // Message-specific payload
  }
}
```

### Authentication Messages

#### Login
**Type:** `login`  
**Description:** Authenticate with the robot system  
**Data:**
```json
{
  "username": "admin",
  "password": "admin"
}
```

**Alternative Token Login:**
```json
{
  "username": "admin",
  "token": "saved_token",
  "password": "AUTO_LOGIN_TOKEN"
}
```

### System Status Messages

#### System Status Request
**Type:** `system_status` or `get_status`  
**Description:** Request current system status  
**Data:** Empty object `{}`

**Response:**
```json
{
  "version": "1.0",
  "type": "system_status",
  "data": {
    "wifi": true,
    "wifi_mode": "station",
    "ip": "192.168.1.100",
    "rssi": -45,
    "battery": -1,
    "memory": "245 KB",
    "cpu": "240Mhz",
    "spiffs_total": "1536 KB",
    "spiffs_used": "512 KB",
    "temperature": 25.6,
    "microphone": {
      "enabled": true,
      "level": 30,
      "detected": false
    },
    "speaker": {
      "enabled": true,
      "type": "I2S",
      "playing": false
    },
    "uptime": 3600
  }
}
```

#### Storage Information
**Type:** `storage_info`  
**Description:** Get SPIFFS storage information  
**Response:**
```json
{
  "version": "1.0",
  "type": "storage_info",
  "data": {
    "total": 1572864,
    "used": 524288,
    "free": 1048576,
    "percent": 33.3
  }
}
```

#### Storage Status by Type
**Type:** `get_storage_status`  
**Data:**
```json
{
  "storage_type": "STORAGE_SPIFFS"
}
```

**Response:**
```json
{
  "version": "1.0",
  "type": "storage_status",
  "data": {
    "storage_type": "STORAGE_SPIFFS",
    "available": true,
    "status": "Connected",
    "type": "Internal Flash"
  }
}
```

### Robot Control Messages

#### Motor Control
**Type:** `motor_command`  
**Description:** Control robot movement  
**Data:**
```json
{
  "left": 0.8,      // Left motor speed (-1.0 to 1.0)
  "right": 0.8,     // Right motor speed (-1.0 to 1.0)
  "duration": 1000, // Duration in milliseconds
  "action": "reset" // Optional: "reset" to stop motors
}
```

**Movement Logic:**
- Both positive: Forward
- Both negative: Backward  
- Left positive, Right negative: Turn right
- Left negative, Right positive: Turn left
- Different speeds: Curved movement

**Response:**
```json
{
  "version": "1.0",
  "type": "motor_status",
  "data": {
    "left": 0.8,
    "right": 0.8
  }
}
```

#### Head Control
**Type:** `head_command`  
**Description:** Control head servo movement  
**Data:**
```json
{
  "pan": 90.0,  // Pan angle (0-180 degrees)
  "tilt": 90.0  // Tilt angle (0-180 degrees)
}
```

#### Arm Control
**Type:** `arm_command`  
**Description:** Control arm servo position  
**Data:**
```json
{
  "position": 90.0  // Arm position (0-180 degrees)
}
```

#### Orientation Request
**Type:** `orientation_request`  
**Description:** Request current orientation sensor data  
**Response:** Orientation sensor readings (accelerometer, gyroscope)

### Camera Control Messages

#### Camera Commands
**Type:** `camera_command`  
**Description:** Control camera streaming  

**Start Streaming:**
```json
{
  "action": "start",
  "interval": 33,        // Frame interval in ms (optional)
  "resolution": "vga"    // Resolution: "qvga", "vga", "hd", "sxga" (optional)
}
```

**Stop Streaming:**
```json
{
  "action": "stop"
}
```

**Notes:**
- Camera frames are sent as binary WebSocket messages
- Clients are automatically subscribed/unsubscribed to frame updates
- Streaming stops automatically when no clients are subscribed

### File Operations

#### File Upload Preparation
Binary file uploads via WebSocket require setting up upload state first through a text message, then sending the binary data.

#### File Upload (Binary)
**Type:** Binary WebSocket message  
**Description:** Upload file content as binary data  
**Prerequisites:** Client must be authenticated  
**Process:**
1. Set up upload context (implementation-specific)
2. Send binary data via WebSocket
3. Receive confirmation response

**Response:**
```json
{
  "version": "1.0",
  "type": "file_operation", 
  "data": {
    "success": true,
    "message": "File uploaded successfully",
    "path": "/audio/uploaded_file.mp3",
    "name": "uploaded_file.mp3"
  }
}
```

### WiFi Configuration

#### Get WiFi Networks
**Type:** `get_wifi_networks`  
**Description:** Scan for available WiFi networks  
**Available in:** AP mode and Station mode

#### Get WiFi Configuration
**Type:** `get_wifi_config`  
**Description:** Get current WiFi settings

#### Update WiFi Configuration  
**Type:** `update_wifi_config`  
**Description:** Update WiFi credentials and settings

#### Connect to WiFi
**Type:** `connect_wifi`  
**Description:** Attempt to connect to specified WiFi network

## Error Handling

### HTTP Errors
- `400 Bad Request`: Missing parameters or invalid data
- `401 Unauthorized`: Authentication required
- `403 Forbidden`: Function restricted (e.g., in AP-only mode)
- `404 Not Found`: Resource not found
- `500 Internal Server Error`: Server-side error

### WebSocket Errors
**Error Message Format:**
```json
{
  "version": "1.0",
  "type": "error",
  "data": {
    "code": 401,
    "message": "Authentication required"
  }
}
```

**Success Response Format:**
```json
{
  "version": "1.0", 
  "type": "ok",
  "data": {
    "message": "Operation completed successfully"
  }
}
```

## Access Restrictions

### AP-Only Mode
When the robot is in AP-only mode (not connected to external WiFi), only the following operations are allowed:
- `system_status`
- `get_wifi_networks`  
- `get_wifi_config`
- `update_wifi_config`
- `connect_wifi`

All other commands will return a `403 Forbidden` error.

## Session Management

- Sessions are tracked per WebSocket client ID
- Maximum 5 concurrent sessions supported
- Session data is cleared on client disconnect
- File upload state is automatically cleaned up on disconnect
- Camera subscriptions are managed per session

## Binary Data Handling

### Camera Frames
- Sent as binary WebSocket messages (opcode: WS_BINARY)
- MJPEG format for video streaming
- Clients must subscribe via `camera_command` to receive frames

### File Uploads
- Binary data sent via WebSocket after authentication
- Supports chunked uploads for large files
- Automatic cleanup on client disconnect
- Fallback to default paths if upload context is missing

## Rate Limiting and Performance

- Camera streaming interval configurable (default: 33ms ≈ 30fps)
- Motor commands update manual control timestamp to pause automation
- File operations are synchronous and may block during large transfers
- WebSocket message processing is handled asynchronously via FreeRTOS tasks

## Examples

### Complete Motor Control Session
```javascript
// 1. Connect to WebSocket
const ws = new WebSocket('ws://192.168.1.100/ws');

// 2. Login
ws.send(JSON.stringify({
  version: "1.0",
  type: "login", 
  data: {
    username: "admin",
    password: "admin"
  }
}));

// 3. Move forward for 2 seconds
ws.send(JSON.stringify({
  version: "1.0",
  type: "motor_command",
  data: {
    left: 0.7,
    right: 0.7, 
    duration: 2000
  }
}));

// 4. Get system status
ws.send(JSON.stringify({
  version: "1.0",
  type: "system_status",
  data: {}
}));
```

### File Download via HTTP
```bash
curl "http://192.168.1.100/download?path=/audio/boot.mp3" -o boot.mp3
```

### File Upload via HTTP
```bash
curl -X POST -F "file=@sound.mp3" -F "path=/audio/" "http://192.168.1.100/upload"
```
