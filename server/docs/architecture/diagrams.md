# System Diagrams

This document provides detailed diagrams of the Cozmo Web Interface system architecture.

## Component Diagram

```
┌─────────────────────────────────────────────────────┐
│                   Web Client                         │
│                                                     │
│  ┌──────────┐  ┌──────────┐  ┌──────────────────┐  │
│  │ Dashboard │  │   Chat   │  │ Robot Controls   │  │
│  └──────────┘  └──────────┘  └──────────────────┘  │
└─────────────────────────────────────────────────────┘
             │           │           │
             ▼           ▼           ▼
┌─────────────────────────────────────────────────────┐
│                   Go Server                         │
│                                                     │
│  ┌──────────┐  ┌──────────┐  ┌──────────────────┐  │
│  │  Auth     │  │ WebSocket│  │ HTTP Handlers    │  │
│  └──────────┘  └──────────┘  └──────────────────┘  │
│        │             │               │             │
│  ┌──────────┐  ┌──────────┐  ┌──────────────────┐  │
│  │  JWT     │  │ WS Mgmt  │  │ Services Layer   │  │
│  └──────────┘  └──────────┘  └──────────────────┘  │
└─────────────────────────────────────────────────────┘
      │             │                 │
      │             ▼                 ▼
      │     ┌──────────────┐  ┌──────────────────┐
      │     │ Cozmo Robot  │  │    OpenAI API    │
      │     └──────────────┘  └──────────────────┘
      │                                │
      └────────────────────┐          │
                           ▼          ▼
                      ┌─────────────────┐
                      │    TTS Engine   │
                      └─────────────────┘
```

## Authentication Flow Diagram

```
┌──────────┐      ┌──────────┐      ┌──────────┐      ┌──────────┐
│  Client  │      │  Auth    │      │  Auth    │      │  JWT     │
│  Browser │─────►│ Handler  │─────►│ Service  │─────►│ Creation │
└──────────┘      └──────────┘      └──────────┘      └────┬─────┘
                                                           │
┌──────────┐      ┌──────────┐                             │
│  Client  │◄─────┤ Response │◄────────────────────────────┘
│  Browser │      │  with JWT│
└──────────┘      └──────────┘

┌──────────┐      ┌──────────┐      ┌──────────┐      ┌──────────┐
│  Client  │      │  JWT     │      │  JWT     │      │ Protected│
│  Request │─────►│ Header   │─────►│ Validate │─────►│  Route   │
└──────────┘      └──────────┘      └──────────┘      └──────────┘
```

## WebSocket Communication Flow

```
┌──────────┐      ┌──────────┐      ┌────────────────┐
│  Client  │─────►│WebSocket │─────►│  WS Handler    │
│  Browser │◄─────┤Connection│◄─────┤                │
└──────────┘      └──────────┘      └────────┬───────┘
                                             │
                                             ▼
┌──────────┐      ┌──────────┐      ┌────────────────┐
│  Cozmo   │◄─────┤WebSocket │◄─────┤ CozmoWS Service│
│  Robot   │─────►│Connection│─────►│                │
└──────────┘      └──────────┘      └────────────────┘
```

## ChatGPT Integration Flow

```
┌──────────┐      ┌──────────┐      ┌──────────┐      ┌──────────┐
│  User    │─────►│  Chat    │─────►│  OpenAI  │─────►│ OpenAI   │
│ Message  │      │ Handler  │      │ Service  │      │   API    │
└──────────┘      └──────────┘      └──────────┘      └────┬─────┘
                                                           │
┌──────────┐      ┌──────────┐      ┌──────────┐           │
│  User    │◄─────┤ Response │◄─────┤   Bot    │◄──────────┘
│ Interface│      │          │      │ Response │
└──────────┘      └──────────┘      └──────────┘
```

## Text-to-Speech Flow

```
┌──────────┐      ┌──────────┐      ┌──────────┐      ┌──────────┐
│  Text    │─────►│   TTS    │─────►│   TTS    │─────►│  Audio   │
│  Input   │      │ Handler  │      │ Service  │      │  File    │
└──────────┘      └──────────┘      └──────────┘      └────┬─────┘
                                                           │
┌──────────┐      ┌──────────┐                             │
│  Audio   │◄─────┤  File    │◄────────────────────────────┘
│  Player  │      │   URL    │
└──────────┘      └──────────┘
```

## Database Entity Relationship Diagram (Future)

```
┌──────────┐       ┌──────────┐      ┌──────────┐
│   User   │       │Conversation│     │ Message  │
├──────────┤       ├──────────┤      ├──────────┤
│ id (PK)  │───┐   │ id (PK)  │──┐   │ id (PK)  │
│ username │   └──►│ user_id  │  └──►│ conv_id  │
│ password │       │ created_at│      │ content  │
│ created_at│      │ updated_at│      │ timestamp│
└──────────┘       └──────────┘      │ is_bot   │
                                     └──────────┘
```

## Deployment Architecture

```
┌───────────────┐
│   Client      │
│   Browser     │
└───────┬───────┘
        │
        ▼
┌───────────────┐
│   NGINX       │ ◄─── HTTPS Termination
│   Load        │ ◄─── Static File Serving
│   Balancer    │ ◄─── Rate Limiting
└───────┬───────┘
        │
        ├─────────────┬─────────────┐
        ▼             ▼             ▼
┌───────────┐  ┌───────────┐  ┌───────────┐
│ Go Server │  │ Go Server │  │ Go Server │ ◄─── Horizontal Scaling
│ Instance 1│  │ Instance 2│  │ Instance 3│
└───────────┘  └───────────┘  └───────────┘
        │             │             │
        └─────────────┼─────────────┘
                      │
                      ▼
               ┌───────────────┐
               │  OpenAI API   │
               └───────────────┘
```

## Security Architecture

```
┌───────────────┐
│   Client      │ ─┐
└───────────────┘  │
                   │ HTTPS (TLS)
┌───────────────┐  │
│   NGINX SSL   │ ◄┘
└───────┬───────┘
        │
        │ Internal Network
        ▼
┌───────────────────────────────────────────┐
│                Go Server                   │
│                                           │
│ ┌─────────────┐    ┌─────────────────┐    │
│ │ CORS        │    │ JWT Middleware  │    │
│ │ Middleware  │    │                 │    │
│ └─────────────┘    └─────────────────┘    │
│                                           │
│ ┌─────────────┐    ┌─────────────────┐    │
│ │ Rate Limit  │    │ Input Validation│    │
│ │ Middleware  │    │                 │    │
│ └─────────────┘    └─────────────────┘    │
└───────────────────────────────────────────┘
        │
        │ HTTPS
        ▼
┌───────────────┐
│  External APIs│
└───────────────┘
```

For a more detailed explanation of each component, see the individual component documentation in the [Components Directory](../components/).
