# ChatGPT Integration Component

This document describes the ChatGPT integration component of the Cozmo Web Interface.

## Overview

The ChatGPT integration component is responsible for:
- Processing user messages
- Sending requests to the OpenAI API
- Managing conversation context
- Formatting and returning AI responses

## Components

### OpenAIService

`OpenAIService` is the core service that handles communication with the OpenAI API:

```go
// OpenAIService represents the OpenAI service
type OpenAIService struct {
	client *openai.Client
	config config.OpenAIConfig
}
```

#### Methods

- `NewOpenAIService(cfg config.OpenAIConfig) *OpenAIService`: Creates a new OpenAI service
- `GenerateResponse(prompt string, conversation []models.ChatMessage) (string, error)`: Generates a response from OpenAI

### ChatHandler

`ChatHandler` handles HTTP requests for chat functionality:

```go
// ChatHandler handles chat-related requests
type ChatHandler struct {
	service *services.OpenAIService
}
```

#### Methods

- `NewChatHandler(service *services.OpenAIService) *ChatHandler`: Creates a new chat handler
- `Chat(c *gin.Context)`: Handles chat requests

## ChatGPT Integration Flow

1. **Chat Request Flow**:
   ```
   User -> ChatHandler.Chat -> OpenAIService.GenerateResponse -> 
   OpenAI API -> Response -> User
   ```

2. **Conversation Management Flow**:
   ```
   Previous Messages + New Message -> Format as OpenAI Messages -> 
   API Request -> Response -> Add to Conversation History
   ```

## Models

### ChatMessage

```go
// ChatMessage represents a chat message
type ChatMessage struct {
	Content   string `json:"content"`
	Timestamp int64  `json:"timestamp"`
	Username  string `json:"username"`
	IsBot     bool   `json:"isBot"`
}
```

### ChatRequest

```go
// ChatRequest represents a chat request
type ChatRequest struct {
	Message      string       `json:"message" binding:"required"`
	Conversation []ChatMessage `json:"conversation"`
}
```

## OpenAI API Integration

The OpenAI integration uses the `github.com/sashabaranov/go-openai` package:

1. **Client Initialization**:
   ```go
   client := openai.NewClient(cfg.APIKey)
   ```

2. **Message Formatting**:
   ```go
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
   ```

3. **API Request**:
   ```go
   resp, err := s.client.CreateChatCompletion(
       context.Background(),
       openai.ChatCompletionRequest{
           Model:       s.config.Model,
           Messages:    messages,
           MaxTokens:   s.config.MaxTokens,
           Temperature: 0.7,
       },
   )
   ```

## Configuration

The OpenAI service is configured through the following settings:

```go
// OpenAIConfig holds OpenAI API specific configuration
type OpenAIConfig struct {
	APIKey  string
	Model   string
	MaxTokens int
}
```

- `APIKey`: The OpenAI API key
- `Model`: The model to use (default: "gpt-4.1-nano-2025-04-14")
- `MaxTokens`: The maximum number of tokens to generate

## Frontend Integration

The frontend sends chat requests to the server and displays the responses:

```javascript
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
        // Handle error
    }
});
```

## Text-to-Speech Integration

The chat component integrates with the TTS service to convert responses to speech:

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

## Security Considerations

- API Key: The OpenAI API key should be stored securely and not exposed to clients
- Rate Limiting: Implement rate limiting to prevent abuse and manage costs
- Content Filtering: Consider implementing content filtering to prevent inappropriate requests/responses

## Performance Considerations

- Caching: Consider caching common responses to reduce API calls
- Streaming: For longer responses, consider implementing streaming to improve user experience
- Message History Limit: Limit the number of messages in the conversation history to control token usage

## Future Enhancements

- Streaming responses for better user experience
- Multiple AI models support
- System message customization
- Function calling capabilities
- Context length management
- Fine-tuning for Cozmo-specific knowledge
