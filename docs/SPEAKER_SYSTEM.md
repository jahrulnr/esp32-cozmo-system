# Speaker System Integration

## Overview

The Cozmo system now includes comprehensive audio support with two speaker options:
1. **PWM Speaker** - Simple, low-cost solution using PWM on any digital pin
2. **MAX98357 I2S Speaker** - High-quality digital audio amplifier for professional audio output

Both speaker types are optional and can be configured independently through the configuration system.

## Hardware Configuration

### PWM Speaker Setup

**Development Mode:**
- Speaker Pin: GPIO 25
- PWM Channel: 1
- Default Volume: 50%

**Production Mode:**
- Speaker Pin: GPIO 25  
- PWM Channel: 1
- Default Volume: 50%

**Wiring:**
- Connect speaker positive terminal to configured GPIO pin
- Connect speaker negative terminal to GND
- Optional: Add series resistor (100-470Ω) to limit current

### MAX98357 I2S Speaker Setup

**Development Mode:**
- BCLK (Bit Clock): GPIO 26
- WCLK (Word Clock/LR): GPIO 27
- DIN (Data): GPIO 25
- Sample Rate: 16kHz
- Bits per Sample: 16-bit

**Production Mode:**
- BCLK (Bit Clock): GPIO 26
- WCLK (Word Clock/LR): GPIO 27  
- DIN (Data): GPIO 25
- Sample Rate: 16kHz
- Bits per Sample: 16-bit

**Wiring:**
- VIN: 3.3V or 5V
- GND: Ground
- BCLK: Bit Clock pin
- WCLK/LRC: Word/Left-Right Clock pin
- DIN: Data Input pin
- GAIN: Leave floating for 15dB gain (or connect to VIN/GND for other gains)
- SD: Connect to GND for normal operation

## Software Features

### Speaker Types

Choose between speaker types in `Config.h`:

```cpp
#define SPEAKER_ENABLED true
#define SPEAKER_TYPE_PWM true   // Enable PWM speaker
#define SPEAKER_TYPE_I2S false  // Enable I2S speaker
```

### Audio Capabilities

#### PWM Speaker
- **Frequency Range**: 20Hz - 20kHz
- **Resolution**: 8-bit PWM
- **Features**: Tones, beeps, simple melodies
- **CPU Usage**: Low
- **Audio Quality**: Basic

#### I2S Speaker (MAX98357)
- **Frequency Range**: Full audio spectrum
- **Resolution**: 16-bit digital audio
- **Features**: High-quality tones, potential for audio playback
- **CPU Usage**: Moderate
- **Audio Quality**: Professional grade

### Sound Library

Both speakers support the same API:

#### Basic Sounds
- `playSpeakerBeep(volume)` - Simple beep
- `playSpeakerConfirmation(volume)` - Rising tone confirmation
- `playSpeakerError(volume)` - Descending error tone
- `playSpeakerNotification(volume)` - Triple beep notification
- `playSpeakerStartup(volume)` - Musical startup melody

#### Advanced Control
- `playSpeakerTone(frequency, duration, volume)` - Custom tone
- `setSpeakerVolume(volume)` - Set default volume (0-100)
- `stopSpeaker()` - Stop any playing sound
- `isSpeakerPlaying()` - Check if audio is playing

### Voice Commands

Available through the command mapper:

- `SPEAKER_BEEP[=volume]` - Play beep
- `SPEAKER_TONE[=freq,duration,volume]` - Play custom tone
- `SPEAKER_CONFIRM[=volume]` - Confirmation sound
- `SPEAKER_ERROR[=volume]` - Error sound  
- `SPEAKER_NOTIFY[=volume]` - Notification sound
- `SPEAKER_VOLUME[=volume]` - Set volume
- `SPEAKER_STOP` - Stop playing

### WebSocket API

#### Speaker Control
```json
{
  "type": "speaker_control",
  "data": {
    "action": "beep",
    "volume": 50
  }
}
```

