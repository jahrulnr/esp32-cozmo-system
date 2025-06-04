package handlers

import (
	"cozmo-clouds/internal/models"
	"cozmo-clouds/internal/services"
	"net/http"
	"time"

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
	var req models.ChatRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": err.Error(),
		})
		return
	}

	username := c.GetString("username")
	if username == "" {
		username = "User"
	}

	// Generate response from OpenAI
	response, err := h.service.GenerateResponse(req.Message, req.Conversation)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"error": err.Error(),
		})
		return
	}

	// Create response message
	message := models.ChatMessage{
		Content:   response,
		Timestamp: time.Now().Unix(),
		Username:  "Cozmo",
		IsBot:     true,
	}

	c.JSON(http.StatusOK, gin.H{
		"message": message,
	})
}
