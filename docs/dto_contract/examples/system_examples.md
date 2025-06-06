# System DTO Examples

## System Status Request

```json
{
  "version": "1.0",
  "type": "system_status_request",
  "data": {}
}
```

## System Status Response

```json
{
  "version": "1.0",
  "type": "system_status",
  "data": {
    "uptime": 3625,
    "freeHeap": 125764,
    "totalHeap": 512000,
    "freePsram": 2048576,
    "totalPsram": 4194304,
    "cpuFreq": 240,
    "cpuTemp": 42.8,
    "sdkVersion": "v4.4.1",
    "firmwareVersion": "1.2.0",
    "ipAddress": "192.168.1.105",
    "macAddress": "24:0A:C4:1D:3E:F8",
    "rssi": -62
  }
}
```

## System Reboot Request

```json
{
  "version": "1.0",
  "type": "system_reboot",
  "data": {
    "delay": 3000
  }
}
```

## System Reset Request (WiFi Settings)

```json
{
  "version": "1.0",
  "type": "system_reset",
  "data": {
    "type": "wifi",
    "confirm": true
  }
}
```

## System Reset Request (Factory Reset)

```json
{
  "version": "1.0",
  "type": "system_reset",
  "data": {
    "type": "factory",
    "confirm": true
  }
}
```

## System Log Message

```json
{
  "version": "1.0",
  "type": "system_log",
  "data": {
    "level": "info",
    "message": "Camera initialized successfully",
    "timestamp": 1717082395123,
    "component": "camera"
  }
}
```

## Error Log Message

```json
{
  "version": "1.0",
  "type": "system_log",
  "data": {
    "level": "error",
    "message": "Failed to connect to WiFi network",
    "timestamp": 1717082391234,
    "component": "network"
  }
}
```
