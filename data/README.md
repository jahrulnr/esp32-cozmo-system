# Cozmo Robot Control Interface

A beautiful, mobile-friendly web interface for controlling ESP32-based Cozmo robot system.

## Features

### 🎮 **Motor Control**
- Real-time joystick control for movement
- Individual motor speed control
- Emergency stop functionality
- WebSocket for low-latency commands

### 🔧 **Servo Control**
- Head pan/tilt control with sliders
- Hand servo positioning
- Center position reset
- Real-time angle feedback

### 📊 **Sensor Visualization**
- Live accelerometer charts (X, Y, Z axes)
- Gyroscope data visualization
- Distance sensor with obstacle detection
- Cliff detector safety system
- Battery and temperature monitoring

### 🗣️ **Voice Control**
- Voice command recognition
- Wake word detection
- Speech-to-text processing
- Voice feedback system

### 💬 **AI Chat**
- GPT-powered conversation
- Natural language robot commands
- Chat history management
- Voice input/output

### 📱 **Mobile-First Design**
- Responsive layout for all screen sizes
- Touch-optimized controls
- PWA support (install as app)
- Offline capability

### 🌐 **WiFi Management**
- Network status monitoring
- WiFi network scanning
- Connection management
- Signal strength display

## Architecture

### 🏗️ **Clean Modular Design**
```
Frontend Architecture:
├── cozmo-core.js       # API & WebSocket communication
├── cozmo-ui.js         # User interface management
├── cozmo-sensors.js    # Real-time sensor visualization
├── cozmo-app.js        # Application orchestration
└── cozmo-ui.css        # Beautiful, responsive styling
```

### 🔄 **Communication Flow**
```
Frontend ←→ WebSocket ←→ ESP32 ←→ Hardware
    ↕           ↕         ↕        ↕
REST API    Notification  Event   Sensors
Commands    System        System   Motors
```

### 📡 **API Integration**
- RESTful API endpoints for all robot functions
- WebSocket for real-time bidirectional communication
- Authentication and session management
- Error handling and retry logic

## File Structure

### **Frontend Assets** (`/data/assets/`)
- `cozmo-ui.css` - Custom styling with mobile-first responsive design
- `cozmo-core.js` - Core API communication and WebSocket handling
- `cozmo-ui.js` - UI interactions, navigation, and control bindings
- `cozmo-sensors.js` - Canvas-based sensor data visualization
- `cozmo-app.js` - Main application initialization and coordination
- `bootstrap.min.css` - UI framework (existing)
- `bootstrap.bundle.js` - UI components (existing)
- `joy.js` - Joystick control library (existing)

### **Views** (`/data/views/`)
- `app.html` - Single-page application with all interface components

### **Configuration**
- `manifest.json` - PWA configuration for mobile app experience

## Key Features for AI Agents

### 🤖 **Easy Maintenance**
- **Modular architecture** - Each component is self-contained
- **Event-driven communication** - Loose coupling between modules
- **Consistent API patterns** - Predictable request/response format
- **Error boundaries** - Graceful error handling and recovery
- **Comprehensive logging** - Debug information and performance monitoring

### 🔧 **Developer-Friendly**
- **Clean separation of concerns** - UI, API, sensors, and app logic
- **Extensible component system** - Easy to add new features
- **Well-documented APIs** - Clear interface definitions
- **Responsive design patterns** - Mobile-first, progressive enhancement
- **Performance optimization** - Minimal bandwidth usage, efficient rendering

### 📊 **Real-Time Capabilities**
- **WebSocket integration** - Instant bidirectional communication
- **Canvas-based charts** - Smooth 60fps sensor visualization
- **Throttled commands** - Prevents ESP32 overload
- **Connection resilience** - Automatic reconnection and error recovery

## Installation

1. Upload all files to ESP32 LittleFS:
   ```bash
   platformio run -e esp32s3dev -t uploadfs
   ```

2. Access the interface:
   ```
   http://[device-ip]/views/app.html
   or
   http://[device-name].local/views/app.html
   ```

## Mobile Usage

### **Install as App**
1. Open in mobile browser
2. Tap "Add to Home Screen"
3. Use as native app experience

### **Touch Controls**
- **Joystick**: Drag for motor control
- **Sliders**: Servo positioning
- **Buttons**: Emergency stop, voice control
- **Pinch/zoom**: Camera view

## API Endpoints

### **Robot Control**
- `POST /api/v1/robot/motor/move` - Move robot (direction, speed)
- `POST /api/v1/robot/motor/stop` - Stop all motors
- `POST /api/v1/robot/servo/position` - Set servo angle
- `POST /api/v1/robot/emergency/stop` - Emergency stop

### **Sensors**
- `GET /api/v1/robot/sensors` - Get all sensor data
- `GET /api/v1/system/stats` - System status and health

### **Communication**
- `POST /api/v1/robot/chat/message` - Send chat message
- `POST /api/v1/robot/voice/toggle` - Toggle voice control

### **WiFi**
- `GET /api/v1/wifi/status` - Current WiFi status
- `GET /api/v1/wifi/scan` - Scan available networks

## Memory Usage

**Total Frontend Size: ~45KB** (well under 1MB constraint)
- HTML: ~8KB
- CSS: ~12KB
- JavaScript: ~25KB (4 modules)
- PWA Manifest: ~500B

## Browser Support

- **Modern browsers** (Chrome, Firefox, Safari, Edge)
- **Mobile browsers** (iOS Safari, Android Chrome)
- **WebSocket support** required
- **ES6+ features** used

## Security

- **Authentication required** for robot control APIs
- **Session management** with secure tokens  
- **CORS protection** for cross-origin requests
- **Rate limiting** on API endpoints

## Performance

- **60fps** sensor chart updates
- **10Hz** motor command throttling
- **Efficient WebSocket** communication
- **Responsive design** optimized for all screen sizes
- **Progressive loading** for optimal startup time

This interface provides a production-ready, beautiful, and highly functional control system for your ESP32 Cozmo robot that's easy for AI agents to maintain and extend.
