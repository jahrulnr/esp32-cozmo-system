# AI Integration Guide

This document provides detailed information about the ChatGPT integration in the Cozmo Web Interface.

## Overview

The Cozmo Web Interface integrates with OpenAI's GPT models to provide conversational AI capabilities. This integration enables:

1. Natural language conversations with users
2. Context-aware responses based on conversation history
3. Integration with text-to-speech for voiced responses
4. Potential control of Cozmo robot through natural language

## Architecture

The AI integration follows this architecture:

```
┌──────────┐      ┌──────────┐      ┌───────────┐      ┌───────────┐
│  User    │─────►│  Chat    │─────►│  OpenAI   │─────►│  OpenAI   │
│ Interface│      │ Handler  │      │  Service  │      │    API    │
└──────────┘      └──────────┘      └───────────┘      └───────────┘
     ▲                                    │                  │
     │                                    ▼                  │
     │                              ┌───────────┐            │
     └──────────────────────────────┤ Response  │◄───────────┘
                                    └───────────┘
```

## Implementation Details

### OpenAI Service

The core of the AI integration is the `OpenAIService`, which handles communication with the OpenAI API:

```go
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

// GenerateResponse generates a response from OpenAI
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

### Chat Handler

The `ChatHandler` processes chat requests from the frontend:

```go
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
	var req models.ChatRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": err.Error(),
		})
		return
	}
	
	response, err := h.service.GenerateResponse(req.Message, req.Conversation)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"error": err.Error(),
		})
		return
	}
	
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

### Frontend Integration

The frontend integrates with the chat API:

```javascript
function sendMessage() {
    const message = $('#message-input').val().trim();
    if (!message) return;
    
    // Clear input
    $('#message-input').val('');
    
    // Add message to UI
    addMessage(message, false);
    
    // Add typing indicator
    const typingId = addTypingIndicator();
    
    // Send to server
    $.ajax({
        url: '/api/chat',
        type: 'POST',
        contentType: 'application/json',
        headers: {
            'Authorization': `Bearer ${localStorage.getItem('auth_token')}`
        },
        data: JSON.stringify({
            message: message,
            conversation: conversation
        }),
        success: function(response) {
            // Remove typing indicator
            removeTypingIndicator(typingId);
            
            // Add bot message to chat
            addMessage(response.message.content, true);
            
            // Convert response to speech if enabled
            if ($('#voice-enabled').is(':checked')) {
                textToSpeech(response.message.content);
            }
            
            // Make Cozmo speak the response if enabled
            if ($('#cozmo-speak').is(':checked')) {
                sendToCozmo('speak', response.message.content);
            }
        },
        error: function(xhr) {
            // Remove typing indicator
            removeTypingIndicator(typingId);
            
            // Show error
            showError('Error: ' + (xhr.responseJSON?.error || 'Failed to send message'));
        }
    });
}
```

## Configuration

The OpenAI integration is configured through environment variables:

```
OPENAI_API_KEY=your_api_key_here
OPENAI_MODEL=gpt-4.1-nano-2025-04-14
OPENAI_MAX_TOKENS=1000
```

These can be set in the `.env` file or directly in the environment.

## Conversation Management

The application manages conversation context by:

1. Storing conversation history in the frontend
2. Sending relevant conversation history with each request
3. Limiting conversation history to control token usage

```javascript
// Global conversation history
let conversation = [];

// Add message to conversation
function addMessage(content, isBot) {
    const message = {
        content: content,
        timestamp: Date.now() / 1000,
        username: isBot ? "Cozmo" : username,
        isBot: isBot
    };
    
    // Add to conversation history
    conversation.push(message);
    
    // Limit conversation length to avoid token limit issues
    if (conversation.length > 20) {
        conversation = conversation.slice(-20);
    }
    
    // Display message in UI
    displayMessage(message);
}
```

## Integration with TTS

The AI integration works with TTS to provide voiced responses:

```javascript
function textToSpeech(text) {
    $.ajax({
        url: '/api/tts',
        type: 'POST',
        contentType: 'application/json',
        headers: {
            'Authorization': `Bearer ${localStorage.getItem('auth_token')}`
        },
        data: JSON.stringify({
            text: text
        }),
        success: function(response) {
            // Create audio element and play
            const audio = new Audio(response.audioPath);
            audio.play();
        },
        error: function(xhr) {
            console.error('TTS Error:', xhr);
        }
    });
}
```

## Integration with Cozmo Robot

The AI can control the Cozmo robot through WebSocket commands:

```javascript
function sendToCozmo(action, content) {
    if (!socket || socket.readyState !== WebSocket.OPEN) {
        console.error('WebSocket not connected');
        return;
    }
    
    socket.send(JSON.stringify({
        type: action,
        content: content
    }));
}
```

## Message Formatting

The chat interface formats messages with markdown-like syntax:

