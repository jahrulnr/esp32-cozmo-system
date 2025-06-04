# Cozmo System Integration

This document describes how the Go web server integrates with the Cozmo robot system.

## Overview

The Cozmo Web Interface communicates with the Cozmo robot system through WebSockets. This allows for bidirectional, real-time communication between the web interface and the robot.

## Integration Architecture

```
┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
│                 │      │                 │      │                 │
│  Web Interface  │◄────►│   Go Server     │◄────►│   Cozmo Robot   │
│  (Browser)      │      │  (WebSocket)    │      │  (ESP32)        │
│                 │      │                 │      │                 │
└─────────────────┘      └─────────────────┘      └─────────────────┘
```

## Communication Flow

1. **Client to Server**:
   - Web client connects to Go server via WebSocket
   - Client sends commands to server
   - Server processes and validates commands

2. **Server to Cozmo**:
   - Go server connects to Cozmo system via WebSocket
   - Server forwards validated commands to Cozmo
   - Cozmo executes commands

3. **Cozmo to Server**:
   - Cozmo sends status updates, sensor data, and camera feed to server
   - Server processes the data

4. **Server to Client**:
   - Server broadcasts updates to all connected clients
   - Clients display the updates

## WebSocket Protocol

### Server to Cozmo Commands

The Go server sends commands to the Cozmo robot in JSON format:

```json
{
  "type": "string",
  "content": "string"
}
```

#### Command Types

1. **Movement Commands**:
   ```json
   {
     "type": "control",
     "content": "move_forward_start"
   }
   ```
   Available commands:
   - `move_forward_start`
   - `move_forward_stop`
   - `move_backward_start`
   - `move_backward_stop`
   - `turn_left_start`
   - `turn_left_stop`
   - `turn_right_start`
   - `turn_right_stop`
   - `stop`

2. **Arm Control**:
   ```json
   {
     "type": "control",
     "content": "lift_up"
   }
   ```
   Available commands:
   - `lift_up`
   - `lift_down`

3. **Head Control**:
   ```json
   {
     "type": "control",
     "content": "head_up"
   }
   ```
   Available commands:
   - `head_up`
   - `head_down`

4. **Actions**:
   ```json
   {
     "type": "action",
     "content": "say_hello"
   }
   ```
   Available actions:
   - `say_hello`
   - `dance`
   - `pickup_cube`
   - `play_animation`

5. **Camera Control**:
   ```json
   {
     "type": "camera",
     "content": "start"
   }
   ```
   Available commands:
   - `start`
   - `stop`
   - `capture`

6. **Speech**:
   ```json
   {
     "type": "speak",
     "content": "Hello, I am Cozmo!"
   }
   ```

### Cozmo to Server Messages

The Cozmo robot sends updates to the Go server in JSON format:

```json
{
  "type": "string",
  "content": "string"
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

3. **Status Updates**:
   ```json
   {
     "type": "status",
     "content": "ready"
   }
   ```

4. **Error Messages**:
   ```json
   {
     "type": "error",
     "content": "Motor overheated"
   }
   ```

## Cozmo System Architecture

The Cozmo robot system is built on an ESP32 platform with:

1. **Hardware Components**:
   - Motors for movement
   - Servos for lift and head control
   - Camera for vision
   - Gyro for orientation
   - Cliff sensors for safety

2. **Software Components**:
   - WebServer for communication
   - WebSocket handlers
   - Motor control
   - Servo control
   - Camera management
   - Sensor data collection

## Connection Management

### Initial Connection

1. The Go server attempts to connect to the Cozmo WebSocket endpoint on startup
2. If the connection fails, a reconnection routine is started

### Reconnection Logic

1. When the connection is lost, the `readPump` function exits
2. The `reconnectionRoutine` is started
3. The routine waits for a few seconds and attempts to reconnect
4. If the reconnection fails, it waits and tries again
5. Once reconnected, the `readPump` function is restarted

### Connection Status

The frontend displays the connection status to the user:

```javascript
$('#cozmo-status .status-value').removeClass('disconnected').addClass('connected').text('Connected');
```

## Error Handling

### Server-Side Errors

The Go server handles errors from the Cozmo connection:

1. **Connection Errors**:
   - Log the error
   - Start reconnection routine
   - Notify clients of disconnection

2. **Message Parsing Errors**:
   - Log the error
   - Continue processing other messages

### Client-Side Errors

The frontend handles WebSocket errors:

```javascript
socket.onerror = function(error) {
    console.error('WebSocket error:', error);
    $('#cozmo-status .status-value').removeClass('connected').addClass('disconnected').text('Error');
};
```

## Security Considerations

1. **Authentication**: Ensure only authenticated users can send commands to the Cozmo robot
2. **Command Validation**: Validate commands before forwarding to the robot
3. **Rate Limiting**: Implement rate limiting to prevent command flooding
4. **TLS**: Use secure WebSocket connections (WSS) in production

## Future Enhancements

1. **Command Queueing**: Implement a command queue for better control flow
2. **Command Acknowledgment**: Add acknowledgment system for command execution
3. **Diagnostic Tools**: Add diagnostic tools for troubleshooting
4. **Multiple Robot Support**: Extend the system to support multiple robots
5. **Firmware Updates**: Add the ability to update the Cozmo firmware through the web interface
