package services

import (
	"cozmo-clouds/internal/config"
	"log"
	"sync"

	"github.com/gorilla/websocket"
)

// CozmoWebSocketService handles WebSocket communication with browsers and Cozmo robots
type CozmoWebSocketService struct {
	// Browser connections
	browsers map[*websocket.Conn]bool

	// Robot connections mapped by their IDs
	robots map[string]*websocket.Conn

	// Channels for managing connections
	browserRegister   chan *websocket.Conn
	browserUnregister chan *websocket.Conn

	// Channel for broadcasting messages
	broadcast chan struct {
		robotID string
		message []byte
	}

	// Channel for chat messages
	chatMessages chan struct {
		message string
		conn    *websocket.Conn
	}

	// Channel for TTS messages
	ttsMessages chan struct {
		text string
		conn *websocket.Conn
	}

	mutex  sync.RWMutex
	config config.WebSocketConfig
}

// NewCozmoWebSocketService creates a new WebSocket service for Cozmo
func NewCozmoWebSocketService(cfg config.WebSocketConfig) *CozmoWebSocketService {
	return &CozmoWebSocketService{
		browsers:          make(map[*websocket.Conn]bool),
		robots:            make(map[string]*websocket.Conn),
		browserRegister:   make(chan *websocket.Conn),
		browserUnregister: make(chan *websocket.Conn),
		broadcast: make(chan struct {
			robotID string
			message []byte
		}),
		chatMessages: make(chan struct {
			message string
			conn    *websocket.Conn
		}),
		ttsMessages: make(chan struct {
			text string
			conn *websocket.Conn
		}),
		config: cfg,
	}
}

// Run starts the WebSocket service
func (s *CozmoWebSocketService) Run(openaiService *OpenAIService, ttsService *TTSService) {
	for {
		select {
		case client := <-s.browserRegister:
			s.mutex.Lock()
			s.browsers[client] = true
			s.mutex.Unlock()
			log.Printf("Browser client registered")

		case client := <-s.browserUnregister:
			s.mutex.Lock()
			if _, ok := s.browsers[client]; ok {
				delete(s.browsers, client)
				client.Close()
				log.Printf("Browser client unregistered")
			}
			s.mutex.Unlock()

		case msg := <-s.broadcast:
			s.mutex.RLock()
			// If robotID is empty, broadcast to all browsers
			if msg.robotID == "" {
				for browser := range s.browsers {
					err := browser.WriteMessage(websocket.TextMessage, msg.message)
					if err != nil {
						log.Printf("Error writing to browser: %v", err)
						browser.Close()
						delete(s.browsers, browser)
					}
				}
			} else {
				// Add robot ID to message and broadcast to browsers
				for browser := range s.browsers {
					// In a real implementation, you'd want to format the message
					// with the robot ID in a structured way (e.g., JSON)
					err := browser.WriteMessage(websocket.TextMessage, msg.message)
					if err != nil {
						log.Printf("Error writing to browser: %v", err)
						browser.Close()
						delete(s.browsers, browser)
					}
				}
			}
			s.mutex.RUnlock()

		case chatMsg := <-s.chatMessages:
			// Handle chat message from browser - process with GPT and send response
			log.Printf("Received chat message: %s", chatMsg.message)

			// Use OpenAI service to get a response if available
			var response string
			if openaiService != nil {
				var err error
				response, err = openaiService.GetChatResponse(chatMsg.message)
				if err != nil {
					log.Printf("Error getting chat response from OpenAI: %v", err)
					response = "Sorry, I encountered an error processing your request."
				}
			} else {
				response = "This is a response from the WebSocket service. OpenAI integration is not available."
			}

			// Send response back to the specific browser
			err := chatMsg.conn.WriteJSON(map[string]interface{}{
				"type": "chat_response",
				"data": response,
			})
			if err != nil {
				log.Printf("Error sending chat response: %v", err)
			}

		case ttsMsg := <-s.ttsMessages:
			// Handle TTS request from browser
			log.Printf("Received TTS request: %s", ttsMsg.text)

			// Use TTS service to generate audio if available
			var audioPath string
			if ttsService != nil {
				var err error
				audioPath, err = ttsService.TextToSpeech(ttsMsg.text)
				if err != nil {
					log.Printf("Error generating TTS: %v", err)
					audioPath = ""
				}
			} else {
				audioPath = "/static/tts/sample.mp3" // Fallback
			}

			// Send TTS response back to the specific browser
			err := ttsMsg.conn.WriteJSON(map[string]interface{}{
				"type": "tts_response",
				"data": map[string]string{
					"audioPath": audioPath,
				},
			})
			if err != nil {
				log.Printf("Error sending TTS response: %v", err)
			}
		}
	}
}

