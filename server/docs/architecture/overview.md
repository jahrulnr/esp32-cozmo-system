# System Architecture

This document describes the high-level architecture of the Cozmo Web Interface system.

## Overview

The Cozmo Web Interface is built as a modular Go application using the Gin web framework. The system follows a layered architecture pattern, separating concerns across multiple layers to improve maintainability and testability.

```
┌────────────┐      ┌────────────┐      ┌────────────┐      ┌────────────┐
│   Client   │◄────►│  Handlers  │◄────►│  Services  │◄────►│   Models   │
└────────────┘      └────────────┘      └────────────┘      └────────────┘
                           ▲                   ▲
                           │                   │
                           ▼                   ▼
                    ┌────────────┐      ┌────────────┐
                    │ Middleware │      │   Config   │
                    └────────────┘      └────────────┘
```

## Layers

### 1. Handlers Layer

The handlers layer is responsible for handling HTTP requests and WebSocket connections. It:
- Parses incoming requests
- Validates input data
- Calls appropriate services
- Formats and sends responses

Key files:
- `internal/handlers/auth_handler.go`
- `internal/handlers/chat_handler.go`
- `internal/handlers/tts_handler.go`
- `internal/handlers/stt_handler.go`
- `internal/handlers/view_handler.go`
- `internal/handlers/websocket_handler.go`

### 2. Services Layer

The services layer contains the business logic of the application. It:
- Implements domain-specific operations
- Enforces business rules
- Orchestrates data flow between different components
- Manages external API integrations

Key files:
- `internal/services/auth_service.go`
- `internal/services/openai_service.go`
- `internal/services/tts_service.go`
- `internal/services/stt_service.go`
- `internal/services/cozmo_ws_service.go`

### 3. Models Layer

The models layer defines the data structures used throughout the application. It:
- Defines request/response structures
- Implements data validation rules
- Provides data transfer objects

Key files:
- `internal/models/models.go`

### 4. Middleware Layer

The middleware layer handles cross-cutting concerns. It:
- Authenticates requests
- Logs request information
- Handles CORS
- Manages panic recovery

Key files:
- `internal/middleware/middleware.go`

### 5. Configuration Layer

The configuration layer manages application settings. It:
- Loads configuration from environment variables
- Provides defaults for missing values
- Validates configuration

Key files:
- `internal/config/config.go`

## Key Components

### WebSocket Communication

The WebSocket component handles real-time bidirectional communication through two separate endpoints:

1. Browser Endpoint (/ws/browser): Web clients connect to this endpoint
2. Robot Endpoint (/ws/robot): Cozmo robots connect to this endpoint with their unique IDs

Communication flow:
```
Web Browser <--> Server <-- [ws/robot] --> Cozmo Robot 1
                       <-- [ws/robot] --> Cozmo Robot 2
                       <-- [ws/robot] --> Cozmo Robot N
```

The `CozmoWebSocketService` maintains two types of connections:
1. Browser connections (browsers connected to /ws/browser)
2. Robot connections (robots connected to /ws/robot)

Message Routing:
- Messages from browsers are broadcast to all connected robots or targeted to specific robots
- Messages from robots include their ID and are broadcast to all connected browsers

Security:
- Robot connections require a valid robot ID
- Browser connections are authenticated using JWT tokens
- All WebSocket communication is managed through the Go server for security and control

### Authentication System

The authentication system uses JWT (JSON Web Tokens) to secure the application. The flow is:

1. User provides credentials
2. Server validates credentials
3. Server generates JWT token
4. Token is sent to client
5. Client includes token in subsequent requests
6. Server validates token on protected routes

### Chat System

The chat system integrates with OpenAI's API to provide conversational AI. The flow is:

1. User sends a message
2. Server forwards message to OpenAI API
3. OpenAI API returns a response
4. Server sends response back to user

### Text-to-Speech System

The TTS system converts text to speech audio files. The flow is:

1. Text is sent to the TTS service
2. TTS service generates an audio file
3. Audio file URL is returned to the client
4. Client plays the audio file

## Data Flow

1. **Authentication Flow**:
   ```
   Client -> Auth Handler -> Auth Service -> JWT Generation -> Client
   ```

2. **Chat Flow**:
   ```
   Client -> Chat Handler -> OpenAI Service -> OpenAI API -> Client
   ```

3. **WebSocket Flow**:
   ```
   Client -> WebSocket Handler -> WebSocket Service -> Cozmo Robot
   Cozmo Robot -> WebSocket Service -> Client
   ```

4. **TTS Flow**:
   ```
   Client -> TTS Handler -> TTS Service -> Audio File -> Client
   ```

## Deployment Architecture

In production, the system would be deployed with:

```
┌────────────┐      ┌────────────┐      ┌────────────┐
│   NGINX    │◄────►│  Go Server │◄────►│  Cozmo Bot │
└────────────┘      └────────────┘      └────────────┘
      ▲                    ▲
      │                    │
      ▼                    ▼
┌────────────┐      ┌────────────┐
│   Client   │      │  OpenAI API│
└────────────┘      └────────────┘
```

- NGINX serves as a reverse proxy and handles SSL termination
- The Go server runs as a systemd service or in a container
- Cozmo Bot runs on its own hardware and connects via WebSockets
