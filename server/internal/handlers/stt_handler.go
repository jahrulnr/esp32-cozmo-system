package handlers

import (
	"cozmo-clouds/internal/models"
	"cozmo-clouds/internal/services"
	"encoding/base64"
	"net/http"

	"github.com/gin-gonic/gin"
)

// STTHandler handles Speech-to-Text requests
type STTHandler struct {
	service *services.STTService
}

// NewSTTHandler creates a new STT handler
func NewSTTHandler(service *services.STTService) *STTHandler {
	return &STTHandler{
		service: service,
	}
}

// Convert converts speech to text
func (h *STTHandler) Convert(c *gin.Context) {
	var req models.STTRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": err.Error(),
		})
		return
	}

	// Decode base64 audio data
	audioData, err := base64.StdEncoding.DecodeString(req.AudioData)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": "Invalid audio data",
		})
		return
	}

	// Convert speech to text
	text, err := h.service.SpeechToText(audioData)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"error": err.Error(),
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"text": text,
	})
}
