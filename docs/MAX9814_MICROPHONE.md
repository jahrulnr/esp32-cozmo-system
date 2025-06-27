# MAX9814 Microphone Sensor Integration

## Overview

The Cozmo system now includes support for the MAX9814 electret microphone amplifier with automatic gain control (AGC). This integration enables sound detection, audio level monitoring, and sound-triggered behaviors.

## Hardware Configuration

### Pin Connections

**Development Mode:**
- Analog Pin: GPIO 8
- Gain Pin: GPIO 9
- Attack/Release Pin: Not used (-1)

**Production Mode:**
- Analog Pin: GPIO 34
- Gain Pin: GPIO 35
- Attack/Release Pin: Not used (-1)

### MAX9814 Module

The MAX9814 is a microphone amplifier with the following features:
- Automatic Gain Control (AGC)
- Adjustable gain: 40dB, 50dB, or 60dB
- Attack/Release time control
- Low noise, high-quality audio amplification

## Software Features

### Initialization

The microphone sensor is automatically initialized during system startup with:
- 50dB gain (middle setting for balanced sensitivity)
- Fast attack/release for better responsiveness
- Baseline noise calibration

### Sound Detection

The system monitors sound levels and provides:
- **Current Level**: Real-time audio level (0-4095)
- **Peak Level**: Peak level over a sampling period
- **Sound Detection**: Boolean detection above threshold
- **Configurable Thresholds**: Different levels for quiet, moderate, and loud sounds

### Automated Behaviors

When automation is enabled, the system reacts to sounds with appropriate behaviors:
- **Quiet sounds**: Perk up slightly, show alert expression
- **Moderate sounds**: Turn head towards sound, show curious expression  
- **Loud sounds**: Look around nervously, show surprised expression

### WebSocket API

#### Get Microphone Data
```json
{
  "type": "microphone_request"
}
```

Response:
```json
{
  "type": "sensor_data",
  "data": {
    "microphone": {
      "level": 1234,
      "peak": 2345,
      "detected": true
    }
  }
}
```

#### Real-time Monitoring

Microphone data is included in the continuous sensor data stream sent by the sensor monitoring task.

### Voice Commands

The following commands are available through the command mapper:

- `MIC_CALIBRATE`: Recalibrate microphone baseline
- `MIC_GAIN_LOW`: Set gain to 40dB
- `MIC_GAIN_MID`: Set gain to 50dB  
- `MIC_GAIN_HIGH`: Set gain to 60dB

### Configuration

Configuration options in `Config.h`:

```cpp
#define MICROPHONE_ENABLED true
#define MICROPHONE_ANALOG_PIN 34
#define MICROPHONE_GAIN_PIN 35
#define MICROPHONE_AR_PIN -1
#define MICROPHONE_SOUND_THRESHOLD 2000
#define MICROPHONE_BASELINE_CALIBRATION_TIME 500
```

## Usage Examples

### Manual Control

```cpp
// Check if sound is detected
if (isSoundDetected()) {
  // React to sound
}

// Get current sound level
int level = getCurrentSoundLevel();

// Calibrate for current environment
calibrateMicrophone();

// Adjust sensitivity
setMicrophoneGain(HIGH);
```

### Integration with Automation

The microphone automatically integrates with the automation system, triggering appropriate behaviors when sounds are detected and automation is enabled.

## Troubleshooting

### No Sound Detection

1. Check pin connections
2. Verify microphone module power
3. Recalibrate baseline: `MIC_CALIBRATE`
4. Adjust gain if needed

### Too Sensitive

1. Lower gain: `MIC_GAIN_LOW`
2. Increase threshold in configuration
3. Recalibrate in quiet environment

### Not Sensitive Enough

1. Increase gain: `MIC_GAIN_HIGH`
2. Decrease threshold in configuration
3. Check microphone positioning

## Technical Details

- **Sampling Rate**: Continuous monitoring with 100ms intervals
- **ADC Resolution**: 12-bit (0-4095 range)
- **Behavior Cooldown**: 5 seconds between sound-triggered behaviors
- **Baseline Calibration**: Automatic on startup, manual recalibration available

## Future Enhancements

Potential improvements for future versions:
- Frequency analysis for different sound types
- Voice activity detection
- Sound direction estimation
- Integration with speech recognition
- Custom sound pattern recognition
