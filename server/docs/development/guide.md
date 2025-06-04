# Development Guide

This document provides guidelines for developers working on the Cozmo Web Interface project.

## Development Environment Setup

### Prerequisites

- Go 1.22 or later
- Git
- Code editor (VS Code recommended)
- Web browser
- OpenAI API key (for ChatGPT integration)

### Local Setup

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
   export ENVIRONMENT=development
   ```

4. Run the application:
   ```bash
   ./run.sh
   ```

5. Access the web interface at http://localhost:8080

### Development Server

For hot reloading during development, you can use [Air](https://github.com/cosmtrek/air):

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
   exclude_dir = ["web/static"]
   ```

3. Run Air:
   ```bash
   air
   ```

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
├── pkg/                 # Public libraries
├── web/                 # Web-related assets
│   ├── assets/          # Frontend assets (CSS, JS, images)
│   ├── static/          # Static files
│   └── templates/       # HTML templates
├── go.mod               # Go module definition
└── go.sum               # Go module checksums
```

## Coding Standards

### Go

1. **Formatting**: Follow Go's standard formatting with `gofmt`
2. **Naming**:
   - Use camelCase for variable and function names
   - Use PascalCase for exported functions and types
   - Use all-caps for constants
3. **Comments**:
   - Add comments to all exported functions and types
   - Use `// Comment` style for single-line comments
   - Use `/* Comment */` style for multi-line comments
4. **Error Handling**:
   - Always check and handle errors
   - Return errors with context
5. **Imports**:
   - Group imports by standard library, external packages, and internal packages
   - Sort imports alphabetically within groups

### HTML/CSS/JavaScript

1. **HTML**:
   - Use semantic HTML tags
   - Keep indentation consistent
   - Use double quotes for attributes
2. **CSS**:
   - Follow the component-based approach
   - Use meaningful class names
   - Use CSS variables for theme colors
3. **JavaScript**:
   - Use camelCase for variable and function names
   - Add comments for complex logic
   - Follow jQuery best practices

## Adding New Features

### Adding a New API Endpoint

1. **Define the Model** in `internal/models/models.go`:
   ```go
   // NewFeatureRequest represents a new feature request
   type NewFeatureRequest struct {
       Param1 string `json:"param1" binding:"required"`
       Param2 int    `json:"param2" binding:"required"`
   }
   ```

2. **Create a Service** in `internal/services/`:
   ```go
   // NewFeatureService represents the new feature service
   type NewFeatureService struct {
       // Dependencies
   }
   
   // NewNewFeatureService creates a new feature service
   func NewNewFeatureService() *NewFeatureService {
       return &NewFeatureService{}
   }
   
   // DoSomething performs the feature's action
   func (s *NewFeatureService) DoSomething(param1 string, param2 int) (string, error) {
       // Implementation
       return "result", nil
   }
   ```

3. **Create a Handler** in `internal/handlers/`:
   ```go
   // NewFeatureHandler handles new feature requests
   type NewFeatureHandler struct {
       service *services.NewFeatureService
   }
   
   // NewNewFeatureHandler creates a new feature handler
   func NewNewFeatureHandler(service *services.NewFeatureService) *NewFeatureHandler {
       return &NewFeatureHandler{
           service: service,
       }
   }
   
   // HandleRequest handles the new feature request
   func (h *NewFeatureHandler) HandleRequest(c *gin.Context) {
       var req models.NewFeatureRequest
       if err := c.ShouldBindJSON(&req); err != nil {
           c.JSON(http.StatusBadRequest, gin.H{
               "error": err.Error(),
           })
           return
       }
       
       result, err := h.service.DoSomething(req.Param1, req.Param2)
       if err != nil {
           c.JSON(http.StatusInternalServerError, gin.H{
               "error": err.Error(),
           })
           return
       }
       
       c.JSON(http.StatusOK, gin.H{
           "result": result,
       })
   }
   ```

4. **Register the Route** in `internal/app/app.go`:
   ```go
   // Initialize services
   newFeatureService := services.NewNewFeatureService()
   
   // Initialize handlers
   newFeatureHandler := handlers.NewNewFeatureHandler(newFeatureService)
   
   // Register routes
   api.POST("/new-feature", newFeatureHandler.HandleRequest)
   ```

### Adding a New View

1. **Create a Template** in `web/templates/`:
   ```html
   {{ define "content" }}
   <div class="new-feature">
       <h1>New Feature</h1>
       <!-- Feature content -->
   </div>
   
   <script>
   $(document).ready(function() {
       // Feature JavaScript
   });
   </script>
   {{ end }}
   ```

2. **Add a View Handler** in `internal/handlers/view_handler.go`:
   ```go
   // NewFeature handles the new feature page request
   func (h *ViewHandler) NewFeature(c *gin.Context) {
       username := c.GetString("username")
       c.HTML(http.StatusOK, "new-feature.html", gin.H{
           "title":    "New Feature - Cozmo Web Interface",
           "username": username,
       })
   }
   ```

3. **Register the Route** in `internal/app/app.go`:
   ```go
   // Register view routes
   protected.GET("/new-feature", viewHandler.NewFeature)
   ```

