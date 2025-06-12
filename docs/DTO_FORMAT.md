# Data Transfer Object (DTO) Format

This document outlines the standardized format for WebSocket communication between the Cozmo-System robot platform and its clients. The DTO format ensures consistency and reliability in data exchange.

## General Structure

All WebSocket messages must adhere to the following JSON structure:

```json
{
  "version": "1.0",
  "type": "command_type",
  "data": {
    // Command-specific properties
  }
}
```

### Fields

- **`version`**: Specifies the protocol version. Current version is `1.0`.
- **`type`**: Indicates the type of command or message being sent.
- **`data`**: Contains the payload specific to the command type.

## Command Types

### Motor Control

Used to control the robot's movement.

```json
{
  "type": "motor_command",
  "data": {
    "direction": "forward",
    "speed": 50
  }
}
```

- **`direction`**: Direction of movement (`forward`, `backward`, `left`, `right`).
- **`speed`**: Speed percentage (0-100).

### Camera Control

Used to manage the camera's functionality.

```json
{
  "type": "camera_command",
  "data": {
    "action": "start_stream",
    "resolution": "720p"
  }
}
```

- **`action`**: Camera action (`start_stream`, `stop_stream`, `capture_snapshot`).
- **`resolution`**: Desired resolution (`480p`, `720p`, `1080p`).

### System Status

Requests system information.

```json
{
  "type": "system_status",
  "data": {}
}
```

### File Operations

Manages files on the robot's file system.

#### List Files

```json
{
  "type": "list_files",
  "data": {
    "directory": "/"
  }
}
```

- **`directory`**: Directory path to list files from.

#### Read File

```json
{
  "type": "read_file",
  "data": {
    "file_path": "/config/settings.json"
  }
}
```

- **`file_path`**: Path of the file to read.

#### Delete File

```json
{
  "type": "delete_file",
  "data": {
    "file_path": "/logs/old.log"
  }
}
```

- **`file_path`**: Path of the file to delete.

### WiFi Management

Manages WiFi connections.

#### List Networks

```json
{
  "type": "wifi_list",
  "data": {}
}
```

#### Connect to Network

```json
{
  "type": "connect_wifi",
  "data": {
    "ssid": "MyNetwork",
    "password": "password123"
  }
}
```

- **`ssid`**: WiFi network name.
- **`password`**: WiFi network password.

## Error Handling

All error responses follow this structure:

```json
{
  "version": "1.0",
  "type": "error",
  "data": {
    "code": 400,
    "message": "Invalid command type."
  }
}
```

- **`code`**: HTTP-like status code indicating the error type.
- **`message`**: Description of the error.

## Examples

### Successful Response

```json
{
  "version": "1.0",
  "type": "success",
  "data": {
    "message": "Command executed successfully."
  }
}
```

### Error Response

```json
{
  "version": "1.0",
  "type": "error",
  "data": {
    "code": 404,
    "message": "File not found."
  }
}
```

---

For more details, refer to the [README.md](../README.md) file.
