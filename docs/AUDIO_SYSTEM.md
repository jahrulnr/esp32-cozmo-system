# Cozmo Audio System Documentation

## Overview

The Cozmo system includes a comprehensive audio playback system that supports both PWM and I2S speakers. The system can play various types of audio including tones, sound effects, and audio files stored in SPIFFS.

## Supported Speaker Types

### 1. PWM Speaker
- **Configuration**: Simple speaker connected to a digital pin
- **Pin**: GPIO 25 (configurable in Config.h)
- **Sample Rate**: 8kHz recommended for audio data
- **Best For**: Simple tones, beeps, and basic audio feedback
- **Audio Quality**: Basic, suitable for system sounds

### 2. I2S Speaker (MAX98357A)
- **Configuration**: MAX98357A I2S audio amplifier
- **Pins**: 
  - BCLK (Bit Clock): GPIO 26
  - WCLK (Word Clock): GPIO 27  
  - DIN (Data In): GPIO 25 (connects to MAX98357's DIN pin, not SD)
- **Sample Rates**: 8kHz, 16kHz, 22kHz, 44.1kHz, 48kHz
- **Default Sample Rate**: 16kHz
- **Best For**: High-quality audio playback, music, speech
- **Audio Quality**: High-quality digital audio

## Supported Audio Types

### 1. System Sounds (Built-in)
- **Beep**: Single beep sound
- **Double Beep**: Two consecutive beeps
- **Confirmation**: Rising tone sequence (success sound)
- **Error**: Descending tone sequence (error sound)
- **Startup**: Power-on sound sequence
- **Notification**: Attention-getting sound

### 2. Custom Tones
- **Frequency Range**: 20Hz - 20kHz
- **Duration**: 10ms - 10 seconds
- **Volume**: 0-100%

### 3. Raw Audio Data
- **Format**: 16-bit PCM (little-endian)
- **Channels**: Mono or Stereo (I2S supports stereo better)
- **Sample Rates**: 
  - PWM: 8kHz recommended
  - I2S: 8kHz, 16kHz, 22kHz, 44.1kHz, 48kHz
- **Source**: Memory arrays or SPIFFS files

### 4. Custom Audio Files (CZMO Format)
- **Format**: Custom format with header + raw PCM data
- **Header**: 16 bytes containing magic number, sample rate, sample count, bit depth
- **Data**: Raw 16-bit PCM samples following header
- **Storage**: SPIFFS filesystem

### 5. MP3 Files (NEW!)
- **Format**: Standard MP3 files
- **Sample Rates**: Any (automatically decoded to PCM)
- **Channels**: Mono or Stereo
- **Bit Rates**: Any (128kbps, 192kbps, 320kbps, etc.)
- **Decoder**: ESP32 Helix MP3 decoder library
- **Storage**: SPIFFS filesystem
- **Advantages**: Standard format, smaller file sizes, widely supported
- **Note**: Files are decoded to PCM in memory for playback

## Configuration

### Enable/Disable Audio
```cpp
#define SPEAKER_ENABLED true
```

### Choose Speaker Type
```cpp
// PWM Speaker
#define SPEAKER_TYPE_PWM true
#define SPEAKER_TYPE_I2S false

// OR I2S Speaker  
#define SPEAKER_TYPE_PWM false
#define SPEAKER_TYPE_I2S true
```

### PWM Speaker Settings
```cpp
#define PWM_SPEAKER_PIN 25
#define PWM_SPEAKER_CHANNEL 1
#define PWM_SPEAKER_DEFAULT_VOLUME 50
```

### I2S Speaker Settings
```cpp
#define I2S_SPEAKER_BCLK_PIN 26          // Bit Clock
#define I2S_SPEAKER_WCLK_PIN 27          // Word Clock (LR Clock)
#define I2S_SPEAKER_DATA_PIN 25          // Data In (connects to MAX98357 DIN)
#define I2S_SPEAKER_SAMPLE_RATE 16000    // Sample rate in Hz
#define I2S_SPEAKER_BITS_PER_SAMPLE 16   // Bit depth
#define I2S_SPEAKER_DEFAULT_VOLUME 50    // Default volume (0-100)
```

## Programming Interface

### Basic Sound Functions
```cpp
// Play system sounds
void playSpeakerBeep(int volume);
void playSpeakerConfirmation(int volume);
void playSpeakerError(int volume);
void playSpeakerNotification(int volume);
void playSpeakerStartup(int volume);

// Play custom tone
void playSpeakerTone(int frequency, int duration, int volume);

// Control playback
void stopSpeaker();
void setSpeakerVolume(int volume);
bool isSpeakerPlaying();
```

### Audio File Functions
```cpp
// Play audio file from SPIFFS
bool playSpeakerAudioFile(const String& filePath, int volume);

// Play raw audio data from memory
void playSpeakerAudioData(const uint8_t* data, size_t dataSize, uint32_t sampleRate, int volume);

// Create custom audio file in SPIFFS
bool createAudioFile(const String& filePath, const int16_t* samples, size_t sampleCount, uint32_t sampleRate);
```

### MP3 Functions (NEW!)
```cpp
// Play MP3 file directly from SPIFFS
bool playSpeakerMP3File(const String& filePath, int volume);

// Get MP3 file information
bool getMP3FileInfo(const String& filePath, int* sampleRate, int* channels, int* bitRate, int* duration);

// Convert MP3 to custom CZMO format (for faster repeated playback)
bool convertMP3ToAudioFile(const String& mp3FilePath, const String& audioFilePath);
```

## Command Interface

### Voice/Remote Control Commands

| Command | Parameter Format | Description | Example |
|---------|------------------|-------------|---------|
| `PLAY_BEEP` | volume (0-100) | Play simple beep | `[PLAY_BEEP=50]` |
| `PLAY_TONE` | freq,duration,volume | Play custom tone | `[PLAY_TONE=440,1000,50]` |
| `PLAY_CONFIRMATION` | volume (0-100) | Play success sound | `[PLAY_CONFIRMATION=70]` |
| `PLAY_ERROR` | volume (0-100) | Play error sound | `[PLAY_ERROR=60]` |
| `PLAY_NOTIFICATION` | volume (0-100) | Play alert sound | `[PLAY_NOTIFICATION=80]` |
| `PLAY_AUDIO_FILE` | filepath,volume | Play SPIFFS audio file | `[PLAY_AUDIO_FILE=/sounds/hello.czmo,60]` |
| `STOP_AUDIO` | none | Stop current playback | `[STOP_AUDIO]` |
| `SET_VOLUME` | volume (0-100) | Set speaker volume | `[SET_VOLUME=75]` |

### MP3 Commands (NEW!)

| Command | Parameter Format | Description | Example |
|---------|------------------|-------------|---------|
| `PLAY_MP3_FILE` | filepath,volume | Play MP3 file from SPIFFS | `[PLAY_MP3_FILE=/sounds/music.mp3,60]` |
| `MP3_INFO` | filepath | Get MP3 file information | `[MP3_INFO=/sounds/song.mp3]` |
| `CONVERT_MP3` | source.mp3,dest.czmo | Convert MP3 to CZMO format | `[CONVERT_MP3=/sounds/music.mp3,/sounds/music.czmo]` |

### Usage Examples
```cpp
// Simple beep at 50% volume
executeCommand("[PLAY_BEEP=50]");

// Play 440Hz tone for 2 seconds at 70% volume
executeCommand("[PLAY_TONE=440,2000,70]");

// Play audio file from SPIFFS
executeCommand("[PLAY_AUDIO_FILE=/sounds/welcome.czmo,60]");

// Stop any playing audio
executeCommand("[STOP_AUDIO]");
```

## Creating Audio Files

### Method 1: From Raw Samples
```cpp
// Create sample data (sine wave example)
int16_t samples[1000];
for (int i = 0; i < 1000; i++) {
    samples[i] = (int16_t)(sin(2 * PI * 440 * i / 8000) * 16383); // 440Hz tone
}

// Save to SPIFFS
bool success = createAudioFile("/sounds/tone440.czmo", samples, 1000, 8000);
```

### Method 2: Convert Existing Audio
To use existing WAV files, you would need to:
1. Convert to 16-bit PCM format
2. Match the sample rate (8kHz for PWM, 16kHz for I2S)
3. Extract raw PCM data
4. Use `createAudioFile()` or save directly to SPIFFS

## Hardware Connections

### PWM Speaker
```
ESP32 GPIO 25 ──┐
                │
               ┌─┴─┐
               │ + │ Speaker (8Ω recommended)
               │ - │
               └─┬─┘
                │
GND ────────────┘
```

### I2S Speaker (MAX98357A)
```
ESP32 GPIO 26 ──── BCLK (Bit Clock)
ESP32 GPIO 27 ──── LRCLK (Word Clock)  
ESP32 GPIO 25 ──── DIN (Data In)
ESP32 3.3V ─────── VDD
ESP32 GND ──────── GND
                   DOUT ──── Speaker +
                   Speaker - ──── GND
```

**Important**: The `I2S_SPEAKER_DATA_PIN` connects to the MAX98357's **DIN** pin (Data In), not the SD (Shutdown) pin. The SD pin can be tied to VDD for always-on operation.

## File System Usage

### SPIFFS Audio Files
- **Path**: `/sounds/filename.czmo`
- **Max Size**: Limited by SPIFFS partition size
- **Format**: Custom CZMO format with 16-byte header
- **Recommended Location**: `/data/sounds/` directory for web interface access

### Example File Structure
```
/data/sounds/
├── beep.czmo
├── welcome.czmo
├── error.czmo
└── notification.czmo
```

## Integration with Other Systems

### Automation System
```cpp
// Play sounds based on robot behaviors
void playBehaviorSound(const String& behavior);

// Examples:
playBehaviorSound("happy");        // Plays confirmation sound
playBehaviorSound("sad");          // Plays error sound
playBehaviorSound("surprised");    // Plays beep
```

### WebSocket API
The audio commands can be sent via WebSocket for remote control:
```javascript
// Send audio command via WebSocket
websocket.send('{"command":"[PLAY_BEEP=50]"}');
websocket.send('{"command":"[PLAY_AUDIO_FILE=/sounds/hello.czmo,60]"}');
```

## Troubleshooting

### Common Issues

1. **No Sound Output**
   - Check speaker connections
   - Verify `SPEAKER_ENABLED` is true
   - Check pin assignments in Config.h
   - Ensure speaker is compatible (8Ω for PWM)

2. **Poor Audio Quality (PWM)**
   - Use lower sample rates (8kHz)
   - Keep audio files short
   - Consider upgrading to I2S speaker

3. **I2S Initialization Failed**
   - Check pin connections to MAX98357A
   - Verify power supply (3.3V)
   - Ensure no pin conflicts with other peripherals

4. **File Playback Issues**
   - Verify file exists in SPIFFS
   - Check file format (use CZMO format)
   - Ensure sufficient SPIFFS space
   - Verify FileManager is initialized

### Debug Tips
- Enable debug logging to see audio system status
- Use `getSpeakerStatus()` and `getSpeakerType()` for diagnostics
- Test with built-in sounds first before trying file playback
- Monitor SPIFFS usage for file storage issues

## Future Enhancements

Potential improvements for the audio system:

1. **WAV File Support**: Add parser for standard WAV files
2. **Streaming Audio**: Support for longer audio files with streaming
3. **Audio Effects**: Add reverb, echo, or pitch shifting
4. **Multiple Files**: Queue system for playing multiple files
5. **Voice Synthesis**: Text-to-speech integration
6. **Audio Recording**: Microphone input and recording capabilities
