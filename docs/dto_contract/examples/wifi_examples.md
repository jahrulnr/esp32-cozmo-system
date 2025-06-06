# WiFi DTO Examples

## WiFi Network Scan Request

```json
{
  "version": "1.0",
  "type": "wifi_scan",
  "data": {}
}
```

## WiFi Network List Response

```json
{
  "version": "1.0",
  "type": "wifi_networks",
  "data": {
    "networks": [
      {
        "ssid": "HomeWiFi",
        "rssi": -45,
        "encryption": "WPA2",
        "channel": 6,
        "bssid": "AA:BB:CC:DD:EE:FF"
      },
      {
        "ssid": "OfficeNetwork",
        "rssi": -67,
        "encryption": "WPA2_ENTERPRISE",
        "channel": 11,
        "bssid": "11:22:33:44:55:66"
      },
      {
        "ssid": "GuestWiFi",
        "rssi": -72,
        "encryption": "OPEN",
        "channel": 1,
        "bssid": "AA:BB:CC:11:22:33"
      }
    ]
  }
}
```

## WiFi Connect Request

```json
{
  "version": "1.0",
  "type": "wifi_connect",
  "data": {
    "ssid": "HomeWiFi",
    "password": "mywifipassword",
    "save": true
  }
}
```

## WiFi Connect Response (Success)

```json
{
  "version": "1.0",
  "type": "wifi_connect_result",
  "data": {
    "success": true,
    "ssid": "HomeWiFi",
    "ip": "192.168.1.105"
  }
}
```

## WiFi Connect Response (Failure)

```json
{
  "version": "1.0",
  "type": "wifi_connect_result",
  "data": {
    "success": false,
    "ssid": "HomeWiFi",
    "error": "Authentication failed"
  }
}
```

## WiFi Disconnect Request

```json
{
  "version": "1.0",
  "type": "wifi_disconnect",
  "data": {}
}
```

## WiFi Status Request

```json
{
  "version": "1.0",
  "type": "wifi_status_request",
  "data": {}
}
```

## WiFi Status Response

```json
{
  "version": "1.0",
  "type": "wifi_status",
  "data": {
    "connected": true,
    "ssid": "HomeWiFi",
    "ip": "192.168.1.105",
    "gateway": "192.168.1.1",
    "subnet": "255.255.255.0",
    "dns": "8.8.8.8",
    "mac": "24:0A:C4:1D:3E:F8",
    "rssi": -58,
    "channel": 6
  }
}
```
