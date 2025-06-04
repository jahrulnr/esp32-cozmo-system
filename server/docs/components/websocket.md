# WebSocket Component

This document describes the WebSocket component of the Cozmo Web Interface.

## Overview

The WebSocket component provides real-time bidirectional communication through two dedicated endpoints:

- `/ws/browser`: For web client connections
- `/ws/robot`: For Cozmo robot connections

The component is responsible for:
- Managing separate WebSocket connections for browsers and robots
- Authenticating and tracking robot connections by unique IDs
- Routing messages between browsers and robots
- Maintaining connection state and handling reconnections

## Components

### CozmoWebSocketService

`CozmoWebSocketService` handles WebSocket communication with Cozmo robots:

```go
// CozmoWebSocketService handles WebSocket communication with browsers and robots
type CozmoWebSocketService struct {
	browsers          map[*websocket.Conn]bool      // Browser client connections
	robots            map[string]*websocket.Conn    // Robot connections mapped by ID
	browserRegister   chan *websocket.Conn          // Channel for registering browser connections
	browserUnregister chan *websocket.Conn          // Channel for unregistering browser connections
	broadcast         chan struct {                 // Channel for broadcasting messages
		robotID  string
		message  []byte
	}
	mutex             sync.RWMutex                  // Read-write mutex for thread safety
	config            config.WebSocketConfig        // WebSocket configuration
}
```

#### Methods

Browser Connection Methods:
- `RegisterBrowserClient(client *websocket.Conn)`: Registers a browser WebSocket connection
- `UnregisterBrowserClient(client *websocket.Conn)`: Unregisters a browser WebSocket connection
- `BroadcastToBrowsers(robotID string, message []byte)`: Broadcasts a message to all connected browsers

Robot Connection Methods:
- `RegisterRobotClient(robotID string, conn *websocket.Conn)`: Registers a robot WebSocket connection
- `UnregisterRobotClient(robotID string)`: Unregisters a robot WebSocket connection
- `BroadcastToRobots(message []byte)`: Broadcasts a message to all connected robots
- `SendToRobot(robotID string, message []byte) error`: Sends a message to a specific robot

Service Management:
- `NewCozmoWebSocketService(cfg config.WebSocketConfig) *CozmoWebSocketService`: Creates a new WebSocket service
- `Run()`: Starts the WebSocket service and handles message routing
- `GetConnectedRobots() []string`: Returns a list of connected robot IDs

### WebSocketHandler

`WebSocketHandler` handles WebSocket connections from both browsers and Cozmo robots:

```go
// WebSocketHandler handles WebSocket connections
type WebSocketHandler struct {
	service  *services.CozmoWebSocketService
	upgrader websocket.Upgrader
}
```

#### Methods

- `NewWebSocketHandler(service *services.CozmoWebSocketService) *WebSocketHandler`: Creates a new WebSocket handler
- `HandleBrowser(c *gin.Context)`: Handles WebSocket connections from browsers
- `HandleRobot(c *gin.Context)`: Handles WebSocket connections from Cozmo robots

## WebSocket Flow

1. **Browser Client Connection Flow**:
   ```
   Browser -> /ws/browser -> WebSocketHandler.HandleBrowser -> 
   Upgrade Connection -> Validate JWT Token ->
   CozmoWebSocketService.RegisterBrowserClient -> 
   Listen for Messages -> Process Messages -> Forward to Robot(s)
   ```

2. **Cozmo Robot Connection Flow**:
   ```
   Cozmo Robot -> /ws/robot?id=robotID -> WebSocketHandler.HandleRobot -> 
   Validate Robot ID -> Upgrade Connection -> 
   CozmoWebSocketService.RegisterRobotClient(robotID) -> 
   Listen for Messages -> Process Messages -> Forward to Browsers
   ```

3. **Browser to Robot Message Flow**:
   ```
   Browser Client -> Message with robotID -> WebSocketHandler.HandleBrowser -> 
   Parse Message -> Validate Target Robot -> 
   CozmoWebSocketService.SendToRobot/BroadcastToRobots -> Robot(s)
   ```

