package main

import (
	"log"

	"cozmo-clouds/internal/app"
	"cozmo-clouds/internal/config"
)

func main() {
	// Load configurations
	cfg, err := config.Load()
	if err != nil {
		log.Fatalf("Failed to load configuration: %v", err)
	}

	// Initialize and run the application
	app := app.New(cfg)
	if err := app.Run(); err != nil {
		log.Fatalf("Failed to run application: %v", err)
	}
}
