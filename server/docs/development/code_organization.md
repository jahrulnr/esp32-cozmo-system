# Code Organization

This document explains the organization and structure of the Cozmo Web Interface codebase.

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
│   │   ├── css/         # CSS styles
│   │   ├── js/          # JavaScript files
│   │   └── images/      # Image assets
│   ├── static/          # Static files (generated content)
│   │   ├── tts/         # TTS audio output
│   │   └── images/      # User-uploaded images
│   └── templates/       # HTML templates
├── docs/                # Project documentation
├── go.mod               # Go module definition
├── go.sum               # Go module checksums
└── run.sh               # Script to build and run the application
```

## Code Module Organization

### 1. Entry Point

`cmd/main.go` is the application entry point:

```go
package main

import (
	"log"
	"server/internal/app"
	"server/internal/config"
)

func main() {
	// Load configuration
	cfg, err := config.LoadConfig()
	if err != nil {
		log.Fatalf("Failed to load configuration: %v", err)
	}

	// Initialize and run the application
	app := app.NewApp(cfg)
	app.Run()
}
```

### 2. App Module

`internal/app/app.go` handles application setup:

```go
package app

import (
	"server/internal/config"
	"server/internal/handlers"
	"server/internal/middleware"
	"server/internal/services"

	"github.com/gin-gonic/gin"
)

// App represents the application
type App struct {
	router *gin.Engine
	config config.Config
}

// NewApp creates a new application
func NewApp(cfg config.Config) *App {
	router := gin.Default()
	return &App{
		router: router,
		config: cfg,
	}
}

// Run starts the application
func (a *App) Run() {
	// Initialize services
	// Configure routes
	// Start server
}
```

### 3. Configuration Module

`internal/config/config.go` handles configuration loading:

```go
package config

import (
	"os"
	"strconv"
	"time"
)

// Config represents the application configuration
type Config struct {
	Server        ServerConfig
	Auth          AuthConfig
	OpenAI        OpenAIConfig
	WebSocket     WebSocketConfig
}

// LoadConfig loads the configuration from environment variables
func LoadConfig() (Config, error) {
	// Load configuration from environment or defaults
}
```

### 4. Handlers

Handlers process HTTP requests:

- `internal/handlers/auth_handler.go`: Authentication handlers
- `internal/handlers/chat_handler.go`: Chat API handlers
- `internal/handlers/view_handler.go`: Page view handlers
- `internal/handlers/tts_handler.go`: TTS handlers
- `internal/handlers/stt_handler.go`: STT handlers
- `internal/handlers/websocket_handler.go`: WebSocket handlers

Example handler structure:

```go
package handlers

import (
	"net/http"
	"server/internal/models"
	"server/internal/services"

	"github.com/gin-gonic/gin"
)

// ChatHandler handles chat-related requests
type ChatHandler struct {
	service *services.OpenAIService
}

// NewChatHandler creates a new chat handler
func NewChatHandler(service *services.OpenAIService) *ChatHandler {
	return &ChatHandler{
		service: service,
	}
}

// Chat handles chat requests
func (h *ChatHandler) Chat(c *gin.Context) {
	// Process request
	// Call service
	// Return response
}
```

### 5. Services

Services contain business logic:

- `internal/services/auth_service.go`: Authentication service
- `internal/services/openai_service.go`: OpenAI integration
- `internal/services/tts_service.go`: TTS service
- `internal/services/stt_service.go`: STT service
- `internal/services/cozmo_ws_service.go`: Cozmo WebSocket service

Example service structure:

```go
package services

import (
	"context"
	"server/internal/config"
	"server/internal/models"

	"github.com/sashabaranov/go-openai"
)

// OpenAIService represents the OpenAI service
type OpenAIService struct {
	client *openai.Client
	config config.OpenAIConfig
}

// NewOpenAIService creates a new OpenAI service
func NewOpenAIService(cfg config.OpenAIConfig) *OpenAIService {
	client := openai.NewClient(cfg.APIKey)
	return &OpenAIService{
		client: client,
		config: cfg,
	}
}

// GenerateResponse generates a response from the OpenAI API
func (s *OpenAIService) GenerateResponse(prompt string, conversation []models.ChatMessage) (string, error) {
	// Business logic implementation
}
```

### 6. Models

Models define data structures:

`internal/models/models.go`:

```go
package models

// User represents a user
type User struct {
	ID       string `json:"id"`
	Username string `json:"username"`
	Password string `json:"-"` // Not returned in JSON responses
}

// AuthRequest represents an authentication request
type AuthRequest struct {
	Username string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required"`
}

// ChatMessage represents a chat message
type ChatMessage struct {
	Content   string `json:"content"`
	Timestamp int64  `json:"timestamp"`
	Username  string `json:"username"`
	IsBot     bool   `json:"isBot"`
}
```

### 7. Middleware

Middleware provides cross-cutting functionality:

`internal/middleware/middleware.go`:

```go
package middleware

