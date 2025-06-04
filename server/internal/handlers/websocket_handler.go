package handlers

import (
	"cozmo-clouds/internal/services"
	"encoding/json"
	"log"
	"net/http"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

// WebSocketHandler handles WebSocket connections
type WebSocketHandler struct {
	service  *services.CozmoWebSocketService
	upgrader websocket.Upgrader
}

// NewWebSocketHandler creates a new WebSocket handler
func NewWebSocketHandler(service *services.CozmoWebSocketService) *WebSocketHandler {
	return &WebSocketHandler{
		service: service,
		upgrader: websocket.Upgrader{
			ReadBufferSize:  1024,
			WriteBufferSize: 1024,
			CheckOrigin: func(r *http.Request) bool {
				return true // Allow all origins for now
			},
		},
	}
}

// HandleBrowser handles WebSocket connections from web browsers
func (h *WebSocketHandler) HandleBrowser(c *gin.Context) {
	// Upgrade HTTP connection to WebSocket
	conn, err := h.upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Printf("Failed to upgrade browser connection to WebSocket: %v", err)
		return
	}

	// Register browser client
	h.service.RegisterBrowserClient(conn)

	// Handle client disconnection
	defer func() {
		h.service.UnregisterBrowserClient(conn)
	}()

	// Read messages from browser
	for {
		messageType, message, err := conn.ReadMessage()
		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				log.Printf("Browser WebSocket error: %v", err)
			}
			break
		}

		// Only process text messages
		if messageType == websocket.TextMessage {
			// Parse the message as JSON
			var msg map[string]interface{}
			if err := json.Unmarshal(message, &msg); err != nil {
				log.Printf("Error parsing WebSocket message: %v", err)
				continue
			}

			// Check the message type
			msgType, ok := msg["type"].(string)
			if !ok {
				log.Printf("WebSocket message missing 'type' field")
				continue
			}

			// Handle different message types
			switch msgType {
			case "chat":
				// Handle chat message
				if chatMsg, ok := msg["data"].(string); ok {
					h.service.SendChatMessage(chatMsg, conn)
				}
			case "tts":
				// Handle TTS request
				if ttsText, ok := msg["data"].(string); ok {
					h.service.SendTTSRequest(ttsText, conn)
				}
			case "robot_command":
				// Forward robot commands to all connected robots
				h.service.BroadcastToRobots(message)
			default:
				log.Printf("Unknown WebSocket message type: %s", msgType)
			}
		}
	}
}

// HandleRobot handles WebSocket connections from Cozmo robots
func (h *WebSocketHandler) HandleRobot(c *gin.Context) {
	robotID := c.Query("id")
	if robotID == "" {
		c.String(http.StatusBadRequest, "Robot ID is required")
		return
	}

	// Upgrade HTTP connection to WebSocket
	conn, err := h.upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Printf("Failed to upgrade robot connection to WebSocket: %v", err)
		return
	}

	// Register robot client
	h.service.RegisterRobotClient(robotID, conn)

	// Handle client disconnection
	defer func() {
		h.service.UnregisterRobotClient(robotID)
	}()

	// Read messages from robot
	for {
		messageType, message, err := conn.ReadMessage()
		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				log.Printf("Robot WebSocket error: %v", err)
			}
			break
		}

		// Only forward text messages
		if messageType == websocket.TextMessage {
			// Forward message to all connected browsers
			h.service.BroadcastToBrowsers(robotID, message)
		}
	}
}
