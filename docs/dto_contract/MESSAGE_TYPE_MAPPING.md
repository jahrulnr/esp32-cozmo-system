# Message Type Mapping

This document defines the standardized message types used in the Cozmo-System platform's WebSocket communication.

## Core Message Types

### System Messages

| Type | Description | Direction |
|------|-------------|-----------|
| `system_status` | System health and metrics | Server → Client |
| `error` | Error notifications | Bidirectional |
| `ping` | Connection testing | Bidirectional |
| `pong` | Connection response | Bidirectional |

### Authentication Messages

| Type | Description | Direction |
|------|-------------|-----------|
| `login` | Authentication request | Client → Server |
| `login_response` | Authentication result | Server → Client |
| `logout` | Session termination | Client → Server |

### Motion Control Messages

| Type | Description | Direction |
|------|-------------|-----------|
| `motion_control` | Movement commands | Client → Server |
| `motion_status` | Movement state updates | Server → Client |
| `servo_control` | Servo position control | Client → Server |
| `servo_status` | Servo position updates | Server → Client |

### Camera Messages

| Type | Description | Direction |
|------|-------------|-----------|
| `camera_start` | Start video stream | Client → Server |
| `camera_stop` | Stop video stream | Client → Server |
| `camera_frame` | Video frame data | Server → Client |
| `camera_config` | Camera settings | Client → Server |

### Sensor Messages

| Type | Description | Direction |
|------|-------------|-----------|
| `sensor_data` | Combined sensor readings | Server → Client |
| `motion_sensor` | Gyroscope/accelerometer data | Server → Client |
| `temperature` | Temperature readings | Server → Client |
| `battery` | Battery status | Server → Client |

### File Operations

| Type | Description | Direction |
|------|-------------|-----------|
| `file_list` | Directory listing | Bidirectional |
| `file_upload` | File transfer initiation | Client → Server |
| `file_data` | File content transfer | Bidirectional |
| `file_status` | Operation status | Server → Client |

## Message Structure

All messages follow the standard DTO format:

```json
{
  "version": "1.0",
  "type": "message_type",
  "data": {
    // Type-specific payload
  }
}
```

## Type-Specific Formats

### System Status

```json
{
  "version": "1.0",
  "type": "system_status",
  "data": {
    "uptime": 3600,
    "memory": {
      "free": 153624,
      "total": 4194304
    },
    "cpu": {
      "frequency": 240,
      "temperature": 45.2
    },
    "network": {
      "ssid": "NetworkName",
      "rssi": -67,
      "ip": "192.168.1.100"
    }
  }
}
```

### Motion Control

```json
{
  "version": "1.0",
  "type": "motion_control",
  "data": {
    "direction": "forward",
    "speed": 75,
    "duration": 2000
  }
}
```

### Camera Configuration

```json
{
  "version": "1.0",
  "type": "camera_config",
  "data": {
    "resolution": "720p",
    "framerate": 30,
    "quality": 75,
    "brightness": 0,
    "contrast": 0
  }
}
```

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-06-12 | Initial release |

## Implementation Notes

1. **Type Validation**
   - Validate message type before processing
   - Check required fields for each type
   - Verify data format matches specification

2. **Error Handling**
   - Use appropriate error types
   - Include helpful error messages
   - Maintain consistent error format

3. **Performance Considerations**
   - Minimize message size
   - Use appropriate data types
   - Consider binary formats for large data

---

_Last Updated: June 12, 2025_
