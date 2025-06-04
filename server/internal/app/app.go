package app

import (
	"fmt"

	"cozmo-clouds/internal/config"
	"cozmo-clouds/internal/handlers"
	"cozmo-clouds/internal/middleware"
	"cozmo-clouds/internal/services"

	"github.com/gin-gonic/gin"
)

// App represents the application
type App struct {
	router *gin.Engine
	config *config.Config
}

// New creates a new application instance
func New(cfg *config.Config) *App {
	// Set gin mode based on environment
	if cfg.Server.Environment == "production" {
		gin.SetMode(gin.ReleaseMode)
	}

	router := gin.New()

	// Initialize services
	authService := services.NewAuthService(cfg.Auth)
	openaiService := services.NewOpenAIService(cfg.OpenAI)
	ttsService := services.NewTTSService(cfg.TTS)
	sttService := services.NewSTTService()
	cozmoWSService := services.NewCozmoWebSocketService(cfg.WebSocket)

	// Initialize handlers
	authHandler := handlers.NewAuthHandler(authService)
	viewHandler := handlers.NewViewHandler(cfg)
	chatHandler := handlers.NewChatHandler(openaiService)
	ttsHandler := handlers.NewTTSHandler(ttsService)
	sttHandler := handlers.NewSTTHandler(sttService)
	wsHandler := handlers.NewWebSocketHandler(cozmoWSService)

	// Start WebSocket service in a goroutine
	go cozmoWSService.Run(openaiService, ttsService)

	// Load templates - using partials pattern with header/footer includes
	router.LoadHTMLGlob(fmt.Sprintf("%s/*.html", cfg.Server.TemplatesPath))

	// Setup static file serving
	router.Static("/assets", cfg.Server.AssetsFilePath)
	router.Static("/static", cfg.Server.StaticFilePath)

	// Setup middleware
	router.Use(middleware.CORS())

	// Public routes
	router.GET("/", viewHandler.Home)
	router.GET("/login", viewHandler.Login)
	router.POST("/login", authHandler.Login)
	router.POST("/register", authHandler.Register)

	// Protected routes
	protected := router.Group("/")
	protected.Use(middleware.JWT(cfg.Auth.JWTSecret))
	{
		// Views
		protected.GET("/dashboard", viewHandler.Dashboard)
		protected.GET("/chat", viewHandler.Chat)
		protected.GET("/settings", viewHandler.Settings)

		// API endpoints
		api := protected.Group("/api")
		{
			api.POST("/chat", chatHandler.Chat)
			api.POST("/tts", ttsHandler.Convert)
			api.POST("/stt", sttHandler.Convert)
		}

		// WebSocket endpoints
		protected.GET("/ws/browser", wsHandler.HandleBrowser) // Browser connections
		protected.GET("/ws/robot", wsHandler.HandleRobot)     // Robot connections
	}

	return &App{
		router: router,
		config: cfg,
	}
}

// Run starts the application
func (a *App) Run() error {
	addr := fmt.Sprintf("%s:%s", a.config.Server.Host, a.config.Server.Port)
	return a.router.Run(addr)
}
