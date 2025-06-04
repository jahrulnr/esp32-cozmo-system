// Dashboard specific JavaScript
$(document).ready(function() {
    console.log('Dashboard script loaded');
    
    // WebSocket connection
    let socket;
    
    // Initialize WebSocket connection
    function connectWebSocket() {
        // Get the secure protocol based on the current page protocol
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const token = localStorage.getItem('auth_token');
        
        // Connect to the WebSocket server with the auth token
        socket = new WebSocket(`${protocol}//${window.location.host}/ws/browser?token=${token}`);
        
        socket.onopen = function() {
            console.log('WebSocket connection established');
        };
        
        socket.onmessage = function(event) {
            try {
                const data = JSON.parse(event.data);
                
                // Handle different message types
                switch (data.type) {
                    case 'robot_status':
                        updateRobotStatus(data.data);
                        break;
                        
                    case 'sensor_data':
                        updateSensorData(data.data);
                        break;
                        
                    default:
                        console.warn('Unknown WebSocket message type:', data.type);
                }
            } catch (error) {
                console.error('Error parsing WebSocket message:', error);
            }
        };
        
        socket.onclose = function(event) {
            if (event.wasClean) {
                console.log('WebSocket connection closed cleanly');
                // Update UI to show disconnected status
                updateConnectionStatus(false);
            } else {
                console.error('WebSocket connection abruptly closed');
                // Try to reconnect after a delay
                setTimeout(connectWebSocket, 3000);
                // Update UI to show disconnected status
                updateConnectionStatus(false);
            }
        };
        
        socket.onerror = function(error) {
            console.error('WebSocket error:', error);
        };
    }
    
    // Connect to WebSocket when the page loads
    connectWebSocket();
    
    // Function to update connection status in UI
    function updateConnectionStatus(isConnected) {
        const $status = $('#cozmo-status .status-value');
        const $connectButton = $('#connect-cozmo');
        
        if (isConnected) {
            $status.removeClass('disconnected').addClass('connected').text('Connected');
            $connectButton.text('Disconnect');
        } else {
            $status.removeClass('connected').addClass('disconnected').text('Disconnected');
            $connectButton.text('Connect');
        }
    }
    
    // Connect/disconnect button handler
    $('#connect-cozmo').on('click', function() {
        const isConnected = $('#cozmo-status .status-value').hasClass('connected');
        
        if (isConnected) {
            // Send disconnect command
            if (socket && socket.readyState === WebSocket.OPEN) {
                socket.send(JSON.stringify({
                    type: 'robot_command',
                    data: {
                        command: 'disconnect'
                    }
                }));
            }
            updateConnectionStatus(false);
        } else {
            // Send connect command
            if (socket && socket.readyState === WebSocket.OPEN) {
                socket.send(JSON.stringify({
                    type: 'robot_command',
                    data: {
                        command: 'connect'
                    }
                }));
                updateConnectionStatus(true);
            } else {
                console.error('WebSocket connection not available');
                alert('Connection lost. Please refresh the page.');
            }
        }
    });

    // Initialize dashboard items with placeholder data
    $('#battery-level').text('87%');
    $('#temperature').text('23°C');
    $('#gyro').text('Stable');
    $('#cliff-sensor').text('Clear');
    
    // Function to update sensor data in UI
    function updateSensorData(data) {
        if (data.batteryLevel !== undefined) {
            $('#battery-level').text(data.batteryLevel + '%');
        }
        
        if (data.temperature !== undefined) {
            $('#temperature').text(data.temperature + '°C');
        }
        
        if (data.gyro !== undefined) {
            $('#gyro').text(data.gyro);
        }
        
        if (data.cliffSensor !== undefined) {
            $('#cliff-sensor').text(data.cliffSensor);
        }
    }
    
    // Function to handle movement commands
    function sendMovementCommand(command) {
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify({
                type: 'robot_command',
                data: {
                    command: 'move',
                    direction: command
                }
            }));
        } else {
            console.error('WebSocket connection not available');
            alert('Connection lost. Please refresh the page.');
        }
    }
    
    // Set up movement control buttons
    $('#move-forward').on('click', function() {
        sendMovementCommand('forward');
    });
    
    $('#move-backward').on('click', function() {
        sendMovementCommand('backward');
    });
    
    $('#turn-left').on('click', function() {
        sendMovementCommand('left');
    });
    
    $('#turn-right').on('click', function() {
        sendMovementCommand('right');
    });
    
    $('#stop').on('click', function() {
        sendMovementCommand('stop');
    });
    
    // Head and lift controls
    $('#head-up').on('click', function() {
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify({
                type: 'robot_command',
                data: {
                    command: 'head',
                    direction: 'up'
                }
            }));
        }
    });
    
    $('#head-down').on('click', function() {
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify({
                type: 'robot_command',
                data: {
                    command: 'head',
                    direction: 'down'
                }
            }));
        }
    });
    
    $('#lift-up').on('click', function() {
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify({
                type: 'robot_command',
                data: {
                    command: 'lift',
                    direction: 'up'
                }
            }));
        }
    });
    
    $('#lift-down').on('click', function() {
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify({
                type: 'robot_command',
                data: {
                    command: 'lift',
                    direction: 'down'
                }
            }));
        }
    });
});
