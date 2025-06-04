# Text-to-Speech Component

This document describes the Text-to-Speech (TTS) component of the Cozmo Web Interface.

## Overview

The Text-to-Speech component is responsible for:
- Converting text to speech audio
- Managing audio file storage
- Providing audio files to the client
- Integrating with the chat system for voice output

## Components

### TTSService

`TTSService` is the core service that handles text-to-speech conversion:

```go
// TTSService represents the Text-to-Speech service
type TTSService struct {
	tts      *htgotts.Speech
	config   config.TTSConfig
	outputDir string
}
```

#### Methods

- `NewTTSService(cfg config.TTSConfig) *TTSService`: Creates a new TTS service
- `TextToSpeech(text string) (string, error)`: Converts text to speech and returns the audio file path

### TTSHandler

`TTSHandler` handles HTTP requests for TTS functionality:

```go
// TTSHandler handles Text-to-Speech requests
type TTSHandler struct {
	service *services.TTSService
}
```

#### Methods

- `NewTTSHandler(service *services.TTSService) *TTSHandler`: Creates a new TTS handler
- `Convert(c *gin.Context)`: Handles TTS conversion requests

## TTS Flow

1. **TTS Request Flow**:
   ```
   Client -> TTSHandler.Convert -> TTSService.TextToSpeech -> 
   Generate Audio File -> Return Audio File Path -> Client -> Play Audio
   ```

## Models

### TTSRequest

```go
// TTSRequest represents a Text-to-Speech request
type TTSRequest struct {
	Text string `json:"text" binding:"required"`
}
```

## htgo-tts Integration

The TTS implementation uses the `github.com/hegedustibor/htgo-tts` package:

1. **Speech Initialization**:
   ```go
   speech := &htgotts.Speech{
       Folder:   cfg.OutputDir,
       Language: cfg.Language,
   }
   ```

2. **Text-to-Speech Conversion**:
   ```go
   err := s.tts.Speak(text)
   ```

## Configuration

The TTS service is configured through the following settings:

```go
// TTSConfig holds Text-to-Speech specific configuration
type TTSConfig struct {
	OutputDir string
	Language  string
}
```

- `OutputDir`: The directory where audio files are stored
- `Language`: The language to use for speech synthesis (default: "en")

## File Management

The TTS service generates unique filenames for each audio file:

```go
// Generate a unique filename based on timestamp
timestamp := time.Now().UnixNano()
filename := fmt.Sprintf("tts_%d", timestamp)
```

Audio files are stored in the output directory and are accessible through the `/static/tts/` URL path.

## Frontend Integration

The frontend sends TTS requests to the server and plays the resulting audio:

```javascript
function textToSpeech(text) {
    $.ajax({
        url: '/api/tts',
        type: 'POST',
        contentType: 'application/json',
        headers: {
            'Authorization': `Bearer ${localStorage.getItem('auth_token')}`
        },
        data: JSON.stringify({
            text: text
        }),
        success: function(response) {
            // Create audio element and play
            const audio = new Audio(response.audioPath);
            audio.play();
        },
        error: function(xhr) {
            console.error('TTS Error:', xhr);
        }
    });
}
```

## ChatGPT Integration

The TTS service integrates with the chat system to provide voice output for chat responses:

```javascript
// Add bot message to chat
addMessage(response.message.content, true);

// Convert response to speech if enabled
if ($('#voice-enabled').is(':checked')) {
    textToSpeech(response.message.content);
}

// Make Cozmo speak the response if enabled
if ($('#cozmo-speak').is(':checked')) {
    sendToCozmo('speak', response.message.content);
}
```

## Cozmo Integration

The TTS service can also send speech commands to the Cozmo robot:

```javascript
function sendToCozmo(type, content) {
    // This would typically connect to your WebSocket handler
    console.log('Sending to Cozmo:', type, content);
}
```

## Implementation Details

### File Storage

Audio files are stored in the `/web/static/tts/` directory, which is served as a static directory by the Gin framework:

```go
router.Static("/static", cfg.Server.StaticFilePath)
```

### Audio Format

The htgo-tts library generates MP3 audio files.

### File Cleanup

Currently, the implementation does not include automatic cleanup of old audio files. In a production environment, a cleanup routine should be implemented to prevent disk space issues.

## Security Considerations

- File Size Limits: Implement limits on the length of text that can be converted to prevent resource exhaustion
- Directory Permissions: Ensure the output directory has appropriate permissions
- Input Sanitization: Sanitize input text to prevent potential injection attacks

## Performance Considerations

- Caching: Consider caching common phrases to reduce processing time and disk usage
- File Size: Monitor file sizes to ensure they don't grow too large
- Rate Limiting: Implement rate limiting to prevent abuse

## Future Enhancements

- Multiple language support
- Voice selection options
- Speech rate and pitch controls
- Audio format options
- Streaming audio for long texts
- Integration with advanced TTS services like Google Cloud TTS or Amazon Polly
- Audio file caching for common phrases
