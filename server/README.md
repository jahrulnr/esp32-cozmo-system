# Cozmo Web Interface

A modular Go web application for controlling and interacting with Cozmo robots. This project provides a feature-rich web interface with GitHub Dark theme, real-time communication via WebSockets, text-to-speech, speech-to-text, and ChatGPT integration.

## Features

- **Modern UI with GitHub Dark Theme**: Clean, responsive interface designed for optimal user experience
- **Real-time Robot Control**: Direct control of Cozmo robot movements and actions
- **WebSocket Integration**: Two-way communication with Cozmo robot systems
- **User Authentication**: Secure login and registration with JWT
- **ChatGPT Integration**: Conversational AI using OpenAI's GPT-4.1 nano model
- **Voice Capabilities**: Text-to-speech and speech-to-text functionality
- **Dashboard**: Real-time monitoring of robot status, sensors, and camera feed
- **Responsive Design**: Works on desktop and mobile devices

## Technology Stack

- **Backend**: Go 1.22+
- **Web Framework**: Gin
- **Frontend**: jQuery with HTML5/CSS3
- **Real-time Communication**: WebSockets (Gorilla WebSocket)
- **Authentication**: JWT (JSON Web Tokens)
- **AI Integration**: OpenAI API (GPT-4.1 nano model)
- **Text-to-Speech**: htgo-tts library

## Project Structure

```
server/
├── api/                 # API documentation
├── cmd/                 # Application entry points
│   └── main.go          # Main application entry point
├── configs/             # Configuration files
├── internal/            # Private application code
│   ├── app/             # Application setup and initialization
│   ├── config/          # Configuration handling
│   ├── handlers/        # HTTP request handlers
│   ├── middleware/      # HTTP middleware
│   ├── models/          # Data models
│   └── services/        # Business logic services
├── pkg/                 # Public libraries that can be used by external applications
├── web/                 # Web-related assets
│   ├── assets/          # Frontend assets (CSS, JS, images)
│   ├── static/          # Static files
│   └── templates/       # HTML templates
├── docs/                # Project documentation
│   ├── architecture/    # Architecture documentation
│   ├── api/             # API documentation
│   ├── components/      # Component-specific documentation
│   └── development/     # Development guides
├── go.mod               # Go module definition
├── go.sum               # Go module checksums
└── run.sh               # Script to build and run the application
```

## Getting Started

### Prerequisites

- Go 1.22 or later
- OpenAI API Key (for ChatGPT integration)

### Environment Variables

The application uses the following environment variables:

| Variable | Description | Default |
|----------|-------------|---------|
| SERVER_PORT | Port on which the server runs | 8080 |
| SERVER_HOST | Host on which the server runs | 0.0.0.0 |
| ENVIRONMENT | Development/production environment | development |
| JWT_SECRET | Secret key for JWT token generation | your-very-secret-key-change-in-production |
| TOKEN_EXPIRATION_HOURS | JWT token expiration in hours | 24 |
| OPENAI_API_KEY | OpenAI API key for ChatGPT | - |
| OPENAI_MODEL | OpenAI model to use | gpt-4.1-nano-2025-04-14 |
| COZMO_WS_ENDPOINT | WebSocket endpoint for Cozmo robot | ws://localhost:81/ws |

### Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/cozmo-system.git
cd cozmo-system/server
```

2. Install dependencies:
```bash
go mod tidy
```

3. Set up environment variables:
```bash
export OPENAI_API_KEY=your_api_key_here
```

4. Run the application:
```bash
./run.sh
```

5. Access the web interface at http://localhost:8080

### Default Login

- Username: admin
## Core Components

### Authentication System

The authentication system uses JWT tokens for secure user sessions. It includes:
- Login/registration endpoints
- Token-based authentication
- Middleware for protecting routes

For more details, see the [Authentication Documentation](docs/components/authentication.md).

### WebSocket Communication

The application establishes WebSocket connections for real-time communication with:
- Web clients (browsers)
- Cozmo robot system

For more details, see the [WebSocket Documentation](docs/components/websocket.md).

### Chat Interface

The chat interface integrates with OpenAI's GPT models to provide:
- Natural language interaction
- Context-aware conversations
- Text-to-speech output

For more details, see the [ChatGPT Integration Documentation](docs/components/chatgpt.md).

### TTS and STT Services

- Text-to-Speech: Converts text to audio using the htgo-tts library
- Speech-to-Text: Provides an interface for future STT implementation

For more details, see the [TTS Documentation](docs/components/tts.md).

## API Documentation

See the [API Documentation](docs/api/api.md) for detailed information about available endpoints including:

- Authentication endpoints (`/login`, `/register`)
- Chat endpoints (`/api/chat`)
- TTS/STT endpoints (`/api/tts`, `/api/stt`)
- WebSocket endpoint (`/ws`)

## Architecture

The application follows a layered architecture pattern with clear separation of concerns:
- Handlers Layer: HTTP request handling
- Services Layer: Business logic
- Models Layer: Data structures
- Middleware Layer: Cross-cutting concerns
- Configuration Layer: Application settings

For a comprehensive overview, see the [Architecture Documentation](docs/architecture/overview.md).

## Frontend Architecture

The frontend is built with jQuery and uses a modular approach:

- **Layout**: Common layout with navigation
- **Pages**: Specific page templates (dashboard, chat, settings)
- **CSS**: GitHub Dark theme with custom styling
- **JavaScript**: Modular JS for different functionalities

For more information, see the [Frontend Documentation](docs/components/frontend.md).

## Integration with Cozmo System

This server communicates with the Cozmo robot system through WebSockets, allowing:
- Sending commands to the robot
- Receiving status updates and sensor data
- Streaming camera feed
- Controlling robot movements and actions

For details on how the web interface integrates with the Cozmo robot system, see the [Cozmo Integration Documentation](docs/components/cozmo_integration.md).

## Development Guide

For information on:
- Setting up a development environment
- Adding new features
- Testing
- Deployment

See the [Development Guide](docs/development/guide.md).

## Extending the Application

### Adding New Robot Functions

1. Add a new command type in the WebSocket message model
2. Implement the handler in the WebSocket service
3. Update the frontend to send the new command

### Adding New Views

1. Create a new HTML template in `web/templates/`
2. Add a route handler in `internal/handlers/view_handler.go`
3. Register the route in `internal/app/app.go`

## License

[MIT License](LICENSE)

## Contributors

- Your Name - Initial work

## Acknowledgments

- OpenAI for the GPT model API
- [htgo-tts](https://github.com/hegedustibor/htgo-tts) for text-to-speech capabilities
- Gin framework contributors
