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

// Arm control slider and buttons
const armSlider = document.getElementById('arm-slider');
const pickupBtn = document.getElementById('pickup-btn');
const dropBtn = document.getElementById('drop-btn');

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
    // Handle text data (JSON sensor data)
    else {
        try {
            const data = JSON.parse(event.data);
            updateSensorData(data);
        } catch (error) {
            console.error('Error parsing WebSocket message:', error);
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

// Update sensor data display
function updateSensorData(data) {
    if (data.gyro) {
        gyroX.textContent = data.gyro.x.toFixed(2);
        gyroY.textContent = data.gyro.y.toFixed(2);
        gyroZ.textContent = data.gyro.z.toFixed(2);
    }
}

// Send commands to the robot
function sendCommand(command) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(command);
    } else {
        console.warn('WebSocket not ready, command not sent:', command);
    }
}

// Movement command functions
function moveForward() {
    sendCommand('move:forward');
}

function moveBackward() {
    sendCommand('move:backward');
}

function turnLeft() {
    sendCommand('move:left');
}

function turnRight() {
    sendCommand('move:right');
}

function stopMoving() {
    sendCommand('move:stop');
}

// Camera position command functions
function updatePan() {
    const angle = panSlider.value;
    sendCommand(`servo:pan:${angle}`);
}

function updateTilt() {
    const angle = tiltSlider.value;
    sendCommand(`servo:tilt:${angle}`);
}

// Arm control functions
function updateArm() {
    const angle = armSlider.value;
    sendCommand(`servo:arm:${angle}`);
}

function pickUp() {
    sendCommand('action:pickup');
}

function drop() {
    sendCommand('action:drop');
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
armSlider.addEventListener('change', updateArm);

pickupBtn.addEventListener('click', pickUp);
dropBtn.addEventListener('click', drop);

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
        sendCommand('ping');
    }
}, 30000);