import (
	"net/http"
	"server/internal/config"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
)

// JWTMiddleware validates JWT tokens
func JWTMiddleware(cfg config.AuthConfig) gin.HandlerFunc {
	return func(c *gin.Context) {
		// Validate JWT token
	}
}

// CORSMiddleware handles CORS
func CORSMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		// Set CORS headers
	}
}
```

### 8. Web Templates

HTML templates use Go's templating engine:

`web/templates/layout.html`:
```html
<!DOCTYPE html>
<html>
<head>
    <title>{{ .title }}</title>
    <link rel="stylesheet" href="/assets/css/github-dark.css">
    <link rel="stylesheet" href="/assets/css/main.css">
</head>
<body>
    <header>
        <!-- Header content -->
    </header>
    
    <main>
        {{ block "content" . }}{{ end }}
    </main>
    
    <footer>
        <!-- Footer content -->
    </footer>
    
    <script src="/assets/js/jquery.min.js"></script>
    <script src="/assets/js/main.js"></script>
</body>
</html>
```

`web/templates/chat.html`:
```html
{{ define "content" }}
<div class="chat-container">
    <!-- Chat interface -->
</div>

<script>
    $(document).ready(function() {
        // Chat-specific JavaScript
    });
</script>
{{ end }}
```

### 9. Frontend Assets

Frontend assets are organized by type:

- `web/assets/css/`: CSS styles
  - `github-dark.css`: GitHub Dark theme
  - `main.css`: Application-specific styles
- `web/assets/js/`: JavaScript files
  - `main.js`: Main JavaScript logic
- `web/assets/images/`: Image assets

### 10. Static Files

Static files include:

- `web/static/tts/`: TTS-generated audio files
- `web/static/images/`: User-uploaded images

## Dependency Management

Dependencies are managed through Go modules:

- `go.mod`: Module definition and dependencies
- `go.sum`: Checksum file for verification

Example `go.mod`:
```go
module server

go 1.22

require (
	github.com/gin-gonic/gin v1.9.0
	github.com/golang-jwt/jwt/v5 v5.0.0
	github.com/gorilla/websocket v1.5.0
	github.com/hegedustibor/htgo-tts v0.0.0-20220821045517-04f3cda7a12f
	github.com/sashabaranov/go-openai v1.9.0
)

require (
	// Transitive dependencies
)
```

## Code Patterns

### Dependency Injection

The application uses dependency injection to provide services to handlers:

```go
// Create services
authService := services.NewAuthService(cfg.Auth)
openAIService := services.NewOpenAIService(cfg.OpenAI)
ttsService := services.NewTTSService("web/static/tts")

// Create handlers
authHandler := handlers.NewAuthHandler(authService)
chatHandler := handlers.NewChatHandler(openAIService)
ttsHandler := handlers.NewTTSHandler(ttsService)
```

### Handler Pattern

Handlers follow a consistent pattern:

1. Parse and validate request
2. Call appropriate service
3. Format and return response

```go
func (h *ChatHandler) Chat(c *gin.Context) {
	// 1. Parse and validate request
	var req models.ChatRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	
	// 2. Call appropriate service
	response, err := h.service.GenerateResponse(req.Message, req.Conversation)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	
	// 3. Format and return response
	c.JSON(http.StatusOK, gin.H{
		"message": models.ChatMessage{
			Content:   response,
			Timestamp: time.Now().Unix(),
			Username:  "Cozmo",
			IsBot:     true,
		},
	})
}
```

### Service Pattern

Services contain the business logic and external integrations:

```go
func (s *OpenAIService) GenerateResponse(prompt string, conversation []models.ChatMessage) (string, error) {
	// Format messages for OpenAI
	messages := []openai.ChatCompletionMessage{}
	
	// Add conversation history
	for _, msg := range conversation {
		role := openai.ChatMessageRoleUser
		if msg.IsBot {
			role = openai.ChatMessageRoleAssistant
		}
		
		messages = append(messages, openai.ChatCompletionMessage{
			Role:    role,
			Content: msg.Content,
		})
	}
	
	// Add new prompt
	messages = append(messages, openai.ChatCompletionMessage{
		Role:    openai.ChatMessageRoleUser,
		Content: prompt,
	})
	
	// Call OpenAI API
	resp, err := s.client.CreateChatCompletion(
		context.Background(),
		openai.ChatCompletionRequest{
			Model:       s.config.Model,
			Messages:    messages,
			MaxTokens:   s.config.MaxTokens,
			Temperature: 0.7,
		},
	)
	
	if err != nil {
		return "", err
	}
	
	return resp.Choices[0].Message.Content, nil
}
```

This organizational structure ensures the code is maintainable, testable, and follows separation of concerns principles.
