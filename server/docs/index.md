# Cozmo Web Interface Documentation

Welcome to the Cozmo Web Interface documentation. This index provides links to all available documentation for the project.

## Getting Started

- [README](/README.md) - Project overview and basic setup
- [Development Setup Guide](/docs/development/setup.md) - Detailed setup instructions for developers

## Architecture

- [Architecture Summary](/docs/architecture/summary.md) - High-level overview of the system architecture
- [Architecture Overview](/docs/architecture/overview.md) - Detailed architecture documentation
- [System Diagrams](/docs/architecture/diagrams.md) - Visual representation of the system architecture
- [Component Relationships](/docs/architecture/relationships.md) - How components interact with each other
- [Design Philosophy](/docs/architecture/design_philosophy.md) - Core design principles and implementation decisions

## API Documentation

- [API Reference](/docs/api/api.md) - Complete API documentation including endpoints, request/response formats

## Components

- [Authentication](/docs/components/authentication.md) - Authentication system documentation
- [WebSocket](/docs/components/websocket.md) - WebSocket communication documentation
- [ChatGPT Integration](/docs/components/chatgpt.md) - ChatGPT integration documentation
- [AI Integration Guide](/docs/components/ai_integration.md) - Detailed AI integration documentation
- [Text-to-Speech](/docs/components/tts.md) - TTS system documentation
- [Frontend Architecture](/docs/components/frontend.md) - Frontend architecture documentation
- [Cozmo Integration](/docs/components/cozmo_integration.md) - Cozmo robot integration documentation

## Development Guides

- [Development Guide](/docs/development/guide.md) - General development guidelines
- [Code Organization](/docs/development/code_organization.md) - How the code is organized
- [Setup Guide](/docs/development/setup.md) - Development environment setup

## Directory Structure

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
├── pkg/                 # Public libraries
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

## Key Technologies

- **Go**: Backend language (version 1.22+)
- **Gin**: Web framework
- **WebSockets**: Real-time communication (Gorilla WebSocket)
- **JWT**: Authentication (JSON Web Tokens)
- **OpenAI API**: ChatGPT integration
- **htgo-tts**: Text-to-speech functionality
- **jQuery**: Frontend JavaScript library

## Contributing

For information on how to contribute to the project, see the [Development Guide](/docs/development/guide.md).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

For issues, questions, or feedback, please open an issue in the GitHub repository.
