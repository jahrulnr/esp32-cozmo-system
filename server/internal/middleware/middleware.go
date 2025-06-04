package middleware

import (
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
)

// JWT middleware for authentication
func JWT(secret string) gin.HandlerFunc {
	return func(c *gin.Context) {
		// Get the Authorization header
		authHeader := c.GetHeader("Authorization")
		if authHeader == "" {
			// Check if JWT cookie exists
			tokenCookie, err := c.Cookie("auth_token")
			if err != nil {
				c.HTML(http.StatusUnauthorized, "login.html", gin.H{
					"error": "Unauthorized: Authentication required",
				})
				c.Abort()
				return
			}
			authHeader = "Bearer " + tokenCookie
		}

		// Check the header format
		bearerToken := strings.Split(authHeader, " ")
		if len(bearerToken) != 2 || bearerToken[0] != "Bearer" {
			c.JSON(http.StatusUnauthorized, gin.H{
				"error": "Invalid token format",
			})
			c.Abort()
			return
		}

		// Parse and validate the token
		token, err := jwt.Parse(bearerToken[1], func(token *jwt.Token) (interface{}, error) {
			if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, jwt.ErrSignatureInvalid
			}
			return []byte(secret), nil
		})

		if err != nil || !token.Valid {
			c.HTML(http.StatusUnauthorized, "login.html", gin.H{
				"error": "Invalid or expired token",
			})
			c.Abort()
			return
		}

		// Extract claims and set them to context
		claims, ok := token.Claims.(jwt.MapClaims)
		if !ok {
			c.JSON(http.StatusUnauthorized, gin.H{
				"error": "Failed to extract claims",
			})
			c.Abort()
			return
		}

		// Set user ID to context
		c.Set("userID", claims["sub"])
		c.Set("username", claims["username"])
		c.Next()
	}
}

// CORS middleware
func CORS() gin.HandlerFunc {
	return func(c *gin.Context) {
		c.Writer.Header().Set("Access-Control-Allow-Origin", "*")
		c.Writer.Header().Set("Access-Control-Allow-Credentials", "true")
		c.Writer.Header().Set("Access-Control-Allow-Headers", "Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization, accept, origin, Cache-Control, X-Requested-With")
		c.Writer.Header().Set("Access-Control-Allow-Methods", "POST, OPTIONS, GET, PUT, DELETE")

		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}

		c.Next()
	}
}
