package handlers

import (
	"cozmo-clouds/internal/config"
	"net/http"

	"github.com/gin-gonic/gin"
)

// ViewHandler handles view-related requests
type ViewHandler struct {
	config *config.Config
}

// NewViewHandler creates a new view handler
func NewViewHandler(cfg *config.Config) *ViewHandler {
	return &ViewHandler{
		config: cfg,
	}
}

// Home handles the home page request
func (h *ViewHandler) Home(c *gin.Context) {
	c.HTML(http.StatusOK, "index.html", gin.H{
		"title": "Cozmo Web Interface",
	})
}

// Login handles the login page request
func (h *ViewHandler) Login(c *gin.Context) {
	c.HTML(http.StatusOK, "login.html", gin.H{
		"title": "Login - Cozmo Web Interface",
	})
}

// Dashboard handles the dashboard page request
func (h *ViewHandler) Dashboard(c *gin.Context) {
	username := c.GetString("username")
	c.HTML(http.StatusOK, "dashboard.html", gin.H{
		"title":      "Dashboard - Cozmo Web Interface",
		"username":   username,
		"pageScript": "dashboard",
	})
}

// Chat handles the chat page request
func (h *ViewHandler) Chat(c *gin.Context) {
	username := c.GetString("username")
	c.HTML(http.StatusOK, "chat.html", gin.H{
		"title":      "Chat - Cozmo Web Interface",
		"username":   username,
		"pageScript": "chat",
	})
}

// Settings handles the settings page request
func (h *ViewHandler) Settings(c *gin.Context) {
	username := c.GetString("username")
	c.HTML(http.StatusOK, "settings.html", gin.H{
		"title":      "Settings - Cozmo Web Interface",
		"username":   username,
		"pageScript": "settings",
	})
}
