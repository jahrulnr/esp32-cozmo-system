# API Documentation

This document details the API endpoints provided by the Cozmo Web Interface.

## Base URL

All endpoints are relative to the base URL of the server (e.g., `http://localhost:8080`).

## Authentication

Most endpoints require authentication via JWT token. The token should be included in the `Authorization` header as a Bearer token:

```
Authorization: Bearer <token>
```

Alternatively, the token can be provided as a cookie named `auth_token`.

## Endpoints

### Authentication

#### Login

Authenticates a user and returns a JWT token.

- **URL**: `/login`
- **Method**: `POST`
- **Auth Required**: No
- **Request Body**:
  ```json
  {
    "username": "string",
    "password": "string"
  }
  ```
- **Success Response**:
  - **Code**: 200 OK
  - **Content**:
    ```json
    {
      "token": "string",
      "user": {
        "id": "string",
        "username": "string"
      }
    }
    ```
- **Error Response**:
  - **Code**: 401 Unauthorized
  - **Content**:
    ```json
    {
      "error": "Invalid credentials"
    }
    ```

#### Register

Registers a new user and returns a JWT token.

- **URL**: `/register`
- **Method**: `POST`
- **Auth Required**: No
- **Request Body**:
  ```json
  {
    "username": "string",
    "password": "string"
  }
  ```
- **Success Response**:
  - **Code**: 200 OK
  - **Content**:
    ```json
    {
      "token": "string",
      "user": {
        "id": "string",
        "username": "string"
      }
    }
    ```
- **Error Response**:
  - **Code**: 400 Bad Request
  - **Content**:
    ```json
    {
      "error": "Username already taken"
    }
    ```

### Chat

#### Send Message

Sends a message to the ChatGPT model and returns the response.

- **URL**: `/api/chat`
- **Method**: `POST`
- **Auth Required**: Yes
- **Request Body**:
  ```json
  {
    "message": "string",
    "conversation": [
      {
        "content": "string",
        "timestamp": 1622547600,
        "username": "string",
        "isBot": true|false
      }
    ]
  }
  ```
- **Success Response**:
  - **Code**: 200 OK
  - **Content**:
    ```json
    {
      "message": {
        "content": "string",
        "timestamp": 1622547600,
        "username": "Cozmo",
        "isBot": true
      }
    }
    ```
- **Error Response**:
  - **Code**: 500 Internal Server Error
  - **Content**:
    ```json
    {
      "error": "Error message"
    }
    ```

### Text-to-Speech

#### Convert Text to Speech

Converts text to speech and returns the URL of the audio file.

- **URL**: `/api/tts`
- **Method**: `POST`
- **Auth Required**: Yes
- **Request Body**:
  ```json
  {
    "text": "string"
  }
  ```
- **Success Response**:
  - **Code**: 200 OK
  - **Content**:
    ```json
    {
      "audioPath": "string"
    }
    ```
- **Error Response**:
  - **Code**: 500 Internal Server Error
  - **Content**:
    ```json
    {
      "error": "Error message"
    }
    ```

### Speech-to-Text

#### Convert Speech to Text

Converts speech audio to text.

- **URL**: `/api/stt`
- **Method**: `POST`
- **Auth Required**: Yes
- **Request Body**:
  ```json
  {
    "audioData": "base64-encoded-audio"
  }
  ```
- **Success Response**:
  - **Code**: 200 OK
  - **Content**:
    ```json
    {
      "text": "string"
    }
    ```
- **Error Response**:
  - **Code**: 400 Bad Request
  - **Content**:
    ```json
    {
      "error": "Invalid audio data"
    }
    ```

### WebSocket

#### WebSocket Connection

Establishes a WebSocket connection for real-time communication.

- **URL**: `/ws`
- **Protocol**: `WebSocket`
- **Auth Required**: Yes (via query parameter or cookie)

#### WebSocket Message Types

##### Client to Server

1. **Authentication**
   ```json
   {
     "type": "auth",
     "content": "jwt-token"
   }
   ```

2. **Control Commands**
   ```json
   {
     "type": "control",
     "content": "command-string"
   }
   ```
   Available commands:
   - `move_forward_start`
   - `move_forward_stop`
   - `move_backward_start`
   - `move_backward_stop`
   - `turn_left_start`
   - `turn_left_stop`
   - `turn_right_start`
   - `turn_right_stop`
   - `stop`
   - `lift_up`
   - `lift_down`
   - `head_up`
   - `head_down`

3. **Actions**
   ```json
   {
     "type": "action",
     "content": "action-string"
   }
   ```
   Available actions:
   - `say_hello`
   - `dance`
   - `pickup_cube`
   - `play_animation`

4. **Camera Controls**
   ```json
   {
     "type": "camera",
     "content": "camera-command"
   }
   ```
   Available commands:
   - `start`
   - `stop`
   - `capture`

##### Server to Client

1. **Camera Feed**
   ```json
   {
     "type": "camera",
     "content": "base64-encoded-image-or-url"
   }
   ```

2. **Sensor Data**
   ```json
   {
     "type": "sensors",
     "content": "{\"battery\":85,\"temperature\":45,\"gyro\":{\"x\":0,\"y\":0,\"z\":0},\"cliffDetected\":false}"
   }
   ```

## Views

The following HTML views are available:

- **Home**: `/`
- **Login**: `/login`
- **Dashboard**: `/dashboard`
- **Chat**: `/chat`
- **Settings**: `/settings`

## Static Files

Static files are served from the following directories:

- **CSS**: `/assets/css/`
- **JavaScript**: `/assets/js/`
- **Images**: `/assets/images/`
- **TTS Audio**: `/static/tts/`
