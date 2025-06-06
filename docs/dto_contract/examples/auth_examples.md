# Authentication DTO Examples

## Login Request

```json
{
  "version": "1.0",
  "type": "auth_login",
  "data": {
    "username": "admin",
    "password": "password123"
  }
}
```

## Login Response (Success)

```json
{
  "version": "1.0",
  "type": "auth_login_response",
  "data": {
    "success": true,
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
  }
}
```

## Login Response (Failure)

```json
{
  "version": "1.0",
  "type": "auth_login_response",
  "data": {
    "success": false,
    "message": "Invalid username or password"
  }
}
```

## Logout Request

```json
{
  "version": "1.0",
  "type": "auth_logout",
  "data": {}
}
```

## Logout Response

```json
{
  "version": "1.0",
  "type": "auth_logout_response",
  "data": {
    "success": true
  }
}
```
