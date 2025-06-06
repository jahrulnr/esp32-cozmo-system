package config

import (
	"os"
	"strconv"
)

// Config holds all application configuration
type Config struct {
	Server    ServerConfig
	Auth      AuthConfig
	OpenAI    OpenAIConfig
	WebSocket WebSocketConfig
	TTS       TTSConfig
}

// ServerConfig holds server-specific configuration
type ServerConfig struct {
	Port           string
	Host           string
	Environment    string
	TemplatesPath  string
	StaticFilePath string
	AssetsFilePath string
}

// AuthConfig holds authentication-specific configuration
type AuthConfig struct {
	JWTSecret            string
	TokenExpirationHours int
}

// OpenAIConfig holds OpenAI API specific configuration
type OpenAIConfig struct {
	APIKey    string
	Model     string
	MaxTokens int
}

// WebSocketConfig holds WebSocket specific configuration
type WebSocketConfig struct {
	BrowserEndpoint string // Endpoint for browser connections
	RobotEndpoint   string // Endpoint for robot connections
}

// TTSConfig holds Text-to-Speech specific configuration
type TTSConfig struct {
	OutputDir string
	Language  string
}

// Load loads configuration from environment variables
func Load() (*Config, error) {
	return &Config{
		Server: ServerConfig{
			Port:           getEnv("SERVER_PORT", "8080"),
			Host:           getEnv("SERVER_HOST", "0.0.0.0"),
			Environment:    getEnv("ENVIRONMENT", "development"),
			TemplatesPath:  getEnv("TEMPLATES_PATH", "./web/templates"),
			StaticFilePath: getEnv("STATIC_FILES_PATH", "./web/static"),
			AssetsFilePath: getEnv("ASSETS_FILES_PATH", "./web/assets"),
		},
		Auth: AuthConfig{
			JWTSecret:            getEnv("JWT_SECRET", "secret"),
			TokenExpirationHours: getEnvAsInt("TOKEN_EXPIRATION_HOURS", 24),
		},
		OpenAI: OpenAIConfig{
			APIKey:    getEnv("OPENAI_API_KEY", "openapi_key"),
			Model:     getEnv("OPENAI_MODEL", "gpt-4.1-nano-2025-04-14"),
			MaxTokens: getEnvAsInt("OPENAI_MAX_TOKENS", 4096),
		},
		WebSocket: WebSocketConfig{
			BrowserEndpoint: getEnv("BROWSER_WS_ENDPOINT", "ws://localhost:81/ws/browser"),
			RobotEndpoint:   getEnv("ROBOT_WS_ENDPOINT", "ws://localhost:81/ws/robot"),
		},
		TTS: TTSConfig{
			OutputDir: getEnv("TTS_OUTPUT_DIR", "./web/static/tts"),
			Language:  getEnv("TTS_LANGUAGE", "id"),
		},
	}, nil
}

// getEnv gets an environment variable or returns a default value
func getEnv(key, defaultValue string) string {
	if value, exists := os.LookupEnv(key); exists {
		return value
	}
	return defaultValue
}

// getEnvAsInt gets an environment variable as an integer or returns a default value
func getEnvAsInt(key string, defaultValue int) int {
	if value, exists := os.LookupEnv(key); exists {
		if intVal, err := strconv.Atoi(value); err == nil {
			return intVal
		}
	}
	return defaultValue
}
