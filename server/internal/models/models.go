package models

// User represents a user in the system
type User struct {
	ID       string `json:"id"`
	Username string `json:"username"`
	Password string `json:"-"` // Password is not serialized to JSON
}

// LoginRequest represents a login request
type LoginRequest struct {
	Username string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required"`
}

// RegisterRequest represents a registration request
type RegisterRequest struct {
	Username string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required"`
}

// ChatMessage represents a chat message
type ChatMessage struct {
	Content   string `json:"content"`
	Timestamp int64  `json:"timestamp"`
	Username  string `json:"username"`
	IsBot     bool   `json:"isBot"`
}

// ChatRequest represents a chat request
type ChatRequest struct {
	Message      string        `json:"message" binding:"required"`
	Conversation []ChatMessage `json:"conversation"`
}

// TTSRequest represents a Text-to-Speech request
type TTSRequest struct {
	Text string `json:"text" binding:"required"`
}

// STTRequest represents a Speech-to-Text request
type STTRequest struct {
	AudioData string `json:"audioData" binding:"required"` // Base64 encoded audio data
}

// WebSocketMessage represents a message sent over WebSocket
type WebSocketMessage struct {
	Type    string `json:"type"`
	Content string `json:"content"`
	Sender  string `json:"sender"`
}