4. **Robot to Browser Message Flow**:
   ```
   Cozmo Robot -> Message -> WebSocketHandler.HandleRobot -> 
   Add Robot ID -> Format Message -> 
   CozmoWebSocketService.BroadcastToBrowsers -> All Browser Clients
   ```

5. **Robot Reconnection Flow**:
   ```
   Robot Connection Lost -> Robot Initiates Reconnection -> 
   /ws/robot?id=robotID -> WebSocketHandler.HandleRobot -> 
   Close Existing Connection -> RegisterRobotClient -> Resume Operation
   ```

## Message Types

### Browser to Server Messages

```json
{
  "type": "string",      // Message type (e.g., "control", "action", "camera")
  "robotId": "string",   // Target robot ID
  "content": "string"    // Message content
}
```

#### Message Types

1. **Browser Authentication**:
   ```json
   {
     "type": "auth",
     "content": "jwt-token"
   }
   ```

2. **Control Commands**:
   ```json
   {
     "type": "control",
     "robotId": "cozmo1",
     "content": "move_forward_start"
   }
   ```

3. **Actions**:
   ```json
   {
     "type": "action",
     "robotId": "cozmo1",
     "content": "say_hello"
   }
   ```

4. **Camera Controls**:
   ```json
   {
     "type": "camera",
     "robotId": "cozmo1",
     "content": "start"
   }
   ```

### Robot to Server Messages

```json
{
  "type": "string",      // Message type (e.g., "auth", "status", "camera")
  "robotId": "string",   // Robot identifier
  "content": "string"    // Message content (can be JSON string)
}
```

#### Message Types

1. **Robot Authentication**:
   ```json
   {
     "type": "auth",
     "robotId": "cozmo1",
     "content": "robot-api-key"
   }
   ```

2. **Status Updates**:
   ```json
   {
     "type": "status",
     "robotId": "cozmo1",
     "content": "{\"battery\":85,\"temperature\":45,\"online\":true}"
   }
   ```

3. **Camera Feed**:
   ```json
   {
     "type": "camera",
     "robotId": "cozmo1",
     "content": "base64-encoded-image"
   }
   ```

4. **Sensor Data**:
   ```json
   {
     "type": "sensors",
     "robotId": "cozmo1",
     "content": "{\"gyro\":{\"x\":0,\"y\":0,\"z\":0},\"cliffDetected\":false}"
   }
   ```

### Server to Browser Messages

```json
{
  "type": "string",    // Message type (e.g., "camera", "sensors")
  "robotId": "string", // Source robot ID
  "content": "string"  // Message content (can be JSON string)
}
```

#### Message Types

1. **Camera Feed**:
   ```json
   {
     "type": "camera",
     "content": "base64-encoded-image-or-url"
   }
   ```

2. **Sensor Data**:
   ```json
   {
     "type": "sensors",
     "content": "{\"battery\":85,\"temperature\":45,\"gyro\":{\"x\":0,\"y\":0,\"z\":0},\"cliffDetected\":false}"
   }
   ```

## WebSocket Implementation

The WebSocket implementation uses the `github.com/gorilla/websocket` package:

1. **Upgrader Configuration**:
   ```go
   upgrader: websocket.Upgrader{
       ReadBufferSize:  1024,
       WriteBufferSize: 1024,
       CheckOrigin: func(r *http.Request) bool {
           return true // Allow all origins for now
       },
   }
   ```

2. **Connection Upgrade**:
   ```go
   conn, err := h.upgrader.Upgrade(c.Writer, c.Request, nil)
   ```

3. **Message Reading**:
   ```go
   messageType, message, err := conn.ReadMessage()
   ```

4. **Message Writing**:
   ```go
   err := client.WriteMessage(websocket.TextMessage, message)
   ```

## Connection Management

The service maintains separate maps for browser clients and robot connections:

```go
browserClients map[*websocket.Conn]bool      // Browser client connections
cozmoClients   map[*websocket.Conn]string    // Cozmo robot connections with IDs
```

