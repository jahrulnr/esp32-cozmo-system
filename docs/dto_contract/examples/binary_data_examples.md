# Binary Data Protocol Examples

This document provides examples of the binary data transfer protocol used for camera streaming and file transfers in the IoT Framework Architecture.

## Camera Streaming

### Camera Control Messages

#### Start Camera Stream

```json
{
  "version": "1.0",
  "type": "camera_control",
  "data": {
    "action": "start",
    "resolution": "VGA",
    "quality": 10,
    "frameRate": 15
  }
}
```

#### Configure Camera

```json
{
  "version": "1.0",
  "type": "camera_control",
  "data": {
    "action": "config",
    "resolution": "SVGA",
    "quality": 20,
    "frameRate": 30,
    "rotation": 90
  }
}
```

#### Stop Camera Stream

```json
{
  "version": "1.0",
  "type": "camera_control",
  "data": {
    "action": "stop"
  }
}
```

### Camera Frame Binary Transfer

#### Binary Start Message

```json
{
  "version": "1.0",
  "type": "binary_start",
  "transferId": "cam_frame_1234567890",
  "contentType": "image/jpeg",
  "totalSize": 24580,
  "metadata": {
    "width": 640,
    "height": 480,
    "frameNumber": 45,
    "timestamp": 1624271540000
  }
}
```

## File Transfers

### File Transfer Request

#### Request File Download

```json
{
  "version": "1.0",
  "type": "file_transfer_request",
  "data": {
    "action": "download",
    "fileName": "config.json",
    "path": "/settings/"
  }
}
```

#### Request File Upload

```json
{
  "version": "1.0",
  "type": "file_transfer_request",
  "data": {
    "action": "upload",
    "fileName": "firmware.bin",
    "path": "/firmware/",
    "size": 1048576,
    "overwrite": true,
    "checksum": "d41d8cd98f00b204e9800998ecf8427e"
  }
}
```

#### Request File Listing

```json
{
  "version": "1.0",
  "type": "file_transfer_request",
  "data": {
    "action": "list",
    "path": "/data/"
  }
}
```

#### Request File Deletion

```json
{
  "version": "1.0",
  "type": "file_transfer_request",
  "data": {
    "action": "delete",
    "fileName": "old_log.txt",
    "path": "/logs/"
  }
}
```

### File Transfer Response

#### Download Response (Success)

```json
{
  "version": "1.0",
  "type": "file_transfer_response",
  "data": {
    "success": true,
    "action": "download",
    "fileName": "config.json",
    "transferId": "file_dl_98765432"
  }
}
```

#### Upload Response (Success)

```json
{
  "version": "1.0",
  "type": "file_transfer_response",
  "data": {
    "success": true,
    "action": "upload",
    "fileName": "firmware.bin"
  }
}
```

#### List Response

```json
{
  "version": "1.0",
  "type": "file_transfer_response",
  "data": {
    "success": true,
    "action": "list",
    "fileName": "",
    "fileList": [
      {
        "name": "config.json",
        "size": 2048,
        "lastModified": "2023-04-15T14:33:22Z",
        "isDirectory": false
      },
      {
        "name": "logs",
        "size": 0,
        "lastModified": "2023-04-10T09:45:00Z",
        "isDirectory": true
      },
      {
        "name": "data.csv",
        "size": 156789,
        "lastModified": "2023-04-14T23:11:05Z",
        "isDirectory": false
      }
    ]
  }
}
```

#### Error Response

```json
{
  "version": "1.0",
  "type": "file_transfer_response",
  "data": {
    "success": false,
    "action": "download",
    "fileName": "missing_file.txt",
    "error": "File not found"
  }
}
```

### File Binary Transfer

#### File Download Start

```json
{
  "version": "1.0",
  "type": "binary_start",
  "transferId": "file_dl_98765432",
  "contentType": "application/json",
  "fileName": "config.json",
  "totalSize": 2048,
  "metadata": {
    "path": "/settings/",
    "lastModified": "2023-04-15T14:33:22Z"
  }
}
```

#### File Upload Abort

```json
{
  "version": "1.0",
  "type": "binary_abort",
  "transferId": "file_up_12345678",
  "contentType": "application/octet-stream",
  "fileName": "firmware.bin",
  "totalSize": 1048576,
  "reason": "Disk full"
}
```
