# Developer Setup Guide

This document provides step-by-step instructions for setting up a development environment for the Cozmo Web Interface.

## Prerequisites

Before you begin, ensure you have the following installed:

1. **Go 1.22 or later**: [Download and install Go](https://go.dev/doc/install)
2. **Git**: [Download and install Git](https://git-scm.com/downloads)
3. **Text Editor or IDE**: VS Code is recommended with the Go extension
4. **OpenAI API Key**: For ChatGPT integration
5. **Docker** (optional): For containerized development

## Initial Setup

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/cozmo-system.git
cd cozmo-system/server
```

### 2. Install Dependencies

```bash
go mod tidy
```

This will download all required Go dependencies.

### 3. Set Up Environment Variables

Create a `.env` file in the project root:

```bash
# .env
ENVIRONMENT=development
SERVER_HOST=0.0.0.0
SERVER_PORT=8080
JWT_SECRET=your-very-secret-key-change-in-production
TOKEN_EXPIRATION_HOURS=24
OPENAI_API_KEY=your_openai_api_key_here
OPENAI_MODEL=gpt-4.1-nano-2025-04-14
COZMO_WS_ENDPOINT=ws://localhost:81/ws
```

Or set environment variables directly:

```bash
export OPENAI_API_KEY=your_openai_api_key_here
export ENVIRONMENT=development
```

### 4. Create Required Directories

Ensure the following directories exist:

```bash
mkdir -p web/static/tts
mkdir -p web/static/images
```

### 5. Make Run Script Executable

```bash
chmod +x run.sh
```

## Running the Application

### Standard Run

```bash
./run.sh
```

This script compiles and runs the application.

### Hot Reloading for Development

For automatic reloading during development, you can use [Air](https://github.com/cosmtrek/air):

1. Install Air:
   ```bash
   go install github.com/cosmtrek/air@latest
   ```

2. Create `.air.toml` in the project root:
   ```toml
   root = "."
   tmp_dir = "tmp"
   
   [build]
   cmd = "go build -o ./tmp/main ./cmd/main.go"
   bin = "./tmp/main"
   include_ext = ["go", "html", "css", "js"]
   exclude_dir = ["web/static", "tmp"]
   ```

3. Run Air:
   ```bash
   air
   ```

## Development Workflow

### 1. Code Structure

The application follows a modular structure:

- `cmd/main.go`: Application entry point
- `internal/`: Private application code
  - `app/`: Application setup
  - `config/`: Configuration handling
  - `handlers/`: HTTP request handlers
  - `middleware/`: HTTP middleware
  - `models/`: Data models
  - `services/`: Business logic
- `web/`: Web-related assets
  - `assets/`: Frontend assets (CSS, JS, images)
  - `static/`: Static files
  - `templates/`: HTML templates

### 2. Making Changes

When working on the codebase:

1. **Create a Feature Branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Follow Go Best Practices**:
   - Format code with `gofmt`
   - Add comments to exported functions
   - Handle errors properly

3. **Testing**:
   - Write unit tests for new functionality
   - Run tests with `go test ./...`

### 3. Frontend Development

The frontend uses a combination of:
- HTML templates with Go's templating engine
- CSS with GitHub Dark theme
- JavaScript with jQuery

To work on the frontend:
1. Edit templates in `web/templates/`
2. Update CSS in `web/assets/css/`
3. Modify JavaScript in `web/assets/js/`
4. Access the application at http://localhost:8080

### 4. API Development

When adding new API endpoints:

1. Define models in `internal/models/models.go`
2. Create or modify services in `internal/services/`
3. Implement handlers in `internal/handlers/`
4. Register routes in `internal/app/app.go`

### 5. WebSocket Development

For WebSocket-related changes:

1. Update message types in `internal/models/models.go`
2. Modify WebSocket service in `internal/services/cozmo_ws_service.go`
3. Update handler in `internal/handlers/websocket_handler.go`
4. Update client-side code in `web/assets/js/main.js`

## Testing

### Unit Testing

Write unit tests for services and handlers:

```go
// TestAuthService tests the authentication service
func TestAuthService(t *testing.T) {
    // Test logic here
}
```

Run tests with:

```bash
go test ./...
```

### Manual Testing

1. Access http://localhost:8080
2. Log in with default credentials (admin/admin123)
3. Test different features:
   - Chat functionality
   - WebSocket communication
   - TTS functionality

## Troubleshooting Common Issues

### 1. Missing OpenAI API Key

**Symptom**: Chat functionality doesn't work, server logs show API key errors.

**Solution**: Ensure `OPENAI_API_KEY` environment variable is set correctly.

### 2. WebSocket Connection Issues

**Symptom**: Cannot connect to Cozmo robot via WebSocket.

**Solution**:
- Ensure Cozmo robot system is running and accessible
- Check `COZMO_WS_ENDPOINT` environment variable is correctly set
- Check for firewall or network issues

### 3. Static File Issues

**Symptom**: Missing static files, 404 errors.

**Solution**: Ensure the required directories exist and have correct permissions:
```bash
mkdir -p web/static/tts
mkdir -p web/static/images
```

### 4. JWT Authentication Issues

**Symptom**: Authentication fails, cannot access protected routes.

**Solution**:
- Check `JWT_SECRET` is set correctly
- Ensure token is being correctly stored in localStorage
- Check if token is expired (default 24 hours)

## Docker Development Environment (Optional)

For a containerized development environment:

1. Create a `Dockerfile.dev`:
   ```dockerfile
   FROM golang:1.22
   
   WORKDIR /app
   
   COPY go.mod go.sum ./
   RUN go mod download
   
   COPY . .
   
   RUN go install github.com/cosmtrek/air@latest
   
   EXPOSE 8080
   
   CMD ["air"]
   ```

2. Create a `docker-compose.yml`:
   ```yaml
   version: '3'
   
   services:
     app:
       build:
         context: .
         dockerfile: Dockerfile.dev
       ports:
         - "8080:8080"
       volumes:
         - .:/app
       environment:
         - ENVIRONMENT=development
         - SERVER_HOST=0.0.0.0
         - SERVER_PORT=8080
         - JWT_SECRET=your-dev-secret
         - OPENAI_API_KEY=${OPENAI_API_KEY}
         - COZMO_WS_ENDPOINT=ws://host.docker.internal:81/ws
   ```

3. Run with Docker Compose:
   ```bash
   docker-compose up
   ```

## Additional Resources

- [Go Documentation](https://go.dev/doc/)
- [Gin Framework](https://gin-gonic.com/docs/)
- [Gorilla WebSocket](https://github.com/gorilla/websocket)
- [OpenAI API Documentation](https://platform.openai.com/docs/api-reference)
- [JWT Documentation](https://jwt.io/introduction)
- [htgo-tts Documentation](https://github.com/hegedustibor/htgo-tts)