- Browser clients are added to the `browserClients` map when they connect
- Robot clients are added to the `cozmoClients` map with their robotId when they connect
- Connections are removed from their respective maps when they disconnect

## Robot Registration and Identification

Each robot must identify itself when connecting:

1. Robot connects to WebSocket endpoint `/ws/robot`
2. Robot sends an authentication message with its unique ID and API key:
   ```json
   {
     "type": "auth",
     "robotId": "cozmo1",
     "content": "robot-api-key"
   }
   ```
3. Server validates the API key and registers the robot
4. Server maintains the mapping between the WebSocket connection and robot ID
5. All subsequent messages from this connection are associated with this robot ID

## Reconnection Logic

The robots handle their own reconnection logic:

1. When a robot detects a disconnection, it attempts to reconnect
2. The robot implements an exponential backoff strategy for reconnection attempts
3. When reconnected, the robot re-authenticates with its ID
4. The server updates its connection maps with the new connection

## Security Considerations

- **Origin Checking**: Browser connections should have origin checking enabled in production
- **Authentication**: 
  - Browser clients authenticate via JWT before being allowed to send commands
  - Robot clients authenticate via API keys specific to each robot
- **Connection Validation**: Robots can only send messages related to their own robotId
- **Rate Limiting**: Implement rate limiting to prevent DoS attacks
- **TLS**: In production, WebSocket connections should use WSS (WebSocket Secure)
- **API Key Security**: Robot API keys should be securely stored and rotated periodically

## Frontend Integration

The frontend uses the native WebSocket API to connect to the browser endpoint:

```javascript
socket = new WebSocket(`ws://${window.location.host}/ws/browser`);

socket.onopen = function() {
    isConnected = true;
    
    // Send authentication message
    socket.send(JSON.stringify({
        type: 'auth',
        content: token
    }));
};

// Handling robot selection
function selectRobot(robotId) {
    currentRobotId = robotId;
    $('#current-robot-id').text(robotId);
    
    // Get status for the selected robot
    socket.send(JSON.stringify({
        type: 'status',
        robotId: robotId,
        content: 'request'
    }));
}

// Sending commands to a specific robot
function sendCommand(command) {
    if (!currentRobotId) {
        showError('No robot selected');
        return;
    }
    
    socket.send(JSON.stringify({
        type: 'control',
        robotId: currentRobotId,
        content: command
    }));
}
```

## Robot Integration

The Cozmo robot uses a WebSocket client to connect to the server:

```cpp
// Connect to server
void connectToServer() {
    if (WiFi.status() == WL_CONNECTED && !wsConnected) {
        // Connect to WebSocket server
        if (webSocket.begin(serverHost, serverPort, "/ws/robot")) {
            webSocket.onEvent(onWebSocketEvent);
            webSocket.setReconnectInterval(5000);
            Serial.println("WebSocket client connected");
        } else {
            Serial.println("WebSocket connection failed");
        }
    }
}

// Authentication on connect
void authenticateRobot() {
    // Create authentication message
    DynamicJsonDocument doc(1024);
    doc["type"] = "auth";
    doc["robotId"] = robotId;
    doc["content"] = robotApiKey;
    
    String message;
    serializeJson(doc, message);
    webSocket.sendTXT(message);
}
```

## Future Enhancements

- **Robot Discovery**: Implement automatic discovery of available robots
- **Multiple Devices per User**: Allow users to control multiple robots
- **Robot Availability Status**: Show online/offline status of robots in real-time
- **Group Commands**: Send commands to groups of robots
- **Permission Management**: Granular permissions for robot access
- **Peer-to-Peer Options**: Direct connections between browser and robot when on same network
- **Message Priority**: Prioritize critical messages like emergency stops
- **Robot Categories**: Support for different robot types/models
- **Connection Analytics**: Monitor and report on connection quality and reliability

- Binary message support for more efficient camera streaming
- Multiple robot support
- Connection status monitoring and automatic recovery
- Message queuing for offline operation
- End-to-end encryption for secure communication
