package services

import (
	"cozmo-clouds/internal/config"
	"cozmo-clouds/internal/models"
	"errors"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

// AuthService represents the authentication service
type AuthService struct {
	config config.AuthConfig
	// In a real app, this would be stored in a database
	users map[string]models.User
}

// NewAuthService creates a new authentication service
func NewAuthService(cfg config.AuthConfig) *AuthService {
	// In-memory users for demo purposes
	users := make(map[string]models.User)
	users["admin"] = models.User{
		ID:       "1",
		Username: "admin",
		Password: "admin123", // In a real app, this would be hashed
	}

	return &AuthService{
		config: cfg,
		users:  users,
	}
}

// Authenticate validates user credentials
func (s *AuthService) Authenticate(username, password string) (*models.User, error) {
	user, exists := s.users[username]
	if !exists || user.Password != password {
		return nil, errors.New("invalid credentials")
	}
	return &user, nil
}

// GenerateToken generates a JWT token for a user
func (s *AuthService) GenerateToken(user *models.User) (string, error) {
	// Set expiration time
	expirationTime := time.Now().Add(time.Duration(s.config.TokenExpirationHours) * time.Hour)

	// Create claims
	claims := jwt.MapClaims{
		"sub":      user.ID,
		"username": user.Username,
		"exp":      expirationTime.Unix(),
	}

	// Create token
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)

	// Sign token with secret key
	signedToken, err := token.SignedString([]byte(s.config.JWTSecret))
	if err != nil {
		return "", err
	}

	return signedToken, nil
}

// RegisterUser registers a new user
func (s *AuthService) RegisterUser(username, password string) (*models.User, error) {
	// Check if user already exists
	if _, exists := s.users[username]; exists {
		return nil, errors.New("username already taken")
	}

	// Create new user
	user := models.User{
		ID:       time.Now().Format("20060102150405"),
		Username: username,
		Password: password, // In a real app, this would be hashed
	}

	// Save user
	s.users[username] = user

	return &user, nil
}
