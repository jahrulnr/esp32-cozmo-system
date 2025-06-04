package services

import (
	"context"
	"cozmo-clouds/internal/config"
	"cozmo-clouds/internal/models"
	"errors"

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

// GenerateResponse generates a response from OpenAI
func (s *OpenAIService) GenerateResponse(prompt string, conversation []models.ChatMessage) (string, error) {
	// Check if API key is set
	if s.config.APIKey == "" {
		return "", errors.New("OpenAI API key not set")
	}

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

	if len(resp.Choices) == 0 {
		return "", errors.New("no response from OpenAI")
	}

	return resp.Choices[0].Message.Content, nil
}

// GetChatResponse generates a chat response using the OpenAI API
func (s *OpenAIService) GetChatResponse(message string) (string, error) {
	// For WebSocket-based chat, we'll use an empty conversation history for now
	// In a real implementation, you'd want to store and track conversation history
	return s.GenerateResponse(message, []models.ChatMessage{})
}
