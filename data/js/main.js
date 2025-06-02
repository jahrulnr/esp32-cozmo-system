// WebSocket connection
let websocket;
const wsProtocol = location.protocol === 'https:' ? 'wss://' : 'ws://';
const wsURL = wsProtocol + window.location.hostname + '/ws';
const cameraFeed = document.getElementById('camera-feed');
const connectionStatus = document.getElementById('connection-status');

// Movement control buttons
const forwardBtn = document.getElementById('forward-btn');
const backwardBtn = document.getElementById('backward-btn');
const leftBtn = document.getElementById('left-btn');
const rightBtn = document.getElementById('right-btn');
const stopBtn = document.getElementById('stop-btn');

// Camera control sliders
const panSlider = document.getElementById('pan-slider');
const tiltSlider = document.getElementById('tilt-slider');

// Sensor data display elements
const gyroX = document.getElementById('gyro-x');
const gyroY = document.getElementById('gyro-y');
const gyroZ = document.getElementById('gyro-z');

// Initialize WebSocket connection
function initWebSocket() {
    websocket = new WebSocket(wsURL);
    websocket.binaryType = 'arraybuffer';
    
    websocket.onopen = onWebSocketOpen;
    websocket.onclose = onWebSocketClose;
    websocket.onmessage = onWebSocketMessage;
    websocket.onerror = onWebSocketError;
}

// WebSocket event handlers
function onWebSocketOpen(event) {
    console.log('WebSocket connection established');
    connectionStatus.textContent = 'Connected';
    connectionStatus.className = 'status-connected';
    
    // Request initial gyro data when connection is established
    sendJsonMessage('gyro_request', {});
}

function onWebSocketClose(event) {
    console.log('WebSocket connection closed');
    connectionStatus.textContent = 'Disconnected';
    connectionStatus.className = 'status-disconnected';
    
    // Try to reconnect after a delay
    setTimeout(initWebSocket, 2000);
}

function onWebSocketMessage(event) {
    // Handle binary data (camera feed)
    if (event.data instanceof ArrayBuffer) {
        handleCameraFrame(event.data);
    } 
    // Handle text data (JSON messages)
    else {
        try {
            const message = JSON.parse(event.data);
            
            // Check if message follows the standard format
            if (!message.type || message.data === undefined) {
                console.warn('Received message does not follow the standard format:', message);
                return;
            }
            
            // Process message based on type
            switch (message.type) {
                case 'sensor_data':
                    updateSensorData(message.data);
                    break;
                case 'gyro_data': // For backwards compatibility
                    updateGyroData(message.data);
                    break;
                case 'camera_frame':
                    // Handle base64 encoded camera frame if it's in JSON
                    if (message.data.format === 'base64') {
                        handleBase64CameraFrame(message.data.data);
                    }
                    break;
                case 'system_status':
                    updateSystemStatus(message.data);
                    break;
                case 'error':
                    handleError(message.data);
                    break;
                case 'ok':
                    console.log('Command successful:', message.data.message);
                    break;
                default:
                    console.log('Unknown message type:', message.type, message.data);
            }
        } catch (error) {
            // console.error('Error parsing WebSocket message:', error, event.data);
        }
    }
}

function onWebSocketError(event) {
    console.error('WebSocket error:', event);
}

// Handle camera frame data
function handleCameraFrame(arrayBuffer) {
    // Convert ArrayBuffer to Blob
    const blob = new Blob([arrayBuffer], { type: 'image/jpeg' });
    
    // Create URL for the Blob and update image source
    const url = URL.createObjectURL(blob);
    cameraFeed.src = url;
    
    // Clean up the URL after the image loads
    cameraFeed.onload = () => URL.revokeObjectURL(url);
}

// Handle base64 encoded camera frame
function handleBase64CameraFrame(base64Data) {
    cameraFeed.src = `data:image/jpeg;base64,${base64Data}`;
}

// Update gyroscope data display (for backward compatibility)
function updateGyroData(data) {
    if (data.x !== undefined && data.y !== undefined && data.z !== undefined) {
        gyroX.textContent = data.x;
        gyroY.textContent = data.y;
        gyroZ.textContent = data.z;
    }
}

