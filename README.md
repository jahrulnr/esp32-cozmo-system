# ESP32 Cozmo Robot System

An intelligent robot system built with ESP32-S3, featuring speech recognition, text-to-speech, vision capabilities features. This project uses PlatformIO with the Arduino framework and implements a custom MVC web framework for control and monitoring.

## 🚀 Features

- Speech Recognition & Text-to-Speech (PicoTTS)
- Computer Vision with ESP32-CAM
- Facial Expressions via OLED Display
- Motor & Servo Control
- Multiple Sensor Integration:
  - Ultrasonic Distance Sensor
  - Cliff Detectors
  - Touch Sensors
  - Temperature Sensor
  - Orientation Sensor
- Web-based Control Interface
- FTP Server for File Management
- Weather Service Integration
- GPT Integration for AI capabilities

## 🛠️ Hardware Requirements

### Core Components
- ESP32-S3 with 16MB Flash and 8MB PSRAM
- OLED Display (SSD1306 I2C)
- PCF8575 I2C Expander (for motors/sensors)
- HC-SR04 Ultrasonic Sensor
- Servo Motors
- MAX98357 I2S Audio Amplifier
- I2S Microphone
- Various sensors (cliff, touch, temperature)

### GPIO Connections
- **Display & I2C**: GPIO 3 (SDA), GPIO 46 (SCL)
- **Audio**: 
  - Microphone: GPIO 21, 47, 14
  - Speaker: GPIO 42, 2, 41
- **Servos**: 
  - Head: GPIO 19
  - Hand: GPIO 20
- **Sensors**: 
  - Ultrasonic: GPIO 0 (trigger), GPIO 45 (echo)
- **Motors**: Controlled via PCF8575 I2C expander
- **SD Card**: GPIO 39, 38, 40 (SD_MMC)

## ⚙️ Setup & Configuration

1. Install PlatformIO
2. Clone the repository
3. Copy configuration file:
   ```bash
   cp include/Config.h.example include/Config.h
   ```
4. Configure hardware pins, WiFi credentials, and API keys in `Config.h`
5. Build and flash:
   ```bash
   # Build for ESP32-S3
   pio run -e esp32s3dev
   
   # Upload firmware
   pio run -e esp32s3dev -t upload
   
	 # Flash speech recognition models (required for voice features)
	 esptool.py --baud 2000000 write_flash 0x47D000 model/srmodels.bin
		
	 # Flash PicoTTS models (required for text-to-speech)
	 esptool.py --baud 2000000 write_flash 0x310000 model/picoTTS/en-US_ta.bin
	 esptool.py --baud 2000000 write_flash 0x3B0000 model/picoTTS/en-US_lh0_sg.bin
   ```

## 📁 Project Structure

```
/app
  ├── app.ino              # Main application entry
  ├── core/                # Core functionality
  │   ├── Motors/          # Motor and servo control
  │   └── Sensors/         # Various sensor implementations
  ├── display/             # Display and face animations
  ├── web/                 # Web interface
  │   ├── Controllers/     # MVC controllers
  │   └── Routes/          # API routes
  └── setup/               # Component initialization
/data                      # Web assets and configs
/include                   # Configuration headers
/lib                       # Custom libraries
/model                     # Speech recognition models
```

## 🔧 Key Dependencies

- U8g2 (Display Graphics)
- ESP32Servo
- PCF8575 Library
- ESP32 Notification System
- ESP32 Microphone
- ESP32 PicoTTS
- ESP32 MVC Framework
- FTP Client/Server
- AsyncWebServer

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a new Pull Request

## ⚠️ Important Notes

- Memory management is critical due to ESP32-S3 limitations
- All hardware features can be enabled/disabled via Config.h
- Web interface requires authentication
- System runs on FreeRTOS tasks with 120s watchdog timeout
- Display system has specific thread-safety requirements

## 📝 License

This project includes components with varying licenses. Please check individual source files and libraries for specific licensing information.
