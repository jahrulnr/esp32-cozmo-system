# Component Relationships

This document describes the relationships between different components of the Cozmo Web Interface system.

## Core Components

The Cozmo Web Interface consists of the following core components:

1. **Web Server (Gin Framework)**
2. **Authentication System**
3. **WebSocket Communication System**
4. **ChatGPT Integration**
5. **Text-to-Speech (TTS) System**
6. **Speech-to-Text (STT) System**
7. **Frontend UI**
8. **Cozmo Robot Integration**

## Component Relationships

### Web Server and Authentication

The web server relies on the authentication system to:
- Validate user credentials during login
- Generate JWT tokens for authenticated users
- Verify JWT tokens for protected routes
- Apply middleware to routes requiring authentication

```
Web Server --uses--> Authentication System
Authentication System --provides--> JWT Tokens
Web Server --applies--> Authentication Middleware
```

### Web Server and WebSocket Communication

The web server initializes and manages the WebSocket service, which:
- Handles client WebSocket connections
- Maintains a connection to the Cozmo robot
- Routes messages between clients and the robot

```
Web Server --initializes--> WebSocket Service
WebSocket Service --handles--> Client Connections
WebSocket Service --maintains--> Cozmo Connection
```

### Web Server and ChatGPT Integration

The web server routes chat requests to the ChatGPT integration service, which:
- Formats messages for the OpenAI API
- Sends requests to the OpenAI API
- Processes responses from the OpenAI API
- Returns formatted responses to the client

```
Web Server --routes--> Chat Handler
Chat Handler --uses--> OpenAI Service
OpenAI Service --calls--> OpenAI API
```

### ChatGPT and TTS Integration

The ChatGPT integration can work with the TTS system to:
- Convert text responses to speech
- Provide audio feedback to the user
- Enable voice-based interaction

```
ChatGPT Integration --can use--> TTS System
TTS System --generates--> Audio Files
Web Server --serves--> Audio Files
```

### WebSocket and Cozmo Integration

The WebSocket service is the primary communication channel for connecting browsers and Cozmo robots:
- Routes commands from browsers to specific robots
- Broadcasts robot updates to connected browsers
- Manages robot connection state and authentication

```
WebSocket Service --routes browser commands to--> Specific Cozmo Robot
Cozmo Robot --initiates connection to--> WebSocket Service
Cozmo Robot --sends updates to--> WebSocket Service --broadcasts to--> Browser Clients
```

### Frontend UI and Server Components

The frontend UI interacts with various server components:
- Authentication system for login/logout
- WebSocket service for real-time updates
- Chat API for conversational interaction
- TTS API for voice output

```
Frontend UI --authenticates via--> Authentication System
Frontend UI --connects to--> WebSocket Service
Frontend UI --sends messages to--> Chat API
Frontend UI --requests voice from--> TTS API
```

## Data Flow Between Components

### User Authentication Flow

```
User --credentials--> Web Server --validates--> Authentication Service
Authentication Service --generates--> JWT Token
Web Server --returns--> JWT Token --stored in--> Frontend
Frontend --includes token in--> Subsequent Requests
```

### Chat Message Flow

```
User --sends message--> Frontend UI --HTTP request--> Web Server
Web Server --routes to--> Chat Handler --calls--> OpenAI Service
OpenAI Service --formats and sends--> OpenAI API Request
OpenAI API --returns--> Response --processed by--> OpenAI Service
Web Server --returns--> Formatted Response --displayed in--> Frontend UI
```

### Robot Control Flow

```
User --selects robot--> Frontend UI --identifies target robot-->
User --control action--> Frontend UI --WebSocket message with robotId--> WebSocket Handler
WebSocket Handler --processes--> Control Command --forwards to--> Cozmo WS Service
Cozmo WS Service --identifies target robot and sends command to--> Specific Cozmo Robot
Cozmo Robot --executes--> Command --sends status update with robotId--> Cozmo WS Service
WebSocket Handler --identifies source robot and broadcasts--> Status Update --updates--> Frontend UI
```

## Component Dependency Diagram

```
┌────────────┐      ┌────────────┐      ┌────────────┐
│  Frontend  │─────►│ Web Server │─────►│ Middleware │
└────────────┘      └────────────┘      └────────────┘
                          │
           ┌─────────────┬┴──────────────┐
           ▼             ▼               ▼
   ┌───────────┐  ┌────────────┐  ┌────────────┐
   │   Auth    │  │  WebSocket │  │    Chat    │
   │  Service  │  │  Service   │  │  Service   │
   └───────────┘  └────────────┘  └────────────┘
         │               │              │
         │               │              ▼
         │               │        ┌────────────┐
         │               │        │  OpenAI    │
         │               │        │  Service   │
         │               │        └────────────┘
         │               ▼
         │         ┌────────────┐     ┌────────────┐
         └───────►│    TTS      │◄────┤    STT     │
                  │  Service    │     │  Service   │
                  └────────────┘     └────────────┘
```

## Initialization Order

During application startup, components are initialized in the following order:

1. Configuration loading
2. Logger initialization
3. Service initialization
   - Authentication Service
   - TTS Service
   - STT Service
   - OpenAI Service
   - WebSocket Service
4. Handler initialization
   - View Handler
   - Auth Handler
   - Chat Handler
   - TTS Handler
   - STT Handler
   - WebSocket Handler
5. Middleware setup
6. Route registration
7. Web server startup

This ensures that dependencies are available when needed by other components.

## Future Component Extensions

The system is designed to be modular and extensible. Future components could include:

1. **Database Integration**: For persistent user and conversation storage
2. **Multiple Robot Support**: Managing connections to multiple Cozmo robots
3. **Advanced Authentication**: OAuth, MFA, etc.
4. **Analytics System**: Tracking usage and performance metrics
5. **Advanced AI Features**: Custom AI models, fine-tuning, etc.

These components would integrate with the existing architecture through well-defined interfaces.