```javascript
// Format message with markdown-like syntax
function formatMessage(content) {
    let formatted = content.replace(/\n/g, '<br>');
    
    // Format code blocks
    formatted = formatted.replace(/```(.*?)```/gs, '<pre><code>$1</code></pre>');
    
    // Format inline code
    formatted = formatted.replace(/`([^`]+)`/g, '<code>$1</code>');
    
    // Format bold text
    formatted = formatted.replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>');
    
    // Format italic text
    formatted = formatted.replace(/\*([^*]+)\*/g, '<em>$1</em>');
    
    return formatted;
}
```

## Advanced Topics

### Adding System Messages

To customize the AI's behavior, you can add system messages:

```go
// Add system message
messages = append(messages, openai.ChatCompletionMessage{
    Role:    openai.ChatMessageRoleSystem,
    Content: "You are Cozmo, a small robot assistant. You are helpful, friendly, and concise.",
})
```

### Streaming Responses

For a better user experience, you can implement streaming responses:

```go
// StreamResponse generates a streaming response from OpenAI
func (s *OpenAIService) StreamResponse(prompt string, conversation []models.ChatMessage, responseChannel chan string, errChannel chan error) {
    // Implementation of streaming response
}
```

### Function Calling

For more advanced control of the Cozmo robot, you can use function calling:

```go
// FunctionCall represents a function call
type FunctionCall struct {
    Name      string          `json:"name"`
    Arguments json.RawMessage `json:"arguments"`
}

// Define available functions
functions := []openai.FunctionDefinition{
    {
        Name:        "move_cozmo",
        Description: "Move the Cozmo robot in a specific direction",
        Parameters: map[string]interface{}{
            "type": "object",
            "properties": map[string]interface{}{
                "direction": map[string]interface{}{
                    "type":        "string",
                    "description": "The direction to move (forward, backward, left, right)",
                    "enum":        []string{"forward", "backward", "left", "right"},
                },
                "duration": map[string]interface{}{
                    "type":        "number",
                    "description": "The duration in seconds",
                },
            },
            "required": []string{"direction"},
        },
    },
}
```

### Multi-Modal Support

For future enhancements, you could add multi-modal support:

```go
// MultiModalRequest represents a multi-modal request
type MultiModalRequest struct {
    Message      string         `json:"message"`
    Images       []string       `json:"images"`
    Conversation []ChatMessage  `json:"conversation"`
}
```

## Security Considerations

- **API Key Protection**: The OpenAI API key must be kept secure and not exposed to clients
- **Rate Limiting**: Implement rate limiting to prevent abuse and control costs
- **Content Filtering**: Consider implementing content filtering to prevent inappropriate content
- **Input Validation**: Validate all user input before sending to the OpenAI API

## Performance Considerations

- **Token Management**: Limit conversation history to reduce token usage
- **Caching**: Consider caching common responses to reduce API calls
- **Timeout Handling**: Implement appropriate timeouts for API calls
- **Error Handling**: Gracefully handle API errors and rate limits

## Troubleshooting

### Common Issues

1. **API Key Issues**

   Problem: API requests fail with authentication errors.
   
   Solution: Ensure the `OPENAI_API_KEY` environment variable is set correctly.

2. **Rate Limit Exceeded**

   Problem: API requests fail with rate limit errors.
   
   Solution: Implement exponential backoff and retry logic.

3. **Token Limit Exceeded**

   Problem: Requests fail due to token limits.
   
   Solution: Reduce the amount of conversation history sent or use a model with higher token limits.

4. **High Latency**

   Problem: Responses are slow.
   
   Solution: Implement streaming responses, optimize conversation history, or use a faster model.

### Debugging Techniques

1. Enable verbose logging for OpenAI API calls:

```go
log.Printf("Sending request to OpenAI: %+v", messages)
```

2. Monitor token usage:

```go
log.Printf("Response tokens: %d prompt tokens, %d completion tokens, %d total tokens",
    resp.Usage.PromptTokens, resp.Usage.CompletionTokens, resp.Usage.TotalTokens)
```

## Future Enhancements

1. **Fine-tuning**: Fine-tune the model for Cozmo-specific knowledge and behaviors
2. **AI Memory**: Implement persistent memory for the AI to remember past interactions
3. **Multi-turn Reasoning**: Implement multi-turn reasoning for complex tasks
4. **Voice-first Interaction**: Enhance the voice interface for more natural interaction
5. **Vision Integration**: Add vision capabilities for image understanding

## Resources

- [OpenAI API Documentation](https://platform.openai.com/docs/api-reference)
- [OpenAI Go Client](https://github.com/sashabaranov/go-openai)
- [GPT Best Practices](https://platform.openai.com/docs/guides/gpt-best-practices)
- [OpenAI Cookbook](https://github.com/openai/openai-cookbook)
