# Sensor DTO Examples

## Sensor Request (Specific)

```json
{
  "version": "1.0",
  "type": "sensor_request",
  "data": {
    "sensors": ["gyro", "accel"]
  }
}
```

## Sensor Request (All)

```json
{
  "version": "1.0",
  "type": "sensor_request",
  "data": {
    "sensors": ["all"]
  }
}
```

## Gyroscope Data Response

```json
{
  "version": "1.0",
  "type": "sensor_gyro",
  "data": {
    "x": 0.25,
    "y": -1.75,
    "z": 0.05,
    "timestamp": 1717082456789
  }
}
```

## Accelerometer Data Response

```json
{
  "version": "1.0",
  "type": "sensor_accel",
  "data": {
    "x": 0.05,
    "y": 0.07,
    "z": 1.02,
    "timestamp": 1717082456789
  }
}
```

## Temperature Data Response

```json
{
  "version": "1.0",
  "type": "sensor_temperature",
  "data": {
    "cpu": 42.5,
    "ambient": 25.3,
    "timestamp": 1717082456789
  }
}
```

## Battery Data Response

```json
{
  "version": "1.0",
  "type": "sensor_battery",
  "data": {
    "level": 78.5,
    "voltage": 3.85,
    "charging": false,
    "timestamp": 1717082456789
  }
}
```

## Enable Continuous Sensor Updates

```json
{
  "version": "1.0",
  "type": "sensor_updates_control",
  "data": {
    "enabled": true,
    "interval": 500,
    "sensors": ["gyro", "accel", "temperature"]
  }
}
```

## Disable Continuous Sensor Updates

```json
{
  "version": "1.0",
  "type": "sensor_updates_control",
  "data": {
    "enabled": false
  }
}
```