// Update sensor data with the new format
function updateSensorData(data) {
    // Update gyroscope data
    if (data.gyro) {
        gyroX.textContent = data.gyro.x;
        gyroY.textContent = data.gyro.y;
        gyroZ.textContent = data.gyro.z;
    }
    
    // Update accelerometer data
    if (data.accel) {
        const accelX = document.getElementById('accel-x');
        const accelY = document.getElementById('accel-y');
        const accelZ = document.getElementById('accel-z');
        const accelMag = document.getElementById('accel-magnitude');
        const magnitudePointer = document.getElementById('magnitude-pointer');
        
        if (accelX) accelX.textContent = data.accel.x;
        if (accelY) accelY.textContent = data.accel.y;
        if (accelZ) accelZ.textContent = data.accel.z;
        if (accelMag) accelMag.textContent = data.accel.magnitude;
        
        // Update magnitude indicator if present
        if (magnitudePointer) {
            const magValue = parseFloat(data.accel.magnitude);
            // Scale the pointer position based on magnitude (0.5g to 1.5g range mapped to 0-100%)
            const percentage = Math.min(Math.max((magValue - 0.5) * 100, 0), 100);
            magnitudePointer.style.left = `${percentage}%`;
        }
    }
}

// Update system status display
function updateSystemStatus(data) {
    // Implementation depends on what system status fields are available
    console.log('System status:', data);
    // Example: if you have UI elements for battery, temperature, etc.
    // batteryLevel.textContent = data.battery + '%';
    // temperature.textContent = data.temperature + '°C';
}

// Handle error messages
function handleError(data) {
    console.error(`Error ${data.code}: ${data.message}`);
    // You might want to display this in the UI
    // errorDisplay.textContent = `Error: ${data.message}`;
}

// Send standardized JSON message
function sendJsonMessage(type, data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        const message = JSON.stringify({
            type: type,
            data: data
        });
        websocket.send(message);
    } else {
        console.warn('WebSocket not ready, message not sent:', type, data);
    }
}

// Movement command functions
function moveForward() {
    sendJsonMessage('motor_command', {
        left: 0.75,
        right: 0.75,
        duration: 0 // 0 means continue until stopped
    });
}

function moveBackward() {
    sendJsonMessage('motor_command', {
        left: -0.75,
        right: -0.75,
        duration: 0
    });
}

function turnLeft() {
    sendJsonMessage('motor_command', {
        left: -0.5,
        right: 0.5,
        duration: 0
    });
}

function turnRight() {
    sendJsonMessage('motor_command', {
        left: 0.5,
        right: -0.5,
        duration: 0
    });
}

function stopMoving() {
    sendJsonMessage('motor_command', {
        left: 0,
        right: 0,
        duration: 0
    });
}

// Camera position command functions
function updatePan() {
    const angle = parseInt(panSlider.value, 10);
    sendJsonMessage('head_command', {
        pan: angle
    });
}

function updateTilt() {
    const angle = parseInt(tiltSlider.value, 10);
    sendJsonMessage('head_command', {
        tilt: angle
    });
}

function pickUp() {
    sendJsonMessage('action_command', {
        action: 'pickup'
    });
}

function drop() {
    sendJsonMessage('action_command', {
        action: 'drop'
    });
}

// Event listeners for controls
forwardBtn.addEventListener('mousedown', moveForward);
forwardBtn.addEventListener('mouseup', stopMoving);
forwardBtn.addEventListener('mouseleave', stopMoving);

backwardBtn.addEventListener('mousedown', moveBackward);
backwardBtn.addEventListener('mouseup', stopMoving);
backwardBtn.addEventListener('mouseleave', stopMoving);

leftBtn.addEventListener('mousedown', turnLeft);
leftBtn.addEventListener('mouseup', stopMoving);
leftBtn.addEventListener('mouseleave', stopMoving);

rightBtn.addEventListener('mousedown', turnRight);
rightBtn.addEventListener('mouseup', stopMoving);
rightBtn.addEventListener('mouseleave', stopMoving);

stopBtn.addEventListener('click', stopMoving);

panSlider.addEventListener('change', updatePan);
tiltSlider.addEventListener('change', updateTilt);

// Touch event support for mobile devices
forwardBtn.addEventListener('touchstart', moveForward);
forwardBtn.addEventListener('touchend', stopMoving);

backwardBtn.addEventListener('touchstart', moveBackward);
backwardBtn.addEventListener('touchend', stopMoving);

leftBtn.addEventListener('touchstart', turnLeft);
leftBtn.addEventListener('touchend', stopMoving);

rightBtn.addEventListener('touchstart', turnRight);
rightBtn.addEventListener('touchend', stopMoving);

// Initialize WebSocket connection when page loads
window.addEventListener('load', initWebSocket);

// Ping the server periodically to keep the connection alive
setInterval(() => {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        sendJsonMessage('ping', { timestamp: Date.now() });
    }
}, 30000);
