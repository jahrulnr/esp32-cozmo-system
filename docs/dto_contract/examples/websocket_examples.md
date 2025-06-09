# WebSocket DTO Examples

This document provides practical examples of WebSocket messages used in the IoT Framework Architecture.

## Basic Message Structure

All WebSocket messages follow this base structure:

```json
{
  "version": "1.0",
  "type": "message_type",
  "data": {
    // message-specific payload
  }
}
```

## Error Messages

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

```json
{
  "version": "1.0",
  "type": "error",
  "data": {
    "code": 400,
    "message": "Invalid message format"
  }
}
```

## Sensor Data

### Request

```json
{
  "version": "1.0",
  "type": "get_sensor_data",
  "data": {}
}
```

### Response

```json
{
  "version": "1.0",
  "type": "sensor_data",
  "data": {
    "temperature": 25.6,
    "humidity": 48.2,
    "timestamp": 1234567890
  }
}
```

### Automatic Updates

```json
{
  "version": "1.0",
  "type": "sensor_update",
  "data": {
    "temperature": 25.7,
    "humidity": 48.5,
    "timestamp": 1234568000
  }
}
```

## GPIO Control

### Set Output Request

```json
{
  "version": "1.0",
  "type": "set_output",
  "data": {
    "pin": 4,
    "state": true
  }
}
```

### Output State Response

```json
{
  "version": "1.0",
  "type": "output_state",
  "data": {
    "pin": 4,
    "state": true,
    "success": true
  }
}
```

## System Status

### Request

```json
{
  "version": "1.0",
  "type": "system_status",
  "data": {}
}
```

### Response

```json
{
  "version": "1.0",
  "type": "system_status",
  "data": {
    "uptime": 3600,
    "heap": 153624,
    "cpu_freq": 240,
    "wifi_ssid": "MyNetwork",
    "wifi_strength": -67,
    "ip": "192.168.1.100",
    "flash_size": 4194304
  }
}
```
