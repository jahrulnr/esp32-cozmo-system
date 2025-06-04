package services

import (
	"cozmo-clouds/internal/config"
	"fmt"
	"os"
	"path/filepath"
	"time"

	htgotts "github.com/hegedustibor/htgo-tts"
)

// TTSService represents the Text-to-Speech service
type TTSService struct {
	tts       *htgotts.Speech
	config    config.TTSConfig
	outputDir string
}

// NewTTSService creates a new Text-to-Speech service
func NewTTSService(cfg config.TTSConfig) *TTSService {
	// Create TTS output directory if it doesn't exist
	os.MkdirAll(cfg.OutputDir, os.ModePerm)

	speech := &htgotts.Speech{
		Folder:   cfg.OutputDir,
		Language: cfg.Language,
	}

	return &TTSService{
		tts:       speech,
		config:    cfg,
		outputDir: cfg.OutputDir,
	}
}

// TextToSpeech converts text to speech and returns the audio file path
func (s *TTSService) TextToSpeech(text string) (string, error) {
	// Generate a unique filename based on timestamp
	timestamp := time.Now().UnixNano()
	filename := fmt.Sprintf("tts_%d", timestamp)

	// Create a temporary speech instance for this request
	// The library typically automatically appends .mp3 extension
	speech := &htgotts.Speech{
		Folder:   s.config.OutputDir,
		Language: s.config.Language,
	}

	// The htgo-tts library doesn't provide direct filename control
	// We need to call Speak and then find the generated file
	_ = speech.Speak(text)

	// The library generates files with specific naming convention
	// Let's find the most recently created file in the output directory
	files, err := filepath.Glob(filepath.Join(s.config.OutputDir, "*.mp3"))
	if err != nil {
		return "", fmt.Errorf("failed to find audio files: %v", err)
	}

	if len(files) == 0 {
		return "", fmt.Errorf("no audio file was generated")
	}

	// Find the most recent file
	var newestFile string
	var newestTime time.Time
	for _, file := range files {
		info, err := os.Stat(file)
		if err != nil {
			continue
		}
		if info.ModTime().After(newestTime) {
			newestTime = info.ModTime()
			newestFile = file
		}
	}

	if newestFile == "" {
		return "", fmt.Errorf("no valid audio file found")
	}

	// Rename the file to our desired filename
	finalPath := filepath.Join(s.config.OutputDir, filename+".mp3")
	err = os.Rename(newestFile, finalPath)
	if err != nil {
		// If rename fails, just use the original file
		finalPath = newestFile
	}

	// Return the relative path from the static directory
	return fmt.Sprintf("/static/tts/%s", filepath.Base(finalPath)), nil
}
