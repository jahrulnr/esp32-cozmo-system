# Architecture Summary

This document provides a high-level summary of the Cozmo Web Interface architecture.

## System Overview

The Cozmo Web Interface is a Go web application that serves as a bridge between users and Cozmo robots. It provides a user interface for controlling and interacting with robots, as well as integrating with AI services like ChatGPT.

```
┌───────────────┐      ┌───────────────┐      ┌───────────────┐
│   Web Client  │◄────►│   Go Server   │◄────►│  Cozmo Robot  │
└───────────────┘      └───────┬───────┘      └───────────────┘
                               │
                               ▼
                       ┌───────────────┐
                       │   OpenAI API  │
                       └───────────────┘
```

## Key Components

1. **Web UI**: GitHub Dark themed user interface
2. **Authentication System**: JWT-based authentication
3. **WebSocket System**: Real-time communication
4. **ChatGPT Integration**: AI-powered conversations
5. **TTS/STT Services**: Voice capabilities
6. **Cozmo Control**: Robot command and control

## Component Dependencies

```
┌───────────────┐
│    Web UI     │
└───────┬───────┘
        │
        ▼
┌───────────────┐     ┌───────────────┐
│ Authentication│◄────┤  WebSockets   │
└───────┬───────┘     └───────┬───────┘
        │                     │
        ▼                     ▼
┌───────────────┐     ┌───────────────┐
│  ChatGPT API  │     │  Cozmo API    │
└───────┬───────┘     └───────────────┘
        │
        ▼
┌───────────────┐
│   TTS/STT     │
└───────────────┘
```

## Data Flow

1. **Authentication Flow**:
   ```
   User → Login Credentials → JWT Generation → Protected Routes
   ```

2. **WebSocket Communication Flow**:
   ```
   Browser → WebSocket Connection → Server → Cozmo Robot
   Cozmo Robot → Server → WebSocket → Browser
   ```

3. **ChatGPT Interaction Flow**:
   ```
   User Input → Server → OpenAI API → Response → TTS (Optional) → User
   ```

4. **Robot Control Flow**:
   ```
   UI Controls → WebSocket → Server → Command Processing → Cozmo Robot
   ```

## Technologies

- **Backend**: Go with Gin framework
- **Frontend**: HTML/CSS/JavaScript with jQuery
- **Real-time Communication**: WebSockets (gorilla/websocket)
- **Authentication**: JWT (golang-jwt/jwt)
- **AI**: OpenAI GPT-4.1 nano model
- **TTS**: htgo-tts
- **Database**: In-memory storage (could be extended to persistent storage)

## Extensibility

The architecture is designed to be modular and extensible:

1. **New Robot Commands**: Add new command types and handlers
2. **Additional AI Models**: Integrate with other AI services
3. **Persistent Storage**: Add database support for user data and conversations
4. **Multiple Robot Support**: Extend WebSocket service to manage multiple robot connections

## Security Architecture

1. **Authentication**: JWT token validation for all protected routes
2. **CORS Protection**: Configuration for allowed origins
3. **API Key Security**: Secure storage of OpenAI API keys
4. **Input Validation**: Request validation at the handler layer

## Performance Considerations

1. **Connection Pooling**: WebSocket connection management
2. **Response Caching**: For frequent AI responses
3. **Static Asset Serving**: Efficient delivery of frontend assets
4. **Resource Limitation**: Token limits for AI, connection limits for WebSockets

## Deployment Architecture

For production deployment, the following architecture is recommended:

```
                        ┌───────────────┐
                        │   NGINX/LB    │
                        └───────┬───────┘
                                │
┌───────────────┐      ┌───────▼───────┐      ┌───────────────┐
│   Web Client  │◄────►│   Go Server   │◄────►│  Cozmo Robot  │
└───────────────┘      └───────┬───────┘      └───────────────┘
                               │
                       ┌───────▼───────┐
                       │   OpenAI API  │
                       └───────────────┘
```

For more detailed information about the architecture, see the [Architecture Overview](overview.md).
