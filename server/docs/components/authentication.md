# Authentication Component

This document describes the authentication component of the Cozmo Web Interface.

## Overview

The authentication component is responsible for:
- User authentication (login)
- User registration
- JWT token generation and validation
- Session management

## Components

### AuthService

`AuthService` is the core service that handles authentication logic:

```go
// AuthService represents the authentication service
type AuthService struct {
	config config.AuthConfig
	users map[string]models.User
}
```

#### Methods

- `NewAuthService(cfg config.AuthConfig) *AuthService`: Creates a new authentication service
- `Authenticate(username, password string) (*models.User, error)`: Validates user credentials
- `GenerateToken(user *models.User) (string, error)`: Generates a JWT token for a user
- `RegisterUser(username, password string) (*models.User, error)`: Registers a new user

### AuthHandler

`AuthHandler` handles HTTP requests related to authentication:

```go
// AuthHandler handles authentication-related requests
type AuthHandler struct {
	service *services.AuthService
}
```

#### Methods

- `NewAuthHandler(service *services.AuthService) *AuthHandler`: Creates a new authentication handler
- `Login(c *gin.Context)`: Handles login requests
- `Register(c *gin.Context)`: Handles registration requests

### JWT Middleware

The JWT middleware validates JWT tokens for protected routes:

```go
// JWT middleware for authentication
func JWT(secret string) gin.HandlerFunc
```

This middleware:
1. Extracts the token from the Authorization header or cookie
2. Validates the token signature
3. Extracts user claims
4. Sets user information in the context for subsequent handlers

## Authentication Flow

1. **Login Flow**:
   ```
   User -> Login Form -> AuthHandler.Login -> AuthService.Authenticate -> AuthService.GenerateToken -> JWT Token -> User
   ```

2. **Registration Flow**:
   ```
   User -> Registration Form -> AuthHandler.Register -> AuthService.RegisterUser -> AuthService.GenerateToken -> JWT Token -> User
   ```

3. **Protected Route Flow**:
   ```
   User Request -> JWT Middleware -> Extract Token -> Validate Token -> Extract Claims -> Set Context -> Handler
   ```

## Models

### User

```go
// User represents a user in the system
type User struct {
	ID       string `json:"id"`
	Username string `json:"username"`
	Password string `json:"-"` // Password is not serialized to JSON
}
```

### LoginRequest

```go
// LoginRequest represents a login request
type LoginRequest struct {
	Username string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required"`
}
```

### RegisterRequest

```go
// RegisterRequest represents a registration request
type RegisterRequest struct {
	Username string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required"`
}
```

## JWT Implementation

The JWT implementation uses the `github.com/golang-jwt/jwt/v5` package:

1. **Token Creation**:
   ```go
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
   ```

2. **Token Validation**:
   ```go
   // Parse and validate the token
   token, err := jwt.Parse(bearerToken[1], func(token *jwt.Token) (interface{}, error) {
       if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
           return nil, jwt.ErrSignatureInvalid
       }
       return []byte(secret), nil
   })
   ```

## Security Considerations

- Password Storage: In a production environment, passwords should be hashed using a secure algorithm like bcrypt
- JWT Secret: The JWT secret should be a strong, randomly generated string stored securely
- Token Expiration: Tokens have an expiration time to limit the window of opportunity for attacks
- HTTPS: In production, all communication should be over HTTPS to prevent token theft

## Future Enhancements

- Password hashing with bcrypt
- Email verification
- Password reset functionality
- Role-based access control
- OAuth integration for third-party authentication
