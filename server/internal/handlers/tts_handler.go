package handlers

import (
	"cozmo-clouds/internal/models"
	"cozmo-clouds/internal/services"
	"net/http"

	"github.com/gin-gonic/gin"
)

// TTSHandler handles Text-to-Speech requests
type TTSHandler struct {
	service *services.TTSService
}

// NewTTSHandler creates a new TTS handler
func NewTTSHandler(service *services.TTSService) *TTSHandler {
	return &TTSHandler{
		service: service,
	}
}

// Convert converts text to speech
func (h *TTSHandler) Convert(c *gin.Context) {
	var req models.TTSRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": err.Error(),
		})
		return
	}

	// Convert text to speech
	audioPath, err := h.service.TextToSpeech(req.Text)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"error": err.Error(),
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"audioPath": audioPath,
	})
}
