// Main JavaScript for IoT Dashboard
document.addEventListener('DOMContentLoaded', () => {
    // Configuration
    const wsUri = `ws://${window.location.hostname}/ws`;
    let websocket = null;
    let isLoggedIn = false;
    let reconnectInterval = null;
    let connected = false;
    const reconnectTime = 3000; // 3 seconds
    const AUTH_TOKEN_KEY = 'cozmo_auth_token'; // Key for storing auth token in localStorage

    // DOM Elements
    const loginPage = document.getElementById('login-page');
    const mainApp = document.getElementById('main-app');
    const consoleOutput = document.getElementById('console-output');
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
    
    // Connect WebSocket
    function connectWebSocket() {
        console.log('Connecting to WebSocket...');
        websocket = new WebSocket(wsUri);

        websocket.onopen = (evt) => {
            console.log('WebSocket Connected');
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
                sendCommand('get_status');
                fetchWifiNetworks();
                fetchFiles('/');
                fetchStorageInfo();
                startSensorUpdates();
                connected = true;
            }
        };

        websocket.onclose = (evt) => {
            connected = false;
            console.log('WebSocket Disconnected');
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

    // Send Command to server using DTO format
    function sendJsonMessage(type, data = {}) {
        if (websocket && websocket.readyState === WebSocket.OPEN) {
            const message = JSON.stringify({
                type: type,
                data: data
            });
            websocket.send(message);
        } else {
            logToConsole('WebSocket not connected', 'error');
        }
    }

    // Legacy function for backward compatibility
    function sendCommand(command, data = {}) {
        sendJsonMessage(command, data);
    }

    function sendCommandToRobot(robotId, command, data = {}) {
        if (websocket && websocket.readyState === WebSocket.OPEN) {
            const message = {
                type: 'command_to_robot',
                data: {
                    robot_id: robotId,
                    command: command,
                    payload: data
                }
            };
            websocket.send(JSON.stringify(message));
        } else {
            logToConsole('WebSocket not connected', 'error');
        }
    }

    // Handle WebSocket messages
    function handleWebSocketMessage(evt) {
        // Handle binary data (camera frames)
        if (evt.data instanceof ArrayBuffer || evt.data instanceof Blob) {
            handleBinaryMessage(evt.data);
            return;
        }
        
        // Handle text data (JSON messages)
        try {
            const msg = JSON.parse(evt.data);
            console.log('Received:', msg);

            // Handle DTO format messages
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
            } else if (msg.type === 'error') {
                handleError(msg);
            } else if (msg.type === 'ok') {
                handleSuccess(msg);
            } else if (msg.type === 'motor_status') {
                updateMotorStatus(msg.data);
            } else if (msg.type === 'storage_info') {
                updateStorageInfo(msg.data);
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
            } else {
                console.log('Unknown message type:', msg.type, msg.data);
            }
        } catch (e) {
            console.error('Error parsing WebSocket message:', e);
            logToConsole(`Error parsing message: ${e.message}`, 'error');
        }
    }
    
    // Variables for frame processing
    let frameHeader = null;
    let frameQueue = [];
    const MAX_FRAME_QUEUE = 2; // Limit queued frames to avoid memory issues
    let processingFrame = false;

    // Handle binary messages (camera frames)
    function handleBinaryMessage(data) {
        // If we don't have frame header metadata, ignore this binary frame or assume it's a file upload response
        if (!frameHeader) {
            console.log("Received binary data without header - likely a file upload response");
            return;
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
        
        // Create a blob from the binary data
        const blob = frame.data instanceof Blob ? frame.data : new Blob([frame.data]);
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
        
        consoleOutput.appendChild(line);
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
        const servoJoystick = document.getElementById('servo-joystick');
        const motorJoystick = document.getElementById('motor-joystick');
        
        setupJoystick(servoJoystick, 'servo');
        setupJoystick(motorJoystick, 'motor');
    }
    
    function setupJoystick(element, type) {
        if (!element) return;
        
        const joystickContainer = element.parentElement;
        const containerRect = joystickContainer.getBoundingClientRect();
        const centerX = containerRect.width / 2;
        const centerY = containerRect.height / 2;
        const maxDistance = Math.min(centerX, centerY) * 0.8;
        
        let isDragging = false;
        
        // Center the joystick initially
        element.style.left = `${centerX - element.clientWidth / 2}px`;
        element.style.top = `${centerY - element.clientHeight / 2}px`;
        
        element.addEventListener('mousedown', startDrag);
        element.addEventListener('touchstart', startDrag, { passive: false });
        
        document.addEventListener('mousemove', drag);
        document.addEventListener('touchmove', drag, { passive: false });
        
        document.addEventListener('mouseup', endDrag);
        document.addEventListener('touchend', endDrag);
        
        function startDrag(e) {
            e.preventDefault();
            isDragging = true;
        }
        
        function drag(e) {
            if (!isDragging) return;
            e.preventDefault();
            
            let clientX, clientY;
            
            if (e.type === 'touchmove') {
                clientX = e.touches[0].clientX;
                clientY = e.touches[0].clientY;
            } else {
                clientX = e.clientX;
                clientY = e.clientY;
            }
            
            const rect = joystickContainer.getBoundingClientRect();
            const x = clientX - rect.left;
            const y = clientY - rect.top;
            
            // Calculate distance from center
            const deltaX = x - centerX;
            const deltaY = y - centerY;
            const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);
            
            // Limit to circle
            const limitDistance = Math.min(distance, maxDistance);
            const angle = Math.atan2(deltaY, deltaX);
            
            const limitedX = centerX + limitDistance * Math.cos(angle);
            const limitedY = centerY + limitDistance * Math.sin(angle);
            
            // Update joystick position
            element.style.left = `${limitedX - element.clientWidth / 2}px`;
            element.style.top = `${limitedY - element.clientHeight / 2}px`;
            
            // Map coordinates to -100 to 100
            const normalizedX = (limitedX - centerX) / maxDistance * 100;
            const normalizedY = (limitedY - centerY) / maxDistance * 100;
            
            // Send to server
            sendJoystickPosition(type, normalizedX, normalizedY);
        }
        
        function endDrag() {
            if (!isDragging) return;
            isDragging = false;
            
            // Return to center
            element.style.left = `${centerX - element.clientWidth / 2}px`;
            element.style.top = `${centerY - element.clientHeight / 2}px`;
            
            // Send neutral position
            sendJoystickPosition(type, 0, 0);
        }
    }
    
    function sendJoystickPosition(type, x, y) {
        sendCommand('joystick_update', {
            type: type,
            x: Math.round(x),
            y: Math.round(y)
        });
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
        sendCommand('start_sensor_updates');
    }
    
    function updateSensors(data) {
        if (data.gyro) {
            const gyroX = document.getElementById('gyro-x');
            const gyroY = document.getElementById('gyro-y');
            const gyroZ = document.getElementById('gyro-z');
            
            if (gyroX) gyroX.textContent = data.gyro.x;
            if (gyroY) gyroY.textContent = data.gyro.y;
            if (gyroZ) gyroZ.textContent = data.gyro.z;
            
            updateGyroVisualization(data.gyro);
        }
        
        if (data.accel) {
            const accelX = document.getElementById('accel-x');
            const accelY = document.getElementById('accel-y');
            const accelZ = document.getElementById('accel-z');
            
            if (accelX) accelX.textContent = data.accel.x;
            if (accelY) accelY.textContent = data.accel.y;
            if (accelZ) accelZ.textContent = data.accel.z;
        }
        
        // Update joystick position displays
        if (data.servo) {
            const servoX = document.getElementById('servo-x');
            const servoY = document.getElementById('servo-y');
            
            if (servoX) servoX.textContent = data.servo.x.toFixed(0);
            if (servoY) servoY.textContent = data.servo.y.toFixed(0);
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
            'temperature-status': data.temperature || 'Unknown',
            'uptime-status': data.uptime || 'Unknown'
        };
        
        Object.entries(elements).forEach(([id, value]) => {
            const element = document.getElementById(id);
            if (element) element.textContent = value;
        });
    }
    
    function updateGyroVisualization(gyro) {
        const indicator = document.getElementById('gyro-indicator');
        if (indicator) {
            // Simple visualization - rotate based on gyro values
            const rotation = Math.atan2(gyro.y, gyro.x) * (180 / Math.PI);
            indicator.style.transform = `rotate(${rotation}deg)`;
        }
    }
    
    // Storage information display
    function updateStorageInfo(data) {
        // Format sizes in a human-readable format
        function formatSize(bytes) {
            if (bytes < 1024) {
                return bytes + ' B';
            } else if (bytes < 1024 * 1024) {
                return (bytes / 1024).toFixed(1) + ' KB';
            } else {
                return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
            }
        }
        
        // Update storage information values
        const elements = {
            'storage-total': formatSize(data.total),
            'storage-used': formatSize(data.used),
            'storage-free': formatSize(data.free),
            'storage-percent': data.percent.toFixed(1) + '%'
        };
        
        Object.entries(elements).forEach(([id, value]) => {
            const element = document.getElementById(id);
            if (element) element.textContent = value;
        });
        
        // Update progress bar
        const progressBar = document.querySelector('#files-section .progress div');
        if (progressBar) {
            progressBar.style.width = data.percent.toFixed(1) + '%';
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
            const fileName = file.name.replace(/\/$/, ''); // Remove trailing slash
            
            fileItem.innerHTML = `
                <i class="file-icon ${icon}"></i>
                <div class="file-name">${fileName}</div>
                <div class="file-size pe-2">${file.size || ''}</div>
                <div class="file-actions">
                    ${isDirectory ? 
                        `<button class="btn btn-sm file-open" data-path="${file.path}">Open</button>` :
                        `<button class="btn btn-sm file-download" data-path="${file.path}">Download</button>`
                    }
                    <button class="btn btn-sm btn-danger file-delete" data-path="${file.path}">Delete</button>
                </div>
            `;
            
            // Add event listeners
            const openBtn = fileItem.querySelector('.file-open');
            if (openBtn) {
                openBtn.addEventListener('click', () => {
                    fetchFiles(file.path);
                });
            }
            
            const downloadBtn = fileItem.querySelector('.file-download');
            if (downloadBtn) {
                downloadBtn.addEventListener('click', () => {
                    // Create download link
                    const link = document.createElement('a');
                    link.href = `/download?path=${encodeURIComponent(file.path)}`;
                    link.download = fileName;
                    link.click();
                });
            }
            
            const deleteBtn = fileItem.querySelector('.file-delete');
            if (deleteBtn) {
                deleteBtn.addEventListener('click', () => {
                    if (confirm(`Delete ${fileName}?`)) {
                        sendJsonMessage('delete_file', { path: fileName });
                        logToConsole(`Deleting ${fileName}...`, 'info');
                        // Update storage info after deleting a file
                        setTimeout(() => {
                            fetchStorageInfo(currentPath);
                        }, 1000);
                    }
                });
            }
            
            fileList.appendChild(fileItem);
        });
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
        formData.append('path', currentPath);
        
        fetch('/upload', {
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
            fetchFiles(currentPath || '/');
            fetchStorageInfo();
        })
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
    
    // Initialize
    initJoysticks();
    
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
    
    // Add logout functionality
    const logoutBtn = document.getElementById('logout-btn');
    if (logoutBtn) {
        logoutBtn.addEventListener('click', logout);
    }
});