### Adding WebSocket Functionality

1. **Define Message Types** in `internal/models/models.go`:
   ```go
   // WebSocketNewFeatureMessage represents a new feature message
   type WebSocketNewFeatureMessage struct {
       Action string `json:"action"`
       Value  string `json:"value"`
   }
   ```

2. **Update WebSocket Handler** to process the new message type:
   ```go
   // Handle processes WebSocket messages
   func (h *WebSocketHandler) Handle(c *gin.Context) {
       // Existing code...
       
       // Process message
       switch msg.Type {
       case "new-feature":
           // Process new feature message
           var newFeatureMsg models.WebSocketNewFeatureMessage
           if err := json.Unmarshal([]byte(msg.Content), &newFeatureMsg); err != nil {
               log.Printf("Error unmarshaling new feature message: %v", err)
               continue
           }
           
           // Process the message
           // ...
           
           // Broadcast response to clients
           h.service.Broadcast([]byte("New feature processed"))
       }
   }
   ```

3. **Add Frontend Code** to send and receive the new message type:
   ```javascript
   // Send new feature message
   socket.send(JSON.stringify({
       type: "new-feature",
       content: JSON.stringify({
           action: "do-something",
           value: "example"
       })
   }));
   
   // Handle new feature response
   socket.onmessage = function(event) {
       const message = JSON.parse(event.data);
       
       // Handle different message types
       switch (message.type) {
           case "new-feature-response":
               console.log("New feature response:", message.content);
               break;
       }
   };
   ```

## Testing

### Unit Testing

Write unit tests for services and handlers:

```go
// TestAuthService_Authenticate tests the Authenticate method
func TestAuthService_Authenticate(t *testing.T) {
    // Setup
    cfg := config.AuthConfig{
        JWTSecret:            "test-secret",
        TokenExpirationHours: 24,
    }
    service := NewAuthService(cfg)
    
    // Test cases
    testCases := []struct {
        name          string
        username      string
        password      string
        expectSuccess bool
    }{
        {"Valid credentials", "admin", "admin123", true},
        {"Invalid username", "invalid", "admin123", false},
        {"Invalid password", "admin", "invalid", false},
    }
    
    for _, tc := range testCases {
        t.Run(tc.name, func(t *testing.T) {
            // Execute
            user, err := service.Authenticate(tc.username, tc.password)
            
            // Assert
            if tc.expectSuccess {
                assert.NoError(t, err)
                assert.NotNil(t, user)
                assert.Equal(t, tc.username, user.Username)
            } else {
                assert.Error(t, err)
                assert.Nil(t, user)
            }
        })
    }
}
```

Run tests with:
```bash
go test ./...
```

### Integration Testing

For integration tests, use a test database and mock external services:

```go
// TestChatHandler_Chat tests the Chat handler
func TestChatHandler_Chat(t *testing.T) {
    // Setup
    mockService := &mocks.MockOpenAIService{
        GenerateResponseFunc: func(prompt string, conversation []models.ChatMessage) (string, error) {
            return "Test response", nil
        },
    }
    handler := NewChatHandler(mockService)
    
    // Create test server
    router := gin.Default()
    router.POST("/api/chat", handler.Chat)
    
    // Create test request
    req := models.ChatRequest{
        Message: "Test message",
        Conversation: []models.ChatMessage{},
    }
    body, _ := json.Marshal(req)
    
    // Execute request
    w := httptest.NewRecorder()
    r := httptest.NewRequest("POST", "/api/chat", bytes.NewBuffer(body))
    r.Header.Set("Content-Type", "application/json")
    router.ServeHTTP(w, r)
    
    // Assert
    assert.Equal(t, http.StatusOK, w.Code)
    
    var response map[string]models.ChatMessage
    err := json.Unmarshal(w.Body.Bytes(), &response)
    assert.NoError(t, err)
    
    message, ok := response["message"]
    assert.True(t, ok)
    assert.Equal(t, "Test response", message.Content)
}
```

## Dependency Management

Use Go modules for dependency management:

1. **Adding a Dependency**:
   ```bash
   go get github.com/example/package@v1.2.3
   ```

2. **Updating Dependencies**:
   ```bash
   go get -u
   ```

3. **Cleaning Up Dependencies**:
   ```bash
   go mod tidy
   ```

## Documentation

Always update documentation when adding or modifying features:

1. **Code Comments**: Add comments to explain complex logic and document exported functions
2. **README Updates**: Update the README.md with new features or changes
3. **API Documentation**: Update API documentation in the docs directory

## Version Control

Follow these guidelines for version control:

1. **Branching Strategy**:
   - `main`: Production-ready code
   - `develop`: Development code
   - `feature/*`: Feature branches
   - `bugfix/*`: Bug fix branches

2. **Commit Messages**:
   - Use descriptive commit messages
   - Start with a verb (e.g., "Add", "Fix", "Update")
   - Keep the first line under 50 characters
   - Add details in the body if needed

3. **Pull Requests**:
   - Create PRs for all changes
   - Add a description of the changes
   - Reference related issues
   - Ensure all tests pass
   - Request code reviews
