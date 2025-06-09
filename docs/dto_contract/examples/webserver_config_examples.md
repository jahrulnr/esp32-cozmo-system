# WebServer Configuration Examples

This document provides practical examples of WebServer configuration used in the IoT Framework Architecture.

## Basic Server Configuration

```json
{
  "server": {
    "port": 80,
    "cors": true,
    "maxClients": 10,
    "pingInterval": 10000,
    "sessionTimeout": 3600000
  },
  "auth": {
    "enabled": true,
    "tokenExpiry": 86400000,
    "users": [
      {
        "username": "admin",
        "password": "password123",
        "role": "admin"
      },
      {
        "username": "user",
        "password": "userpass",
        "role": "user"
      }
    ]
  },
  "static": {
    "enabled": true,
    "root": "/data",
    "cache": true,
    "maxAge": 3600
  }
}
```

## Configuration with Custom Routes

```json
{
  "server": {
    "port": 8080,
    "cors": true,
    "maxClients": 5
  },
  "auth": {
    "enabled": true,
    "users": [
      {
        "username": "admin",
        "password": "securepassword",
        "role": "admin"
      }
    ]
  },
  "routes": {
    "api": {
      "path": "/api/v1",
      "method": "GET",
      "requiresAuth": true,
      "handler": "handleApiRequest"
    },
    "publicData": {
      "path": "/data",
      "method": "GET",
      "requiresAuth": false,
      "handler": "handlePublicData"
    },
    "updateSettings": {
      "path": "/settings",
      "method": "POST",
      "requiresAuth": true,
      "handler": "handleSettingsUpdate"
    }
  },
  "static": {
    "enabled": true,
    "root": "/www",
    "cache": true,
    "maxAge": 86400
  }
}
```

## Minimal Configuration (No Authentication)

```json
{
  "server": {
    "port": 80,
    "cors": false
  },
  "auth": {
    "enabled": false
  },
  "static": {
    "enabled": true,
    "root": "/data"
  }
}
```

## Production-Ready Configuration

```json
{
  "server": {
    "port": 443,
    "cors": true,
    "maxClients": 100,
    "pingInterval": 30000,
    "sessionTimeout": 1800000
  },
  "auth": {
    "enabled": true,
    "tokenExpiry": 7200000,
    "users": [
      {
        "username": "systemadmin",
        "password": "$2a$10$p7SHzUF3MkOXK5G4vD1sI.k8hDKZA0VhHi3HoGxUgRjJKdGY3UvdC",
        "role": "admin"
      },
      {
        "username": "monitor",
        "password": "$2a$10$qNYm3otzkSx7X3AcJ8MKtOUjnhJ2FOojxKBCGor8q2GP.zvVrF76.",
        "role": "user"
      }
    ]
  },
  "routes": {
    "api": {
      "path": "/api/v2",
      "method": "GET",
      "requiresAuth": true,
      "handler": "handleApiRequest"
    },
    "metrics": {
      "path": "/metrics",
      "method": "GET",
      "requiresAuth": true,
      "handler": "handleMetricsRequest"
    },
    "firmware": {
      "path": "/firmware/update",
      "method": "POST",
      "requiresAuth": true,
      "handler": "handleFirmwareUpdate"
    }
  },
  "static": {
    "enabled": true,
    "root": "/www",
    "cache": true,
    "maxAge": 604800
  }
}
```
