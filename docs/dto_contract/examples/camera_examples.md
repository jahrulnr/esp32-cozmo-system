# Camera DTO Examples

## Camera Start Request

```json
{
  "version": "1.0",
  "type": "camera_start",
  "data": {
    "resolution": "VGA",
    "quality": 20,
    "fps": 15
  }
}
```

## Camera Stop Request

```json
{
  "version": "1.0",
  "type": "camera_stop",
  "data": {}
}
```

## Camera Setting Request

```json
{
  "version": "1.0",
  "type": "camera_setting",
  "data": {
    "setting": "brightness",
    "value": 75
  }
}
```

## Flash/LED Setting

```json
{
  "version": "1.0",
  "type": "camera_setting",
  "data": {
    "setting": "flash",
    "value": true
  }
}
```

## Camera Frame Header

This is sent immediately before binary frame data:

```json
{
  "version": "1.0",
  "type": "camera_frame",
  "data": {
    "frameId": 12345,
    "timestamp": 1717082430123,
    "size": 24680,
    "format": "jpeg",
    "width": 640,
    "height": 480
  }
}
```

## Camera Snapshot Request

```json
{
  "version": "1.0",
  "type": "camera_snapshot",
  "data": {
    "filename": "snapshot_20250605_123045.jpg"
  }
}
```