// RegisterBrowserClient registers a browser WebSocket connection
func (s *CozmoWebSocketService) RegisterBrowserClient(client *websocket.Conn) {
	s.browserRegister <- client
}

// UnregisterBrowserClient unregisters a browser WebSocket connection
func (s *CozmoWebSocketService) UnregisterBrowserClient(client *websocket.Conn) {
	s.browserUnregister <- client
}

// RegisterRobotClient registers a robot WebSocket connection
func (s *CozmoWebSocketService) RegisterRobotClient(robotID string, conn *websocket.Conn) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	// Close existing connection if any
	if existing, ok := s.robots[robotID]; ok {
		existing.Close()
	}

	s.robots[robotID] = conn
	log.Printf("Robot %s connected", robotID)
}

// UnregisterRobotClient unregisters a robot WebSocket connection
func (s *CozmoWebSocketService) UnregisterRobotClient(robotID string) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	if conn, ok := s.robots[robotID]; ok {
		conn.Close()
		delete(s.robots, robotID)
		log.Printf("Robot %s disconnected", robotID)
	}
}

// BroadcastToRobots broadcasts a message to all connected robots
func (s *CozmoWebSocketService) BroadcastToRobots(message []byte) {
	s.mutex.RLock()
	defer s.mutex.RUnlock()

	for robotID, conn := range s.robots {
		err := conn.WriteMessage(websocket.TextMessage, message)
		if err != nil {
			log.Printf("Error writing to robot %s: %v", robotID, err)
			conn.Close()
			delete(s.robots, robotID)
		}
	}
}

// BroadcastToBrowsers broadcasts a message to all connected browsers
func (s *CozmoWebSocketService) BroadcastToBrowsers(robotID string, message []byte) {
	s.broadcast <- struct {
		robotID string
		message []byte
	}{robotID, message}
}

// GetConnectedRobots returns a list of connected robot IDs
func (s *CozmoWebSocketService) GetConnectedRobots() []string {
	s.mutex.RLock()
	defer s.mutex.RUnlock()

	robots := make([]string, 0, len(s.robots))
	for robotID := range s.robots {
		robots = append(robots, robotID)
	}
	return robots
}

// SendToRobot sends a message to a specific robot
func (s *CozmoWebSocketService) SendToRobot(robotID string, message []byte) error {
	s.mutex.RLock()
	defer s.mutex.RUnlock()

	if conn, ok := s.robots[robotID]; ok {
		return conn.WriteMessage(websocket.TextMessage, message)
	}
	return nil
}

// SendChatMessage sends a chat message to be processed
func (s *CozmoWebSocketService) SendChatMessage(message string, conn *websocket.Conn) {
	s.chatMessages <- struct {
		message string
		conn    *websocket.Conn
	}{
		message: message,
		conn:    conn,
	}
}

// SendTTSRequest sends a text-to-speech request
func (s *CozmoWebSocketService) SendTTSRequest(text string, conn *websocket.Conn) {
	s.ttsMessages <- struct {
		text string
		conn *websocket.Conn
	}{
		text: text,
		conn: conn,
	}
}