**Actions:**
- `beep` - Simple beep
- `confirm` - Confirmation sound
- `error` - Error sound
- `notify` - Notification sound
- `tone` - Custom tone (requires frequency, duration, volume)
- `stop` - Stop playing
- `volume` - Set volume

#### System Status
Speaker status is included in system status responses:
```json
{
  "speaker": {
    "enabled": true,
    "type": "PWM",
    "playing": false
  }
}
```

### Automation Integration

The speaker system automatically integrates with Cozmo's behaviors:

- **Happy/Joy behaviors** → Confirmation sounds
- **Sad/Disappointed behaviors** → Error sounds  
- **Surprised/Startled behaviors** → Alert beeps
- **Notification behaviors** → Notification sounds

Sounds are triggered automatically when behaviors contain relevant keywords.

## Configuration Options

### PWM Speaker Configuration
```cpp
#define PWM_SPEAKER_PIN 25
#define PWM_SPEAKER_CHANNEL 1
#define PWM_SPEAKER_DEFAULT_VOLUME 50
```

### I2S Speaker Configuration  
```cpp
#define I2S_SPEAKER_BCLK_PIN 26
#define I2S_SPEAKER_WCLK_PIN 27
#define I2S_SPEAKER_DATA_PIN 25
#define I2S_SPEAKER_SAMPLE_RATE 16000
#define I2S_SPEAKER_BITS_PER_SAMPLE 16
#define I2S_SPEAKER_DEFAULT_VOLUME 50
```

## Usage Examples

### Manual Control
```cpp
// Play startup sound
playSpeakerStartup(70);

// Play custom tone
playSpeakerTone(1000, 500, 50); // 1kHz for 500ms at 50% volume

// Set volume and play notification
setSpeakerVolume(30);
playSpeakerNotification();
```

### WebSocket Control
```javascript
// Play beep via WebSocket
websocket.send(JSON.stringify({
  type: "speaker_control",
  data: {
    action: "beep",
    volume: 60
  }
}));

// Play custom tone
websocket.send(JSON.stringify({
  type: "speaker_control", 
  data: {
    action: "tone",
    frequency: 800,
    duration: 300,
    volume: 40
  }
}));
```

### Voice Commands
- `[SPEAKER_BEEP=75]` - Play beep at 75% volume
- `[SPEAKER_TONE=1500,1000,50]` - 1.5kHz tone for 1 second at 50% volume
- `[SPEAKER_CONFIRM]` - Confirmation sound at default volume

## Troubleshooting

### PWM Speaker Issues

**No Sound:**
1. Check pin connections
2. Verify speaker polarity
3. Test with higher volume
4. Check if PWM channel conflicts with other uses

**Distorted Sound:**
1. Lower volume setting
2. Add series resistor
3. Check power supply stability

### I2S Speaker Issues

**No Sound:**
1. Verify all I2S pin connections
2. Check power supply (3.3V or 5V)
3. Ensure GND connections
4. Verify GAIN pin configuration

**Poor Quality:**
1. Check sample rate settings
2. Verify I2S timing
3. Ensure stable power supply
4. Check for electrical interference

### General Issues

**Commands Not Working:**
1. Verify speaker initialization in logs
2. Check configuration settings
3. Ensure speaker type matches configuration

**WebSocket Not Responding:**
1. Check WebSocket connection
2. Verify JSON format
3. Check system status for speaker availability

## Performance Considerations

### PWM Speaker
- **Memory Usage**: Minimal
- **CPU Impact**: Low
- **Concurrent Operations**: Can interfere with other PWM usage
- **Blocking**: Some operations block execution

### I2S Speaker  
- **Memory Usage**: Moderate (DMA buffers)
- **CPU Impact**: Low (hardware-accelerated)
- **Concurrent Operations**: Dedicated I2S hardware
- **Blocking**: Non-blocking operation possible

## Future Enhancements

Potential improvements for future versions:
- WAV file playback support
- Real-time audio streaming
- Multiple simultaneous tones
- Audio synthesis capabilities
- Volume fade in/out
- Custom melody composition tools
- Voice synthesis integration
