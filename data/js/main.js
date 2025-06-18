// Main JavaScript for IoT Dashboard

// DOM Elements
const loginPage = document.getElementById('login-page');
const mainApp = document.getElementById('main-app');
const consoleOutput = document.getElementById('console-output');
const recentLogs = document.getElementById('recent-logs');
const sidebarToggle = document.getElementById('sidebar-toggle');
const sidebar = document.getElementById('sidebar');
const menuItems = document.querySelectorAll('.menu-item');
const contentSections = document.querySelectorAll('.content-section');
const loginForm = document.getElementById('login-form');
const chatForm = document.getElementById('chat-form');
const chatMessages = document.getElementById('chat-messages');
const wifiList = document.getElementById('wifi-list');
const fileList = document.getElementById('file-list');
const breadcrumb = document.getElementById('breadcrumb');
const printTime = document.querySelectorAll('.print-time');
const servoHeadSlider = document.getElementById('servo-head-slider');
const servoHandSlider = document.getElementById('servo-hand-slider');
    
// Variables for frame processing
let frameHeader = null;
let frameQueue = [];
const MAX_FRAME_QUEUE = 2; // Limit queued frames to avoid memory issues
let processingFrame = false;

document.addEventListener('DOMContentLoaded', () => {
    // Configuration
    const wsUri = `ws://${window.location.hostname}/ws`;
    let websocket = null;
    let isLoggedIn = false;
    let reconnectInterval = null;
    let connected = false;
    const reconnectTime = 3000; // 3 seconds
    const AUTH_TOKEN_KEY = 'cozmo_auth_token'; // Key for storing auth token in localStorage
    
    // Variables for joystick controls
    let joystickInitialized = false;
    let lastSendTimeServo = 0;
    let lastSendTimeMotor = 0;
    const sendThrottle = 100; // Send position updates every 100ms

    printTime.forEach((el, key) => {
        const now = new Date();
        el.textContent = now.getHours() +":"+ now.getMinutes() +":"+ now.getSeconds();
    });
    
    // Connect WebSocket
    function connectWebSocket() {
        
        websocket = new WebSocket(wsUri);

        websocket.onopen = (evt) => {
            
            clearInterval(reconnectInterval);
            logToConsole('WebSocket connection established', 'success');
            
            if (isLoggedIn) {
                // If we're already logged in from a saved token, send auth message to server
                const savedAuth = localStorage.getItem(AUTH_TOKEN_KEY);
                if (savedAuth) {
                    const authData = JSON.parse(savedAuth);
                    
                    // If you have a token-based system, use the token instead
                    sendCommand('login', {
                        username: authData.username,
                        password: 'AUTO_LOGIN_TOKEN',
                        token: authData.token
                    });
                }
                
                // Fetch initial data
                fetchWifiNetworks();
                fetchFiles('/');
                fetchStorageInfo();
                startSensorUpdates();
                connected = true;
            }
        };

        websocket.onclose = (evt) => {
            connected = false;
            
            logToConsole('WebSocket connection lost. Attempting to reconnect...', 'warning');
            
            // Try to reconnect
            if (!reconnectInterval) {
                reconnectInterval = setInterval(connectWebSocket, reconnectTime);
            }
        };

        websocket.onmessage = (evt) => {
            handleWebSocketMessage(evt);
        };

        websocket.onerror = (evt) => {
            connected = false;
            console.error('WebSocket Error:', evt);
            logToConsole('WebSocket error occurred', 'error');
            
            // Close the connection if it's still active before reconnecting
            if (websocket && websocket.readyState !== WebSocket.CLOSED) {
                try {
                    websocket.close();
                } catch (err) {
                    console.error('Error closing websocket:', err);
                }
            }
        };
    }

    // Check if WebSocket connection is established
    function checkWebSocketConnection() {
        if (websocket && websocket.readyState === WebSocket.OPEN) {
            return true;
        } else {
            logToConsole('WebSocket not connected', 'error');
            return false;
        }
    }

    // Send Command to server using the new DTO contract format (v1.0)
    function sendJsonMessage(type, data = {}) {
        if (checkWebSocketConnection()) {
            const message = JSON.stringify({
                version: "1.0",
                type: type,
                data: data
            });
            websocket.send(message);
            return true;
        }
        return false;
    }

    // Legacy function for backward compatibility
    function sendCommand(command, data = {}) {
        sendJsonMessage(command, data);
    }

    // Handle WebSocket messages
    function handleWebSocketMessage(evt) {
        // Handle binary data (camera frames)
        if (evt.data instanceof ArrayBuffer || evt.data instanceof Blob) {
            console.log("Binary received", evt)
            handleBinaryMessage(evt.data);
            return;
        }
        
        // Handle text data (JSON messages)
        try {
            const msg = JSON.parse(evt.data);
            

            // Check for version field to determine format
            if (msg.version === "1.0") {
                // New DTO contract format
                
            }

            // Handle DTO format messages (works with both old and new format)
            if (msg.type === 'login_response') {
                handleLoginResponse(msg);
            } else if (msg.type === 'system_status') {
                updateStatus(msg.data);
            } else if (msg.type === 'sensor_data') {
                updateSensors(msg.data);
            } else if (msg.type === 'camera_frame') {
                updateCameraFrame(msg.data);
            } else if (msg.type === 'wifi_list') {
                populateWifiList(msg.data);
            } else if (msg.type === 'wifi_connection') {
                handleWifiConnection(msg.data);
            } else if (msg.type === 'list_files') {
                populateFileList(msg.data);
            } else if (msg.type === 'file_operation') {
                handleFileOperation(msg.data);
            } else if (msg.type === 'chat_message') {
                addChatMessage(msg.data, false);
            } else if (msg.type === 'log_message') {
                logToConsole(msg.data.message, msg.data.level);
            } else if (msg.type === 'batch_log_messages') {
                if (Array.isArray(msg.data.logs)) {
                    // Process batch of log messages
                    msg.data.logs.forEach(log => {
                        logToConsole(log.message, log.level);
                    });
                }
            } else if (msg.type === 'error') {
                handleError(msg);
            } else if (msg.type === 'ok') {
                handleSuccess(msg);
            } else if (msg.type === 'motor_status') {
                updateMotorStatus(msg.data);
            } else if (msg.type === 'storage_info') {
                updateStorageInfo(msg.data);
            } else if (msg.type === 'file_content') {
                displayFileContent(msg.data);
            } else if (msg.type === 'task_list') {
                handleTaskList(msg.data);
            } else if (msg.type === 'task_control_response') {
                handleTaskControlResponse(msg.data);
            } else if (msg.type === 'notification') {
                handleNotification(msg.data);
            } else if (msg.type === 'wifi_config') {
                handleWifiConfig(msg.data);
            } else if (msg.type === 'wifi_config_update') {
                handleWifiConfigUpdate(msg.data);
            } else if (msg.type === 'camera_frame_header') {
                handleCameraFrameHeader(msg.data);
            } else if (msg.type === 'logout_response') {
                handleLogoutResponse(msg.data);
            } else if (msg.type === 'automation_status') {
                updateAutomationStatus(msg.data);
            } else {
                console.log('Unknown message type:', msg.type, msg.data);
            }
        } catch (e) {
            console.error('Error parsing WebSocket message:', e);
            logToConsole(`Error parsing message: ${e.message}`, 'error');
        }
    }

    // Handle binary messages (camera frames)
    function handleBinaryMessage(data) {
        // If we don't have frame header metadata, try to detect JPEG from magic bytes
        if (!frameHeader) {
            console.log("Received binary data without header, size:", (data.size || data.byteLength));
            
            // Try to detect if this is a JPEG by checking the first few bytes
            let isJpeg = false;
            
            if (data instanceof Blob) {
                // Sample a small chunk to check header bytes
                data.slice(0, 3).arrayBuffer().then(buffer => {
                    const header = new Uint8Array(buffer);
                    // Check JPEG magic bytes (FF D8 FF)
                    if (header[0] === 0xFF && header[1] === 0xD8 && header[2] === 0xFF) {
                        console.log("Detected JPEG from magic bytes");
                        const blob = new Blob([data], { type: 'image/jpeg' });
                        updateImageSources(URL.createObjectURL(blob));
                    } else {
                        // Unknown format, try as generic binary
                        console.log("Unknown binary format, first bytes:", header);
                        updateImageSources(URL.createObjectURL(data));
                    }
                });
            } else if (data instanceof ArrayBuffer || ArrayBuffer.isView(data)) {
                // For ArrayBuffer or views like Uint8Array
                const header = new Uint8Array(data instanceof ArrayBuffer ? data : data.buffer, 0, 3);
                if (header[0] === 0xFF && header[1] === 0xD8 && header[2] === 0xFF) {
                    console.log("Detected JPEG from magic bytes");
                    const blob = new Blob([data], { type: 'image/jpeg' });
                    updateImageSources(URL.createObjectURL(blob));
                } else {
                    // Unknown format, try as generic binary
                    console.log("Unknown binary format, first bytes:", Array.from(header).map(b => b.toString(16)));
                    updateImageSources(URL.createObjectURL(data));
                }
            } else {
                // Fallback for unknown data type
                updateImageSources(URL.createObjectURL(data));
            }
            
            return;
        }
        
        function updateImageSources(url) {
            const cameraFeed = document.getElementById('camera-feed');
            const cameraFeedMain = document.getElementById('camera-feed-main');
            
            if (cameraFeed) {
                cameraFeed.onload = () => URL.revokeObjectURL(cameraFeed.src);
                cameraFeed.src = url;
            }
            
            if (cameraFeedMain) {
                cameraFeedMain.onload = () => URL.revokeObjectURL(cameraFeedMain.src);
                cameraFeedMain.src = url;
            }
        }
        
        // Add frame to queue
        frameQueue.push({
            data: data,
            header: frameHeader
        });
        
        // Limit queue size
        while (frameQueue.length > MAX_FRAME_QUEUE) {
            const oldFrame = frameQueue.shift();
            if (oldFrame.url) {
                URL.revokeObjectURL(oldFrame.url);
            }
        }
        
        // Clear the frame header to avoid reusing it for a wrong frame
        frameHeader = null;
        
        // Process frame if not already processing
        if (!processingFrame) {
            processNextFrame();
        }
    }
    
    // Process next frame in queue
    function processNextFrame() {
        if (frameQueue.length === 0) {
            processingFrame = false;
            return;
        }
        
        processingFrame = true;
        const frame = frameQueue.shift();
        
        // Create a blob from the binary data with the correct MIME type
        let mimeType = 'application/octet-stream';
        
        // If we have frame header with format info, use the appropriate MIME type
        if (frame.header && frame.header.format) {
            if (frame.header.format.toLowerCase() === 'jpeg') {
                mimeType = 'image/jpeg';
            } else if (frame.header.format.toLowerCase() === 'png') {
                mimeType = 'image/png';
            }
        }
        
        // Create the blob with the correct MIME type
        const blob = frame.data instanceof Blob ? 
            new Blob([frame.data], { type: mimeType }) : 
            new Blob([frame.data], { type: mimeType });
            
        console.log("Creating image URL with mime type:", mimeType, "size:", (frame.data.size || frame.data.byteLength));
        frame.url = URL.createObjectURL(blob);
        
        // Update the camera feed with the new image
        const cameraFeed = document.getElementById('camera-feed');
        const cameraFeedMain = document.getElementById('camera-feed-main');
        
        // Function to handle image load completion
        const onImageLoad = function() {
            URL.revokeObjectURL(this.src);
            
            // Process next frame with a small delay
            setTimeout(() => {
                processNextFrame();
            }, 10);
        };
        
        if (cameraFeed) {
            cameraFeed.onload = onImageLoad;
            cameraFeed.src = frame.url;
        }
        
        if (cameraFeedMain) {
            cameraFeedMain.onload = onImageLoad;
            cameraFeedMain.src = frame.url;
        }
        
        // If neither element exists, move on to next frame
        if (!cameraFeed && !cameraFeedMain) {
            URL.revokeObjectURL(frame.url);
            processNextFrame();
        }
    }

    // Login functions
    function handleLoginResponse(response) {
        if (response.data && response.data.success) {
            isLoggedIn = true;
            
            // Store authentication token in localStorage
            const authToken = {
                timestamp: Date.now(),
                username: document.getElementById('username')?.value || 'admin',
                token: response.data.token || 'authenticated' // Use token from response or a placeholder
            };
            localStorage.setItem(AUTH_TOKEN_KEY, JSON.stringify(authToken));
            
            // Show main app, hide login
            loginPage.classList.add('hidden');
            mainApp.classList.remove('hidden');
            logToConsole('Login successful', 'success');
            
            // Initialize app data after login
            sendCommand('get_status');
            fetchWifiNetworks();
            fetchFiles('/');
            fetchStorageInfo();
            startSensorUpdates();
        } else {
            document.getElementById('login-error').textContent = 
                response.data && response.data.message ? response.data.message : 'Login failed';
            logToConsole('Login failed: Invalid credentials', 'error');
        }
    }

    // Error handling
    function handleError(error) {
        logToConsole(`Error: ${error.message || 'Unknown error'}`, 'error');
    }

    // Console logging
    function logToConsole(message, level = 'info') {
        const line = document.createElement('div');
        line.className = `console-line console-${level}`;
        
        const timestamp = new Date().toTimeString().split(' ')[0];
        line.textContent = `[${timestamp}] ${message}`;
        
        recentLogs.innerHTML += line.outerHTML;
        consoleOutput.appendChild(line);
        recentLogs.scrollTop = recentLogs.scrollHeight;
        consoleOutput.scrollTop = consoleOutput.scrollHeight;
    }

    // Navigation
    function showContent(id) {
        contentSections.forEach(section => {
            section.classList.add('hidden');
        });
        document.getElementById(id).classList.remove('hidden');
        
        menuItems.forEach(item => {
            item.classList.remove('active');
        });
        document.querySelector(`[data-target="${id}"]`).classList.add('active');
        
        // Load specific data based on the section
        if (id === 'files-section') {
            fetchFiles(currentPath || '/');
            fetchStorageInfo();
        }
    }

    // Joystick controls

    function initJoysticks() {
        if (joystickInitialized) return;
        joystickInitialized = true;
        
        if (servoHeadSlider) {
            servoHeadSlider.addEventListener('input', function() {
                const value = this.value;
                document.getElementById('servo-x').textContent = value;
                
                // Send to server with throttling
                const currentTime = Date.now();
                if (currentTime - lastSendTimeServo >= sendThrottle) {
                    sendServoPosition('head', value);
                    lastSendTimeServo = currentTime;
                }
            });
        }
        
        if (servoHandSlider) {
            servoHandSlider.addEventListener('input', function() {
                const value = this.value;
                document.getElementById('servo-y').textContent = value;
                
                // Send to server with throttling
                const currentTime = Date.now();
                if (currentTime - lastSendTimeServo >= sendThrottle) {
                    sendServoPosition('hand', value);
                    lastSendTimeServo = currentTime;
                }
            });
        }

        // Initialize the motor joystick using the bobboteck library
        motorJoystickObj = new JoyStick('motor-joystick-container', {
            title: 'motor-joystick',
            width: 200,
            height: 200,
            internalFillColor: '#2ecc71',
            internalStrokeColor: '#27ae60',
            externalStrokeColor: '#2c3e50',
            autoReturnToCenter: true
        }, motorJoystickCallback);
    }

    // Function for sending servo position
    function sendServoPosition(type, value) {
        // Convert to number if needed
        value = parseInt(value, 10);
        
        // Validate input
        if (isNaN(value)) {
            console.error(`Invalid servo position value: ${value}`);
            return;
        }
        
        
        
        sendCommand('servo_update', {
            type: type,
            position: value
        });
    }

    // Callback function for the motor joystick
    function motorJoystickCallback(stickData) {
        // Update display values on the UI
        updateJoystickDisplayValues('motor', stickData.x, stickData.y);
        
        // Send to server with throttling
        const currentTime = Date.now();
        if (currentTime - lastSendTimeMotor >= sendThrottle) {
            sendJoystickPosition('motor', stickData.x, stickData.y);
            lastSendTimeMotor = currentTime;
        }
    }
    
    function sendJoystickPosition(type, x, y) {
        // Convert string values to numbers if needed (joy.js library returns strings)
        x = parseFloat(x);
        y = parseFloat(y);
        
        // First validate inputs
        if (isNaN(x) || isNaN(y)) {
            console.error(`Invalid joystick position values: X=${x}, Y=${y}`);
            return;
        }
        
        // Apply a small threshold to avoid sending tiny values that are effectively zero
        const threshold = 0.5;
        const xVal = Math.abs(x) < threshold ? 0 : x;
        const yVal = Math.abs(y) < threshold ? 0 : y;
        
        // Round to integers to reduce noise in the values
        const validX = Math.round(xVal);
        const validY = Math.round(yVal);
        
        
        
        sendCommand('joystick_update', {
            type: type,
            x: validX,
            y: validY
        });
    }
    
    function updateJoystickDisplayValues(type, x, y) {
        // First validate inputs
        if (isNaN(x) || isNaN(y)) {
            console.error(`Invalid joystick display values: X=${x}, Y=${y}`);
            return;
        }

        if (typeof x !== 'number' || typeof y !== 'number') {
            x = 1.0 * x;
            y = 1.0 * y;
        }
        
        // Apply a small threshold to avoid displaying tiny values that are effectively zero
        const threshold = 0.5;
        const xVal = Math.abs(x) < threshold ? 0 : x;
        const yVal = Math.abs(y) < threshold ? 0 : y;
        
        // Round to integers to reduce noise in the display
        const validX = Math.round(xVal);
        const validY = Math.round(yVal);
        
        
        
        try {
            if (type === 'servo') {
                // Update servo joystick display
                const servoX = document.getElementById('servo-x');
                const servoY = document.getElementById('servo-y');
                
                if (servoX) servoX.textContent = validX;
                if (servoY) servoY.textContent = validY;
            } else if (type === 'motor') {
                // Update motor joystick display
                const motorX = document.getElementById('motor-x');
                const motorY = document.getElementById('motor-y');
                
                if (motorX) motorX.textContent = validX;
                if (motorY) motorY.textContent = validY;
            }
        } catch (error) {
            console.error(`Error updating ${type} joystick display:`, error);
        }
    }
    
    function updateCameraFrame(data) {
        // Store frame metadata for the next binary message (used for camera_frame messages)
        frameHeader = data;
    }
    
    function toggleCamera() {
        const cameraBtn = document.getElementById('camera-toggle');
        const isActive = cameraBtn.getAttribute('data-active') === 'true';
        
        if (isActive) {
            sendCommand('camera_command', { action: 'stop' });
            cameraBtn.setAttribute('data-active', 'false');
            cameraBtn.textContent = 'Start Camera';
        } else {
            sendCommand('camera_command', { action: 'start' });
            cameraBtn.setAttribute('data-active', 'true');
            cameraBtn.textContent = 'Stop Camera';
        }
    }

    // Sensor updates
    function startSensorUpdates() {
        // nothing
    }
    
    function updateSensors(data) {
        if (data.gyro) {
            const gyroX = document.getElementById('gyro-x');
            const gyroY = document.getElementById('gyro-y');
            const gyroZ = document.getElementById('gyro-z');
            
            if (gyroX) gyroX.textContent = toFixed(data.gyro.x, 2);
            if (gyroY) gyroY.textContent = toFixed(data.gyro.y, 2);
            if (gyroZ) gyroZ.textContent = toFixed(data.gyro.z, 2);
            
            updateGyroVisualization(data.gyro);
        }
        
        if (data.accel) {
            const accelX = document.getElementById('accel-x');
            const accelY = document.getElementById('accel-y');
            const accelZ = document.getElementById('accel-z');
            
            if (accelX) accelX.textContent = toFixed(data.accel.x, 2);
            if (accelY) accelY.textContent = toFixed(data.accel.y, 2);
            if (accelZ) accelZ.textContent = toFixed(data.accel.z, 2);
            
            updateAccelVisualization(data.accel);
        }
        
        // Update distance sensor data if available
        if (data.distance) {
            const distanceValue = document.getElementById('distance-value');
            const distanceValueSmall = document.getElementById('distance-value-small');
            const obstacleStatus = document.getElementById('obstacle-status');
            const obstacleStatusSmall = document.getElementById('obstacle-status-small');
            
            // Update main distance value
            if (distanceValue) {
                // Check if the distance reading is valid
                if (data.distance.valid) {
                    const formattedValue = toFixed(data.distance.value, 1);
                    distanceValue.textContent = formattedValue;
                    if (distanceValueSmall) distanceValueSmall.textContent = formattedValue;
                    updateRadarVisualization(data.distance.value, data.distance.obstacle);
                } else {
                    distanceValue.textContent = "N/A";
                    if (distanceValueSmall) distanceValueSmall.textContent = "N/A";
                    updateRadarVisualization(-1, false); // Invalid reading
                }
            }
            
            // Update obstacle status
            if (obstacleStatus) {
                const statusText = data.distance.obstacle ? "Obstacle Detected!" : "No Obstacle";
                const bgColor = data.distance.obstacle ? 
                    "rgba(248, 81, 73, 0.2)" : "rgba(86, 211, 100, 0.2)";
                const textColor = data.distance.obstacle ? 
                    "var(--color-danger)" : "var(--color-success)";
                
                obstacleStatus.textContent = statusText;
                obstacleStatus.style.backgroundColor = bgColor;
                obstacleStatus.style.color = textColor;
                
                // Update small obstacle status if available
                if (obstacleStatusSmall) {
                    obstacleStatusSmall.textContent = statusText;
                }
            }
        }

        if (data.temperature) {
            const temperatureData = data.temperature.value + data.temperature.unit;
            const temperatureContainer = document.getElementById("temperature-status")
            if(temperatureContainer)
                temperatureContainer.textContent = temperatureData;
            const temperatureSensor = document.getElementById("temperature")
            if(temperatureSensor)
                temperatureSensor.textContent = temperatureData;
        }

        if (data.cliff) {
            const cliffSensor = document.getElementById("cliff-detector")
            const cliffContainer = document.createElement("span");
            cliffContainer.append("Left: " + (data.cliff.left ? "true":"false"));
            cliffContainer.append(document.createElement("br"))
            cliffContainer.append("Right: " + (data.cliff.right ? "true":"false"));
            if(cliffSensor)
                cliffSensor.innerHTML = cliffContainer.outerHTML;
        }
        
        // Only update joystick displays from sensor data if we have a specific flag
        // This prevents sensor updates from overriding manual joystick positioning
        if (data.updateJoysticks === true) {
            // Update motor position displays if available
            if (data.motor) {
                updateJoystickDisplayValues('motor', data.motor.x, data.motor.y);
            }
        }

        if (data.servo) {
            if (data.servo.head) {
                document.getElementById('servo-x').textContent = data.servo.head;
                servoHeadSlider.value = data.servo.head;
            }
            if (data.servo.hand) {
                document.getElementById('servo-y').textContent = data.servo.hand;
                servoHandSlider.value = data.servo.hand;
            }
        }
    }
    
    // Update the radar visualization with distance data
    function updateRadarVisualization(distance, isObstacle) {
        const radarPin = document.getElementById('radar-pin');
        if (!radarPin) return;
        
        // Update obstacle status elements if they exist
        const obstacleStatus = document.getElementById('obstacle-status');
        const obstacleStatusSmall = document.getElementById('obstacle-status-small');
        
        if (obstacleStatus) {
            if (isObstacle) {
                obstacleStatus.classList.add('obstacle-detected');
            } else {
                obstacleStatus.classList.remove('obstacle-detected');
            }
        }
        
        // Check if we have a valid distance measurement
        if (distance < 0) {
            // Invalid distance reading, hide the pin
            radarPin.classList.remove('visible');
            radarPin.classList.remove('obstacle');
            radarPin.classList.remove('no-obstacle');
            return;
        }
        
        // Show the pin and update its position based on the distance
        radarPin.classList.add('visible');
        
        // Update the pin style based on obstacle detection
        if (isObstacle) {
            radarPin.classList.add('obstacle');
            radarPin.classList.remove('no-obstacle');
        } else {
            radarPin.classList.add('no-obstacle');
            radarPin.classList.remove('obstacle');
        }
        
        // Calculate the position of the pin on the radar
        // Max distance is 400cm, but we'll scale it to fit in our radar (which represents a 100% range)
        const maxDistance = 400; // Maximum distance in cm
        const normalizedDistance = Math.min(distance / maxDistance, 1.0);
        
        // Convert to radar coordinates (from center)
        // Radar is reversed - closer objects are further from center
        const scale = 1 - normalizedDistance;
        
        // Get the sweep element for manual positioning
        const sweepElement = document.querySelector('.radar-sweep');
        
        // Use both the current gyro Z rotation and a time-based animation
        const animationPosition = (Date.now() % 2000) / 2000;
        
        // Base animation angle (-15 to +15 degrees)
        let baseAngle = 0;
        if (animationPosition < 0.5) {
            baseAngle = -15 + (animationPosition * 2) * 30;
        } else {
            baseAngle = 15 - ((animationPosition - 0.5) * 2) * 30;
        }
        
        // Combine base animation with gyro Z value for responsive radar
        // Scale down gyro value to make it more subtle (gyro data can be quite large)
        let gyroInfluence = targetGyroData.z / 30;
        // Limit the influence to a reasonable range (-20 to +20 degrees)
        gyroInfluence = Math.min(20, Math.max(-20, gyroInfluence));
        
        // Update the currentRadarAngle global variable to track the current position
        currentRadarAngle = gyroInfluence;
        
        // Calculate final sweep angle by combining base animation and gyro
        // Weight the gyro influence more heavily (60% gyro, 40% animation)
        const sweepAngle = (baseAngle * 0.4) + (gyroInfluence * 1.5);
        
        // Optional: Update the radar-sweep element animation dynamically
        // if (sweepElement) {
        //     sweepElement.style.transform = `rotate(${sweepAngle}deg)`;
        //     sweepElement.style.animation = "none"; // Disable default CSS animation
        // }
        
        // Convert degrees to radians for the math calculations
        const angle = sweepAngle * (Math.PI / 180);
        
        // Calculate x and y positions on the radar (centered at 50%, 50%)
        const radius = scale * 50; // Scale to 0-50% of radar size
        
        // Add extra movement to the pin based on gyro data for a more natural look
        const gyroFactor = 1.3; // Increase the gyro influence on pin position
        const pinAngle = angle + (currentRadarAngle * (Math.PI / 180) * gyroFactor);
        
        const x = 50 + Math.sin(pinAngle) * radius; // Sine for x because 0 degrees is up
        const y = 50 - Math.cos(pinAngle) * radius; // Cosine (negative) for y because 0 degrees is up
        
        // Apply smooth transition to the pin position
        radarPin.style.left = x + '%';
        radarPin.style.top = y + '%';
        
        // Also update the distance value displays
        const distanceValue = document.getElementById('distance-value');
        const distanceValueSmall = document.getElementById('distance-value-small');
        
        if (distanceValue) {
            const formattedValue = distance.toFixed(1);
            distanceValue.textContent = formattedValue;
            if (distanceValueSmall) {
                distanceValueSmall.textContent = formattedValue;
            }
        }
    }
    
    // Add missing handler functions
    function handleSuccess(msg) {
        logToConsole(`Success: ${msg.data.message || 'Operation completed'}`, 'success');
    }
    
    function updateMotorStatus(data) {
        logToConsole(`Motor Status - Left: ${data.left}, Right: ${data.right}`, 'info');
        
        // Update motor status display if elements exist
        const leftStatus = document.getElementById('motor-left-status');
        const rightStatus = document.getElementById('motor-right-status');
        const motorSpeed = document.getElementById('motor-speed');
        const motorDirection = document.getElementById('motor-direction');
        
        if (leftStatus) leftStatus.textContent = data.left;
        if (rightStatus) rightStatus.textContent = data.right;
        if (motorSpeed) motorSpeed.textContent = Math.abs(data.left || data.right || 0);
        if (motorDirection) {
            // Calculate direction based on motor values
            let direction = 0;
            if (data.left > data.right) direction = -90; // Left turn
            else if (data.right > data.left) direction = 90; // Right turn
            motorDirection.textContent = direction;
        }
    }
    
    function updateStatus(data) {

        // Update system status indicators
        const elements = {
            'wifi-status': data.ip || 'Unknown',
            'battery-status': data.battery || 'Unknown',
            'memory-status': data.memory || 'Unknown',
            'cpu-status': data.cpu || 'Unknown',
            'temperature-status': (data.temperature || '-127') + "C",
            'uptime-status': data.uptime || 'Unknown'
        };
        
        Object.entries(elements).forEach(([id, value]) => {
            const element = document.getElementById(id);
            if (element) element.textContent = value;
        });
    }
    
    // 3D visualization of gyroscope data using canvas
    let gyroCanvas, gyroCtx, gyroData = { x: 0, y: 0, z: 0 };
    let gyroAnimation;
    let targetGyroData = { x: 0, y: 0, z: 0 };
    let lastGyroUpdateTime = 0;
    // Current gyro Z value for the radar sweep
    let currentRadarAngle = 0;
    
    // 3D visualization of accelerometer data using canvas
    let accelCanvas, accelCtx, accelData = { x: 0, y: 0, z: 0 };
    let accelAnimation;
    let targetAccelData = { x: 0, y: 0, z: 0 };
    let lastAccelUpdateTime = 0;
    
    // Interpolation speed factor (higher = faster transitions)
    const interpolationSpeed = 0.15;
    
    function initGyroCanvas() {
        gyroCanvas = document.getElementById('gyro-canvas');
        if (!gyroCanvas) return;
        
        gyroCtx = gyroCanvas.getContext('2d');
        
        // Start the animation loop
        if (gyroAnimation) cancelAnimationFrame(gyroAnimation);
        renderGyro();
    }
    
    function renderGyro() {
        if (!gyroCanvas || !gyroCtx) return;
        
        // Get dynamic interpolation factor
        const factors = calculateInterpolationFactor();
        
        // Interpolate between current values and target values using dynamic factor
        gyroData.x = applyLowPassFilter(gyroData.x, targetGyroData.x, factors.gyro);
        gyroData.y = applyLowPassFilter(gyroData.y, targetGyroData.y, factors.gyro);
        gyroData.z = applyLowPassFilter(gyroData.z, targetGyroData.z, factors.gyro);
        
        // Clear the canvas
        gyroCtx.clearRect(0, 0, gyroCanvas.width, gyroCanvas.height);
        
        const width = gyroCanvas.width;
        const height = gyroCanvas.height;
        const centerX = width / 2;
        const centerY = height / 2;
        const size = Math.min(width, height) * 0.4;
        
        // Calculate 3D rotation matrix based on gyro values
        const angleX = gyroData.x / 50;  // Scale down to make rotation more manageable
        const angleY = gyroData.y / 50;
        const angleZ = gyroData.z / 50;
        
        // Draw 3D cube representing the gyroscope
        drawCube(centerX, centerY, size, angleX, angleY, angleZ);
        
        // Draw coordinate axes
        drawAxes(centerX, centerY, size, angleX, angleY, angleZ);
        
        // Continue animation loop
        gyroAnimation = requestAnimationFrame(renderGyro);
    }
    
    function drawAxes(centerX, centerY, size, rotX, rotY, rotZ) {
        // X-axis (red)
        drawAxis(centerX, centerY, size, rotX, rotY, rotZ, [1, 0, 0], 'red');
        
        // Y-axis (green)
        drawAxis(centerX, centerY, size, rotX, rotY, rotZ, [0, 1, 0], 'green');
        
        // Z-axis (blue)
        drawAxis(centerX, centerY, size, rotX, rotY, rotZ, [0, 0, 1], 'blue');
    }
    
    function drawAxis(centerX, centerY, size, rotX, rotY, rotZ, vector, color) {
        const start = rotatePoint([0, 0, 0], rotX, rotY, rotZ);
        const end = rotatePoint(vector.map(v => v * size * 1.5), rotX, rotY, rotZ);
        
        gyroCtx.beginPath();
        gyroCtx.moveTo(centerX + start[0], centerY + start[1]);
        gyroCtx.lineTo(centerX + end[0], centerY + end[1]);
        gyroCtx.strokeStyle = color;
        gyroCtx.lineWidth = 2;
        gyroCtx.stroke();
        
        // Draw axis label at the end of the axis
        gyroCtx.fillStyle = color;
        gyroCtx.font = '14px Arial';
        const label = color === 'red' ? 'X' : color === 'green' ? 'Y' : 'Z';
        gyroCtx.fillText(label, centerX + end[0] + 5, centerY + end[1] + 5);
    }
    
    function drawCube(centerX, centerY, size, rotX, rotY, rotZ) {
        // Define cube vertices (corners)
        const vertices = [
            [-1, -1, -1],
            [1, -1, -1],
            [1, 1, -1],
            [-1, 1, -1],
            [-1, -1, 1],
            [1, -1, 1],
            [1, 1, 1],
            [-1, 1, 1]
        ].map(v => v.map(coord => coord * size / 2));
        
        // Define edges connecting vertices
        const edges = [
            [0, 1], [1, 2], [2, 3], [3, 0],  // Bottom face
            [4, 5], [5, 6], [6, 7], [7, 4],  // Top face
            [0, 4], [1, 5], [2, 6], [3, 7]   // Connecting edges
        ];
        
        // Define faces for coloring
        const faces = [
            [0, 1, 2, 3],  // Bottom face
            [7, 6, 5, 4],  // Top face
            [4, 5, 1, 0],  // Front face
            [5, 6, 2, 1],  // Right face
            [6, 7, 3, 2],  // Back face
            [7, 4, 0, 3]   // Left face
        ];
        
        // Colors for faces
        const faceColors = [
            'rgba(255, 100, 100, 0.5)',  // Bottom (Red)
            'rgba(100, 255, 100, 0.5)',  // Top (Green)
            'rgba(100, 100, 255, 0.5)',  // Front (Blue)
            'rgba(255, 255, 100, 0.5)',  // Right (Yellow)
            'rgba(255, 100, 255, 0.5)',  // Back (Magenta)
            'rgba(100, 255, 255, 0.5)'   // Left (Cyan)
        ];
        
        // Rotate vertices
        const rotatedVertices = vertices.map(v => rotatePoint(v, rotX, rotY, rotZ));
        
        // Calculate z-order of faces for proper depth rendering
        const faceDepths = faces.map((face, index) => {
            // Calculate average z-coordinate of face
            const avgZ = face.reduce((sum, vIdx) => sum + rotatedVertices[vIdx][2], 0) / face.length;
            return { index, avgZ };
        }).sort((a, b) => a.avgZ - b.avgZ);
        
        // Draw faces in z-order (back to front)
        faceDepths.forEach(({ index }) => {
            const face = faces[index];
            
            gyroCtx.beginPath();
            gyroCtx.moveTo(centerX + rotatedVertices[face[0]][0], centerY + rotatedVertices[face[0]][1]);
            
            for (let i = 1; i < face.length; i++) {
                gyroCtx.lineTo(centerX + rotatedVertices[face[i]][0], centerY + rotatedVertices[face[i]][1]);
            }
            
            gyroCtx.closePath();
            gyroCtx.fillStyle = faceColors[index];
            gyroCtx.strokeStyle = 'rgba(200, 200, 200, 0.8)';
            gyroCtx.lineWidth = 1;
            gyroCtx.fill();
            gyroCtx.stroke();
        });
    }
    
    function rotatePoint(point, angleX, angleY, angleZ) {
        let [x, y, z] = point;
        
        // Rotate around X-axis
        let temp1 = y;
        let temp2 = z;
        y = temp1 * Math.cos(angleX) - temp2 * Math.sin(angleX);
        z = temp1 * Math.sin(angleX) + temp2 * Math.cos(angleX);
        
        // Rotate around Y-axis
        temp1 = x;
        temp2 = z;
        x = temp1 * Math.cos(angleY) + temp2 * Math.sin(angleY);
        z = -temp1 * Math.sin(angleY) + temp2 * Math.cos(angleY);
        
        // Rotate around Z-axis
        temp1 = x;
        temp2 = y;
        x = temp1 * Math.cos(angleZ) - temp2 * Math.sin(angleZ);
        y = temp1 * Math.sin(angleZ) + temp2 * Math.cos(angleZ);
        
        return [x, y, z];
    }
    
    function initAccelCanvas() {
        accelCanvas = document.getElementById('accel-canvas');
        if (!accelCanvas) return;
        
        accelCtx = accelCanvas.getContext('2d');
        
        // Start the animation loop
        if (accelAnimation) cancelAnimationFrame(accelAnimation);
        renderAccel();
    }
    
    function renderAccel() {
        if (!accelCanvas || !accelCtx) return;
        
        // Get dynamic interpolation factor
        const factors = calculateInterpolationFactor();
        
        // Interpolate between current values and target values using dynamic factor
        accelData.x = applyLowPassFilter(accelData.x, targetAccelData.x, factors.accel);
        accelData.y = applyLowPassFilter(accelData.y, targetAccelData.y, factors.accel);
        accelData.z = applyLowPassFilter(accelData.z, targetAccelData.z, factors.accel);
        
        // Clear the canvas
        accelCtx.clearRect(0, 0, accelCanvas.width, accelCanvas.height);
        
        const width = accelCanvas.width;
        const height = accelCanvas.height;
        const centerX = width / 2;
        const centerY = height / 2;
        const size = Math.min(width, height) * 0.4;
        
        // Draw the gravity vector
        drawGravityVector(centerX, centerY, size);
        
        // Draw coordinate axes
        drawAccelAxes(centerX, centerY, size);
        
        // Continue animation loop
        accelAnimation = requestAnimationFrame(renderAccel);
    }
    
    function drawGravityVector(centerX, centerY, size) {
        // Normalize acceleration vector
        const magnitude = Math.sqrt(
            accelData.x * accelData.x + 
            accelData.y * accelData.y + 
            accelData.z * accelData.z
        );
        
        if (magnitude === 0) return;
        
        const scale = size / Math.max(1, magnitude);
        const normalizedX = accelData.x * scale;
        const normalizedY = accelData.y * scale;
        const normalizedZ = accelData.z * scale;
        
        // Calculate vector endpoints with perspective
        const start = [0, 0, 0];
        const end = [normalizedX, normalizedY, normalizedZ];
        
        // Draw the main gravity vector (white with gradient)
        accelCtx.beginPath();
        accelCtx.moveTo(centerX, centerY);
        accelCtx.lineTo(centerX + normalizedX, centerY + normalizedY);
        
        // Create a gradient for the vector
        const gradient = accelCtx.createLinearGradient(
            centerX, centerY, 
            centerX + normalizedX, centerY + normalizedY
        );
        gradient.addColorStop(0, 'rgba(255, 255, 255, 1)');
        gradient.addColorStop(1, 'rgba(0, 200, 255, 1)');
        
        accelCtx.strokeStyle = gradient;
        accelCtx.lineWidth = 4;
        accelCtx.stroke();
        
        // Draw arrow head
        drawArrowhead(
            centerX + normalizedX, 
            centerY + normalizedY, 
            Math.atan2(normalizedY, normalizedX), 
            10
        );
        
        // Draw vector magnitude indicator
        accelCtx.font = '12px Arial';
        accelCtx.fillStyle = 'rgba(255, 255, 255, 0.8)';
        accelCtx.fillText(
            `${magnitude}g`, 
            centerX + normalizedX + 10, 
            centerY + normalizedY + 10
        );
        
        // Draw a circle representing Earth's gravity (1g) as reference
        accelCtx.beginPath();
        accelCtx.arc(centerX, centerY, size, 0, Math.PI * 2);
        accelCtx.strokeStyle = 'rgba(100, 100, 100, 0.4)';
        accelCtx.lineWidth = 1;
        accelCtx.stroke();
        
        // Draw the 3D sphere representing the accelerometer
        drawSphere(centerX, centerY, size / 5);
    }
    
    function drawSphere(centerX, centerY, radius) {
        // Create a radial gradient
        const gradient = accelCtx.createRadialGradient(
            centerX - radius/3, centerY - radius/3, radius/10,
            centerX, centerY, radius
        );
        
        gradient.addColorStop(0, 'rgba(100, 200, 255, 1)');
        gradient.addColorStop(0.8, 'rgba(30, 100, 180, 0.8)');
        gradient.addColorStop(1, 'rgba(20, 60, 120, 0.5)');
        
        // Draw sphere
        accelCtx.beginPath();
        accelCtx.arc(centerX, centerY, radius, 0, Math.PI * 2);
        accelCtx.fillStyle = gradient;
        accelCtx.fill();
        
        // Add highlight to create 3D effect
        accelCtx.beginPath();
        accelCtx.arc(centerX - radius/3, centerY - radius/3, radius/3, 0, Math.PI * 2);
        accelCtx.fillStyle = 'rgba(255, 255, 255, 0.3)';
        accelCtx.fill();
    }
    
    function drawArrowhead(x, y, angle, size) {
        accelCtx.save();
        accelCtx.translate(x, y);
        accelCtx.rotate(angle);
        
        // Draw the arrowhead
        accelCtx.beginPath();
        accelCtx.moveTo(0, 0);
        accelCtx.lineTo(-size, -size/2);
        accelCtx.lineTo(-size, size/2);
        accelCtx.closePath();
        
        accelCtx.fillStyle = 'rgba(0, 200, 255, 1)';
        accelCtx.fill();
        
        accelCtx.restore();
    }
    
    function drawAccelAxes(centerX, centerY, size) {
        // X-axis (red)
        accelCtx.beginPath();
        accelCtx.moveTo(centerX, centerY);
        accelCtx.lineTo(centerX + size, centerY);
        accelCtx.strokeStyle = 'red';
        accelCtx.lineWidth = 1;
        accelCtx.stroke();
        accelCtx.fillStyle = 'red';
        accelCtx.fillText('X', centerX + size + 5, centerY + 5);
        
        // Y-axis (green)
        accelCtx.beginPath();
        accelCtx.moveTo(centerX, centerY);
        accelCtx.lineTo(centerX, centerY - size);
        accelCtx.strokeStyle = 'green';
        accelCtx.lineWidth = 1;
        accelCtx.stroke();
        accelCtx.fillStyle = 'green';
        accelCtx.fillText('Y', centerX + 5, centerY - size - 5);
        
        // Z-axis (blue) - shown with perspective
        accelCtx.beginPath();
        accelCtx.moveTo(centerX, centerY);
        accelCtx.lineTo(centerX - size/2, centerY + size/2);
        accelCtx.strokeStyle = 'blue';
        accelCtx.lineWidth = 1;
        accelCtx.stroke();
        accelCtx.fillStyle = 'blue';
        accelCtx.fillText('Z', centerX - size/2 - 15, centerY + size/2 + 5);
    }
    
    function updateGyroVisualization(gyro) {
        // Update the target gyro data for the visualization
        targetGyroData = {
            x: gyro.x || 0,
            y: gyro.y || 0,
            z: gyro.z || 0
        };
        
        // Also update the currentRadarAngle for direct access
        // Scale and limit the influence for consistent calculations
        let gyroInfluence = (gyro.z || 0) / 30; 
        currentRadarAngle = Math.min(20, Math.max(-20, gyroInfluence));
        
        // Set last update timestamp
        lastGyroUpdateTime = Date.now();
        
        // Initialize the canvas if not already done
        if (!gyroCanvas || !gyroCtx) {
            // If first update, set current values to target to avoid initial animation
            if (gyroData.x === 0 && gyroData.y === 0 && gyroData.z === 0) {
                gyroData = {...targetGyroData};
            }
            initGyroCanvas();
        }
    }
    
    function updateAccelVisualization(accel) {
        // Update the target accel data for the visualization
        targetAccelData = {
            x: accel.x || 0,
            y: accel.y || 0,
            z: accel.z || 0
        };
        
        // Set last update timestamp
        lastAccelUpdateTime = Date.now();
        
        // Initialize the canvas if not already done
        if (!accelCanvas || !accelCtx) {
            // If first update, set current values to target to avoid initial animation
            if (accelData.x === 0 && accelData.y === 0 && accelData.z === 0) {
                accelData = {...targetAccelData};
            }
            initAccelCanvas();
        }
    }
    
    // Storage information display
    function updateStorageInfo(data) {
        // Format sizes in a human-readable format
        function formatSize(bytes) {
            var result = "";
            var unit = "";
            if (bytes < 1024) {
                result = bytes
                unit = ' B';
            } else if (bytes < 1024 * 1024) {
                result = (bytes / 1024)
                unit = ' KB';
            } else {
                result = (bytes / (1024 * 1024))
                unit = ' MB';
            }

            return toFixed(result, 2) + unit;
        }
        
        // Update storage information values
        const elements = {
            'storage-total': formatSize(data.total),
            'storage-used': formatSize(data.used),
            'storage-free': formatSize(data.free),
            'storage-percent': toFixed(data.percent, 2) + '%'
        };
        
        Object.entries(elements).forEach(([id, value]) => {
            const element = document.getElementById(id);
            if (element) element.textContent = value;
        });
        
        // Update progress bar
        const progressBar = document.querySelector('#files-section .progress div');
        if (progressBar) {
            progressBar.style.width = data.percent + '%';
        }
    }
    
    function fetchStorageInfo() {
        sendJsonMessage('storage_info', {});
    }
    
    // Camera control functions
    function takeCameraSnapshot() {
        sendJsonMessage('camera_command', { action: 'snapshot' });
        logToConsole('Taking camera snapshot...', 'info');
    }
    
    function setCameraZoom(direction) {
        sendJsonMessage('camera_command', { 
            action: 'zoom', 
            direction: direction 
        });
    }
    
    function setCameraResolution(resolution) {
        sendJsonMessage('camera_command', { 
            action: 'set_resolution', 
            resolution: resolution 
        });
    }
    
    function setCameraSetting(setting, value) {
        sendJsonMessage('camera_command', { 
            action: 'set_setting',
            setting: setting,
            value: value
        });
    }
    
    // Motor preset functions
    function sendMotorPreset(direction) {
        const presets = {
            'forward': { left: 200, right: 200 },
            'backward': { left: -200, right: -200 },
            'left': { left: -150, right: 150 },
            'right': { left: 150, right: -150 },
            'stop': { left: 0, right: 0 }
        };
        
        const preset = presets[direction];
        if (preset) {
            sendMotorCommand(preset.left, preset.right, 1000);
        }
    }
    
    // System functions
    function resetMotors() {
        sendJsonMessage('motor_command', { action: 'reset' });
        logToConsole('Resetting motors...', 'info');
    }
    
    function emergencyStop() {
        sendJsonMessage('emergency_stop', {});
        logToConsole('EMERGENCY STOP activated!', 'error');
    }
    
    function refreshSystemStatus() {
        sendJsonMessage('system_status', {});
    }
    
    function sendDebugCommand() {
        const commandInput = document.getElementById('debug-command');
        if (commandInput && commandInput.value.trim()) {
            const command = commandInput.value.trim();
            sendJsonMessage('debug_command', { command: command });
            logToConsole(`Debug command sent: ${command}`, 'info');
            commandInput.value = '';
        }
    }
    
    function clearRecentLogs() {
        const logsContainer = document.querySelector('#dashboard-section .console-container');
        if (logsContainer) {
            logsContainer.innerHTML = '';
        }
    }
    
    function createNewFolder() {
        const folderName = prompt('Enter folder name:');
        if (folderName && folderName.trim()) {
            sendJsonMessage('create_folder', {
                path: currentPath,
                name: folderName.trim()
            });
            logToConsole(`Creating folder: ${folderName}`, 'info');
            // Update storage info after creating a folder
            setTimeout(() => {
                fetchFiles(currentPath);
                fetchStorageInfo();
            }, 500);
        }
    }

    // Missing function implementations
    let currentPath = '/';

    function fetchWifiNetworks() {
        sendJsonMessage('get_wifi_networks', {});
        logToConsole('Scanning for WiFi networks...', 'info');
    }

    function fetchFiles(path) {
        currentPath = path || '/';
        sendJsonMessage('list_files', { path: currentPath });
        logToConsole(`Loading files from ${currentPath}...`, 'info');
    }

    function populateWifiList(networks) {
        if (!wifiList) return;
        
        wifiList.innerHTML = '';
        
        if (!networks || networks.length === 0) {
            const noNetworks = document.createElement('div');
            noNetworks.className = 'wifi-item';
            noNetworks.innerHTML = '<div class="wifi-name">No networks found</div>';
            wifiList.appendChild(noNetworks);
            return;
        }
        
        networks.forEach(network => {
            const wifiItem = document.createElement('div');
            wifiItem.className = 'wifi-item';
            
            const signalStrength = network.rssi > -50 ? 'excellent' : 
                                 network.rssi > -60 ? 'good' : 
                                 network.rssi > -70 ? 'fair' : 'poor';
            
            wifiItem.innerHTML = `
                <div class="wifi-name">${network.ssid}</div>
                <div class="wifi-signal signal-${signalStrength}">${network.rssi} dBm</div>
                <div class="wifi-encryption">${network.encryption || 'Open'}</div>
                <button class="btn btn-sm wifi-connect" data-ssid="${network.ssid}">Connect</button>
            `;
            
            const connectBtn = wifiItem.querySelector('.wifi-connect');
            connectBtn.addEventListener('click', () => {
                const password = prompt(`Enter password for ${network.ssid}:`);
                if (password !== null) {
                    sendJsonMessage('connect_wifi', {
                        ssid: network.ssid,
                        password: password
                    });
                    logToConsole(`Connecting to ${network.ssid}...`, 'info');
                }
            });
            
            wifiList.appendChild(wifiItem);
        });
    }

    function populateFileList(files) {
        if (!fileList) return;
        
        fileList.innerHTML = '';
        
        // Add breadcrumb
        if (breadcrumb) {
            const pathParts = currentPath.split('/').filter(part => part);
            breadcrumb.innerHTML = '<span class="breadcrumb-item" data-path="/">Root</span>';
            
            let currentBreadcrumbPath = '';
            pathParts.forEach(part => {
                currentBreadcrumbPath += '/' + part;
                breadcrumb.innerHTML += ` / <span class="breadcrumb-item" data-path="${currentBreadcrumbPath}">${part}</span>`;
            });
            
            // Add click handlers to breadcrumb items
            breadcrumb.querySelectorAll('.breadcrumb-item').forEach(item => {
                item.addEventListener('click', () => {
                    const path = item.getAttribute('data-path');
                    fetchFiles(path);
                });
            });
        }
        
        if (!files || files.length === 0) {
            const noFiles = document.createElement('div');
            noFiles.className = 'file-item';
            noFiles.innerHTML = '<div class="file-name">No files found</div>';
            fileList.appendChild(noFiles);
            return;
        }
        
        files.forEach(file => {
            const fileItem = document.createElement('div');
            fileItem.className = 'file-item';
            
            const isDirectory = file.type === 'directory' || file.name.endsWith('/');
            const icon = isDirectory ? 'fas fa-folder' : 'fas fa-file';
            const fileName = file.name;
            const filePath = file.path;
            const fullname = filePath + fileName;
            
            // Get file extension for icon
            let fileIcon = icon;
            if (!isDirectory) {
                const extension = fileName.split('.').pop().toLowerCase();
                if (['jpg', 'jpeg', 'png', 'gif', 'bmp'].includes(extension)) {
                    fileIcon = 'fas fa-file-image';
                } else if (['txt', 'log', 'md'].includes(extension)) {
                    fileIcon = 'fas fa-file-alt';
                } else if (['html', 'htm', 'css', 'js'].includes(extension)) {
                    fileIcon = 'fas fa-file-code';
                } else if (['json', 'xml', 'csv'].includes(extension)) {
                    fileIcon = 'fas fa-file-code';
                }
            }
            
            fileItem.innerHTML = `
                <i class="file-icon ${fileIcon}"></i>
                <div class="file-name">${fileName}</div>
                <div class="file-size pe-2">${file.size ? formatFileSize(file.size) : ''}</div>
                <div class="file-actions">
                    ${isDirectory ? 
                        `<button class="btn btn-sm file-open" data-path="${fullname}">Open</button>` :
                        `<button class="btn btn-sm file-view" data-path="${fullname}">View</button>
                         <button class="btn btn-sm file-download" data-path="${fullname}">Download</button>`
                    }
                    <button class="btn btn-sm btn-danger file-delete" data-path="${fullname}">Delete</button>
                </div>
            `;
            
            // Add event listeners
            const openBtn = fileItem.querySelector('.file-open');
            if (openBtn) {
                openBtn.addEventListener('click', () => {
                    fetchFiles(openBtn.getAttribute('data-path'));
                });
            }
            
            const viewBtn = fileItem.querySelector('.file-view');
            if (viewBtn) {
                viewBtn.addEventListener('click', () => {
                    readFile(viewBtn.getAttribute('data-path'));
                });
            }
            
            const downloadBtn = fileItem.querySelector('.file-download');
            if (downloadBtn) {
                downloadBtn.addEventListener('click', () => {
                    // Create download link
                    const link = document.createElement('a');
                    link.href = `/download?path=${encodeURIComponent(downloadBtn.getAttribute('data-path'))}`;
                    link.download = fileName;
                    document.body.appendChild(link);
                    link.click();
                    document.body.removeChild(link);
                });
            }
            
            const deleteBtn = fileItem.querySelector('.file-delete');
            if (deleteBtn) {
                deleteBtn.addEventListener('click', () => {
                    if (confirm(`Delete ${fileName}?`)) {
                        sendJsonMessage('delete_file', { path: deleteBtn.getAttribute('data-path') });
                        logToConsole(`Deleting ${fileName}...`, 'info');
                        // Update storage info after deleting a file
                        setTimeout(() => {
                            fetchFiles(currentPath);
                            fetchStorageInfo();
                        }, 1000);
                    }
                });
            }
            
            fileList.appendChild(fileItem);
        });
    }
    
    // Format file size for display
    function formatFileSize(bytes) {
        if (bytes === 0) return '0 B';
        const k = 1024;
        const sizes = ['B', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }
    
    // Read file content
    function readFile(filePath) {
        sendJsonMessage('read_file', { path: filePath });
        logToConsole(`Reading file ${filePath}...`, 'info');
    }
    
    // Display file content in the viewer
    function displayFileContent(data) {
        if (!data || !data.success) {
            logToConsole(`Failed to read file: ${data?.message || 'Unknown error'}`, 'error');
            return;
        }
        
        // Get file name from path
        const pathParts = data.path.split('/');
        const fileName = pathParts[pathParts.length - 1];
        
        // Set file name in viewer header
        const fileViewerName = document.getElementById('file-viewer-name');
        if (fileViewerName) {
            fileViewerName.textContent = fileName;
        }
        
        const contentContainer = document.getElementById('file-viewer-content');
        const imageContainer = document.getElementById('file-viewer-image-container');
        const imageElement = document.getElementById('file-viewer-image');
        const viewerContainer = document.getElementById('file-viewer-container');
        
        if (!contentContainer || !imageContainer || !imageElement || !viewerContainer) {
            logToConsole('File viewer elements not found', 'error');
            return;
        }
        
        // Check file type
        const fileType = data.type?.toLowerCase();
        const imageTypes = ['jpg', 'jpeg', 'png', 'gif', 'bmp'];
        
        if (imageTypes.includes(fileType)) {
            // Display as image - create data URL
            // Use decodeURIComponent instead of the deprecated unescape
            const base64Content = btoa(decodeURIComponent(encodeURIComponent(data.content)));
            const imgSrc = `data:image/${fileType === 'jpg' ? 'jpeg' : fileType};base64,${base64Content}`;
            
            // Show image container, hide content container
            contentContainer.style.display = 'none';
            imageContainer.style.display = 'block';
            imageElement.src = imgSrc;
        } else {
            // Display as text
            contentContainer.style.display = 'block';
            imageContainer.style.display = 'none';
            contentContainer.textContent = data.content;
            
            // Apply basic syntax highlighting based on file type
            if (['js', 'javascript'].includes(fileType)) {
                applySyntaxHighlighting(contentContainer, 'js');
            } else if (['html', 'htm'].includes(fileType)) {
                applySyntaxHighlighting(contentContainer, 'html');
            } else if (['css'].includes(fileType)) {
                applySyntaxHighlighting(contentContainer, 'css');
            } else if (['json'].includes(fileType)) {
                applySyntaxHighlighting(contentContainer, 'json');
            }
        }
        
        // Show the viewer
        viewerContainer.style.display = 'block';
    }
    
    // Very basic syntax highlighting
    function applySyntaxHighlighting(element, language) {
        // This is a simplified version - for production, use a proper syntax highlighting library
        let content = element.textContent;
        
        // Replace HTML with highlighted version based on language
        if (language === 'js') {
            // Very basic JS highlighting
            const keywords = ['function', 'return', 'var', 'let', 'const', 'if', 'else', 'for', 'while', 'class', 'import', 'export', 'default'];
            keywords.forEach(keyword => {
                const regex = new RegExp(`\\b${keyword}\\b`, 'g');
                content = content.replace(regex, `<span class="syntax-keyword">${keyword}</span>`);
            });
            
            // Strings
            content = content.replace(/"[^"]*"/g, match => `<span class="syntax-string">${match}</span>`);
            content = content.replace(/'[^']*'/g, match => `<span class="syntax-string">${match}</span>`);
            
            // Numbers
            content = content.replace(/\b(\d+)\b/g, '<span class="syntax-number">$1</span>');
            
            // Comments
            content = content.replace(/\/\/[^\n]*/g, match => `<span class="syntax-comment">${match}</span>`);
        } else if (language === 'html') {
            // Very basic HTML highlighting
            content = content.replace(/(&lt;[^&]*&gt;)/g, '<span class="syntax-html">$1</span>');
        }
        
        element.innerHTML = content;
    }

    function addChatMessage(messageData, isOutgoing = false) {
        if (!chatMessages) return;
        
        const messageDiv = document.createElement('div');
        messageDiv.className = `chat-message ${isOutgoing ? 'message-outgoing' : 'message-incoming'}`;
        
        const timestamp = new Date().toTimeString().split(' ')[0];
        const sender = isOutgoing ? 'User' : 'System';
        
        messageDiv.innerHTML = `
            <div class="message-header">
                <span class="message-sender">${sender}</span>
                <span class="message-time">${timestamp}</span>
            </div>
            <div class="message-content">${messageData.content || messageData.message || ''}</div>
        `;
        
        chatMessages.appendChild(messageDiv);
        chatMessages.scrollTop = chatMessages.scrollHeight;
    }

    function uploadFile() {
        const fileInput = document.getElementById('file-upload');
        if (!fileInput || !fileInput.files.length) return;
        
        const file = fileInput.files[0];
        const formData = new FormData();
        formData.append('file', file);
        
        fetch('/upload?path='+currentPath, {
            method: 'POST',
            body: formData
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                logToConsole(`File ${file.name} uploaded successfully`, 'success');
                fetchFiles(currentPath); // Refresh file list
                fetchStorageInfo(); // Update storage info
            } else {
                logToConsole(`Upload failed: ${data.message}`, 'error');
            }
        })
        .catch(error => {
            logToConsole(`Upload error: ${error.message}`, 'error');
        });
        
        // Clear the input
        fileInput.value = '';
    }

    function sendMotorCommand(left, right, duration = 0) {
        sendJsonMessage('motor_command', {
            left: left,
            right: right,
            duration: duration
        });
    }

    // Event Listeners
    if (loginForm) {
        loginForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const username = document.getElementById('username').value;
            const password = document.getElementById('password').value;
            
            sendCommand('login', {
                username: username,
                password: password
            });
        });
    }
    
    if (chatForm) {
        chatForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const chatInput = document.getElementById('chat-input');
            const message = chatInput.value.trim();
            
            if (message) {
                sendCommand('send_chat', { content: message });
                addChatMessage({ content: message }, true);
                chatInput.value = '';
            }
        });
    }
    
    if (sidebarToggle) {
        sidebarToggle.addEventListener('click', () => {
            sidebar.classList.toggle('sidebar-collapsed');
        });
    }
    
    // Mobile menu toggle
    const mobileMenuToggle = document.getElementById('mobile-menu-toggle');
    if (mobileMenuToggle) {
        mobileMenuToggle.addEventListener('click', () => {
            sidebar.classList.toggle('show');
        });
    }
    
    menuItems.forEach(item => {
        item.addEventListener('click', () => {
            const target = item.getAttribute('data-target');
            showContent(target);
            
            if (window.innerWidth <= 768) {
                sidebar.classList.remove('show');
            }
        });
    });
    
    // WiFi controls
    const refreshWifiBtn = document.getElementById('refresh-wifi');
    if (refreshWifiBtn) {
        refreshWifiBtn.addEventListener('click', fetchWifiNetworks);
    }
    
    // File controls
    const uploadFileBtn = document.getElementById('upload-file-btn');
    if (uploadFileBtn) {
        uploadFileBtn.addEventListener('click', () => {
            const fileInput = document.getElementById('file-upload');
            if (fileInput) fileInput.click();
        });
    }
    
    const fileInput = document.getElementById('file-upload');
    if (fileInput) {
        fileInput.addEventListener('change', uploadFile);
    }
    
    const createFolderBtn = document.getElementById('create-folder-btn');
    if (createFolderBtn) {
        createFolderBtn.addEventListener('click', createNewFolder);
    }

    const refreshFiles = document.getElementById('refresh-btn');
    if (refreshFiles) {
        refreshFiles.addEventListener('click', () => {
            fetchFiles(currentPath);
            fetchStorageInfo();
        });
    }
    
    // File viewer controls
    const closeFileViewerBtn = document.getElementById('close-file-viewer');
    if (closeFileViewerBtn) {
        closeFileViewerBtn.addEventListener('click', () => {
            const viewerContainer = document.getElementById('file-viewer-container');
            if (viewerContainer) {
                viewerContainer.style.display = 'none';
            }
        });
    }
    
    // Camera controls
    const cameraToggleBtn = document.getElementById('camera-toggle');
    if (cameraToggleBtn) {
        cameraToggleBtn.addEventListener('click', toggleCamera);
    }
    
    const cameraToggleMainBtn = document.getElementById('camera-toggle-main');
    if (cameraToggleMainBtn) {
        cameraToggleMainBtn.addEventListener('click', toggleCamera);
    }
    
    const cameraSnapshotBtn = document.getElementById('camera-snapshot');
    if (cameraSnapshotBtn) {
        cameraSnapshotBtn.addEventListener('click', takeCameraSnapshot);
    }
    
    const cameraZoomInBtn = document.getElementById('camera-zoom-in');
    if (cameraZoomInBtn) {
        cameraZoomInBtn.addEventListener('click', () => setCameraZoom('in'));
    }
    
    const cameraZoomOutBtn = document.getElementById('camera-zoom-out');
    if (cameraZoomOutBtn) {
        cameraZoomOutBtn.addEventListener('click', () => setCameraZoom('out'));
    }
    
    const cameraResolutionSelect = document.getElementById('camera-resolution');
    if (cameraResolutionSelect) {
        cameraResolutionSelect.addEventListener('change', (e) => {
            setCameraResolution(e.target.value);
        });
    }
    
    // Camera settings sliders
    const cameraBrightness = document.getElementById('camera-brightness');
    if (cameraBrightness) {
        cameraBrightness.addEventListener('input', (e) => {
            setCameraSetting('brightness', parseInt(e.target.value));
        });
    }
    
    const cameraContrast = document.getElementById('camera-contrast');
    if (cameraContrast) {
        cameraContrast.addEventListener('input', (e) => {
            setCameraSetting('contrast', parseInt(e.target.value));
        });
    }
    
    const cameraSaturation = document.getElementById('camera-saturation');
    if (cameraSaturation) {
        cameraSaturation.addEventListener('input', (e) => {
            setCameraSetting('saturation', parseInt(e.target.value));
        });
    }
    
    const cameraFlash = document.getElementById('camera-flash');
    if (cameraFlash) {
        cameraFlash.addEventListener('change', (e) => {
            setCameraSetting('flash', e.target.checked);
        });
    }
    
    // Motor preset controls
    const presetForward = document.getElementById('preset-forward');
    if (presetForward) {
        presetForward.addEventListener('click', () => sendMotorPreset('forward'));
    }
    
    const presetBackward = document.getElementById('preset-backward');
    if (presetBackward) {
        presetBackward.addEventListener('click', () => sendMotorPreset('backward'));
    }
    
    const presetLeft = document.getElementById('preset-left');
    if (presetLeft) {
        presetLeft.addEventListener('click', () => sendMotorPreset('left'));
    }
    
    const presetRight = document.getElementById('preset-right');
    if (presetRight) {
        presetRight.addEventListener('click', () => sendMotorPreset('right'));
    }
    
    const presetStop = document.getElementById('preset-stop');
    if (presetStop) {
        presetStop.addEventListener('click', () => sendMotorPreset('stop'));
    }
    
    // Automation control functions
    function requestAutomationStatus() {
        if (connected) {
            sendJsonMessage('get_automation_status');
            
        }
    }
    
    function setAutomationEnabled(enabled) {
        if (connected) {
            sendJsonMessage('automation_control', { enabled: enabled });
            
        }
    }
    
    function updateAutomationStatus(data) {
        const automationToggle = document.getElementById('automation-toggle');
        if (automationToggle) {
            automationToggle.checked = data.enabled;
            
        }
    }

    // Automation control UI
    const automationToggle = document.getElementById('automation-toggle');
    if (automationToggle) {
        automationToggle.addEventListener('change', (e) => {
            setAutomationEnabled(e.target.checked);
        });
        
        // Request current status when page loads
        setTimeout(() => {
            requestAutomationStatus();
        }, 1000);
    }
    
    // Dashboard controls - use more specific selectors
    const dashboardRefreshBtn = document.querySelector('#dashboard-section .btn:nth-of-type(1)');
    if (dashboardRefreshBtn && dashboardRefreshBtn.textContent.includes('Refresh')) {
        dashboardRefreshBtn.addEventListener('click', refreshSystemStatus);
    }
    
    // Add event listeners to quick control buttons
    const quickControlButtons = document.querySelectorAll('#dashboard-section .flex.gap-2 .btn');
    quickControlButtons.forEach((btn, index) => {
        if (btn.textContent.includes('Reset Motors')) {
            btn.addEventListener('click', resetMotors);
        } else if (btn.textContent.includes('Emergency Stop')) {
            btn.addEventListener('click', emergencyStop);
        }
    });
    
    const clearRecentLogsBtn = document.getElementById('clear-recent-logs');
    if (clearRecentLogsBtn) {
        clearRecentLogsBtn.addEventListener('click', clearRecentLogs);
    }
    
    // Debug controls
    const sendCommandBtn = document.getElementById('send-command');
    if (sendCommandBtn) {
        sendCommandBtn.addEventListener('click', sendDebugCommand);
    }
    
    const debugCommandInput = document.getElementById('debug-command');
    if (debugCommandInput) {
        debugCommandInput.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                sendDebugCommand();
            }
        });
    }
    
    const clearConsoleBtn = document.getElementById('clear-console');
    if (clearConsoleBtn) {
        clearConsoleBtn.addEventListener('click', () => {
            if (consoleOutput) {
                consoleOutput.innerHTML = '';
            }
        });
    }
    
    // Distance Sensor controls
    const distanceRequestBtn = document.getElementById('distance-request');
    if (distanceRequestBtn) {
        distanceRequestBtn.addEventListener('click', () => {
            // Check WebSocket connection before sending request
            if (checkWebSocketConnection()) {
                // Send a request to get current distance measurement
                sendJsonMessage('distance_request', {});
                
            }
        });
    }
    
    // Add window resize listener to reposition joysticks
    window.addEventListener('resize', resetJoysticks);
    
    // Function to reset joysticks position
    function resetJoysticks() {
        const motorJoystick = document.getElementById('motor-joystick');
        
        // Reset motor joystick
        if (motorJoystick) {
            motorJoystick.style.left = '50%';
            motorJoystick.style.top = '50%';
            updateJoystickDisplayValues('motor', 0, 0); // Reset motor display values
            // Also notify the server
            sendJoystickPosition('motor', 0, 0);
        }
    }
    
    // Call reset after a short delay to ensure elements are fully rendered
    setTimeout(resetJoysticks, 100);
    
    // Check for saved authentication token before showing login page
    checkSavedAuth();
    
    // Connect to WebSocket
    connectWebSocket();
    window.connectWS = connectWebSocket;
        
    // Show dashboard by default when logged in
    showContent('dashboard-section');
    
    function handleFileOperation(data) {
        // Log the result of the file operation
        const message = data.message || (data.success ? "Operation successful" : "Operation failed");
        const level = data.success ? "success" : "error";
        
        logToConsole(message, level);
        
        // If the operation was successful, refresh the file list
        if (data.success) {
            setTimeout(() => fetchFiles(currentPath), 500);
            // Also update storage info
            setTimeout(fetchStorageInfo, 700);
        }
    }
    
    function handleWifiConnection(data) {
        const message = data.message || (data.success ? "WiFi connection successful" : "WiFi connection failed");
        const level = data.success ? "success" : "error";
        
        logToConsole(message, level);
        
        // If connection was successful, update UI and refresh system status
        if (data.success) {
            // Update the system status to reflect new connection
            setTimeout(refreshSystemStatus, 1000);
        }
    }
    
    // Functions to handle task-related messages
    function handleTaskList(data) {
        logToConsole('Received task list', 'info');
        
        // If data is an array, we can display each task
        if (Array.isArray(data)) {
            data.forEach(task => {
                const taskName = task.name || 'Unknown';
                const taskStatus = task.status || 'unknown';
                const taskDesc = task.description || '';
                
                logToConsole(`Task: ${taskName} - Status: ${taskStatus} ${taskDesc ? '- ' + taskDesc : ''}`, 'info');
            });
        }
        
        // Here you could update UI elements if there's a task list display in the interface
        // For example:
        // const taskListElement = document.getElementById('task-list');
        // if (taskListElement) {
        //    taskListElement.innerHTML = data.map(task => 
        //        `<div class="task-item ${task.status}">${task.name} - ${task.status}</div>`
        //    ).join('');
        // }
    }
    
    function handleTaskControlResponse(data) {
        const message = data.message || (data.success ? 'Task operation successful' : 'Task operation failed');
        const level = data.success ? 'success' : 'error';
        
        logToConsole(message, level);
        
        // Refresh system status after a task control operation
        if (data.success) {
            setTimeout(refreshSystemStatus, 1000);
        }
    }
    
    // Function to handle notification messages from the server
    function handleNotification(data) {
        const message = data.message || 'System notification';
        const level = data.level || 'info';
        
        // Log the notification to the console
        logToConsole(message, level);
        
        
        // You could also display a more prominent notification if desired
        // For example, a toast notification or modal dialog
        // This depends on the UI framework or custom notification system in use
    }
    
    // Function to handle WiFi configuration data
    function handleWifiConfig(data) {
        logToConsole('Received WiFi configuration', 'info');
        
        // Update any WiFi configuration form fields if they exist
        const ssidField = document.getElementById('wifi-ssid');
        const apSsidField = document.getElementById('wifi-ap-ssid');
        const apModeSwitch = document.getElementById('ap-mode-switch');
        
        if (ssidField && data.ssid) {
            ssidField.value = data.ssid;
        }
        
        if (apSsidField && data.ap_ssid) {
            apSsidField.value = data.ap_ssid;
        }
        
        if (apModeSwitch && data.is_ap_mode !== undefined) {
            apModeSwitch.checked = data.is_ap_mode;
        }
        
        // Update connection status display if it exists
        const connectionStatus = document.getElementById('wifi-connection-status');
        if (connectionStatus && data.connected !== undefined) {
            connectionStatus.textContent = data.connected ? 'Connected' : 'Disconnected';
            connectionStatus.className = data.connected ? 'status-connected' : 'status-disconnected';
        }
    }
    
    // Function to handle WiFi configuration update response
    function handleWifiConfigUpdate(data) {
        const message = data.message || (data.success ? 'WiFi configuration updated' : 'Failed to update WiFi configuration');
        const level = data.success ? 'success' : 'error';
        
        logToConsole(message, level);
        
        // If update was successful, refresh system status after a short delay
        if (data.success) {
            setTimeout(refreshSystemStatus, 1000);
        }
    }
    
    // Function to handle camera frame header data
    function handleCameraFrameHeader(data) {
        // Store frame header information for the next binary message (used for camera_frame_header messages)
        // This serves the same purpose as updateCameraFrame but for a different message type
        frameHeader = data;
    }
    
    // Function to check for saved authentication token
    function checkSavedAuth() {
        try {
            const savedAuth = localStorage.getItem(AUTH_TOKEN_KEY);
            if (savedAuth) {
                const authData = JSON.parse(savedAuth);
                
                // Check if the token is valid and not expired
                // You can add token expiration logic here if needed
                // For example, tokens expire after 30 days:
                const now = Date.now();
                const tokenAge = now - authData.timestamp;
                const tokenMaxAge = 30 * 24 * 60 * 60 * 1000; // 30 days
                
                if (tokenAge < tokenMaxAge) {
                    // Token is valid, auto-login
                    isLoggedIn = true;
                    
                    // Show main app, hide login
                    loginPage.classList.add('hidden');
                    mainApp.classList.remove('hidden');
                    
                    logToConsole(`Welcome back, ${authData.username}`, 'success');
                    
                    return true;
                } else {
                    // Token expired, remove it
                    localStorage.removeItem(AUTH_TOKEN_KEY);
                    logToConsole('Previous session expired, please log in again', 'info');
                }
            }
        } catch (error) {
            console.error('Error checking saved authentication:', error);
            localStorage.removeItem(AUTH_TOKEN_KEY); // Clear potentially corrupted data
        }
        
        // Show login page by default (no valid saved auth)
        loginPage.classList.remove('hidden');
        mainApp.classList.add('hidden');
        return false;
    }
    
    // Function to handle logout
    function logout() {
        // Tell server we're logging out
        if (websocket && websocket.readyState === WebSocket.OPEN) {
            sendJsonMessage('logout', {});
        }
        
        // Clear authentication data
        isLoggedIn = false;
        localStorage.removeItem(AUTH_TOKEN_KEY);
        
        // Show login page
        loginPage.classList.remove('hidden');
        mainApp.classList.add('hidden');
        
        logToConsole('Logged out successfully', 'info');
        
        // Reconnect WebSocket to clear any session data on server
        if (websocket && websocket.readyState === WebSocket.OPEN) {
            websocket.close();
            setTimeout(connectWebSocket, 500);
        }
    }
    
    // Function to handle logout response
    function handleLogoutResponse(data) {
        // We've already handled the logout locally,
        // but we can show server's response message if needed
        if (data.message) {
            logToConsole(data.message, data.success ? 'info' : 'error');
        }
    }
    
    // Advanced interpolation function with dynamic speed based on update time
    function calculateInterpolationFactor() {
        const now = Date.now();
        const timeSinceGyroUpdate = now - lastGyroUpdateTime;
        const timeSinceAccelUpdate = now - lastAccelUpdateTime;
        
        // Adjust speed based on time since last update
        // This helps prevent jitter when updates are delayed
        const gyroFactor = Math.min(1, Math.max(0.05, interpolationSpeed * (1 - timeSinceGyroUpdate / 1000)));
        const accelFactor = Math.min(1, Math.max(0.05, interpolationSpeed * (1 - timeSinceAccelUpdate / 1000)));
        
        return {
            gyro: gyroFactor,
            accel: accelFactor
        };
    }
    
    // Apply low-pass filter to help reduce noise in sensor data
    function applyLowPassFilter(current, target, factor = 0.1) {
        return current * (1 - factor) + target * factor;
    }

    function toFixed(data, fix){
        // Convert input to number if it's a string
        const value = typeof data === 'string' ? parseFloat(data) : data;
        
        // Check for NaN and return 0 if invalid
        if (isNaN(value)) return 0;
        
        const fixed = Math.pow(10, fix);
        return Math.round(value * fixed) / fixed;
    }
    
    // Initialize
    initJoysticks();
    initGyroCanvas();
    initAccelCanvas();
    
    // Request initial distance data after a delay to ensure WebSocket is connected
    // setTimeout(() => {
    //     if (document.getElementById('distance-value') && checkWebSocketConnection()) {
    //         sendJsonMessage('distance_request', {});
            
    //     }
    // }, 2000);
    
    // Set up automatic regular distance updates
    // setInterval(() => {
    //     // Only request distance updates if the sensor section is visible and WebSocket is connected
    //     if (document.getElementById('distance-value') && 
    //         document.getElementById('sensors-section').style.display !== 'none' &&
    //         checkWebSocketConnection()) {
    //         sendJsonMessage('distance_request', {});
    //     }

    //     // Periodically check system status
    //     if (connected) {
    //         sendJsonMessage('get_status', {});
    //     }
    // }, 1000);
});
