/**
 * Cozmo Robot Control - UI Module
 * Handles user interface interactions, navigation, and control bindings
 */

class CozmoUI {
    constructor(core) {
        this.core = core;
        this.currentView = 'dashboard';
        this.joystick = null;
        this.isVoiceActive = false;
        this.isSidebarOpen = false;
        
        // UI element references
        this.elements = {};
        
        this.init();
    }

    /**
     * Initialize UI system
     */
    init() {
        this.cacheElements();
        this.setupEventListeners();
        this.setupNavigation();
        this.setupControls();
        this.setupConnectionStatus();
        this.hideLoadingScreen();
    }

    /**
     * Cache frequently used DOM elements
     */
    cacheElements() {
        this.elements = {
            // Navigation
            sidebar: document.getElementById('sidebar'),
            sidebarToggle: document.getElementById('sidebar-toggle'),
            sidebarClose: document.getElementById('sidebar-close'),
            sidebarOverlay: document.getElementById('sidebar-overlay'),
            navLinks: document.querySelectorAll('[data-view]'),
            
            // Views
            views: document.querySelectorAll('.view-content'),
            
            // Controls
            emergencyStop: document.getElementById('emergency-stop'),
            voiceControl: document.getElementById('voice-control'),
            cameraToggle: document.getElementById('camera-toggle'),
            servoX: document.getElementById('servo-x'),
            servoY: document.getElementById('servo-y'),
            servoXValue: document.getElementById('servo-x-value'),
            servoYValue: document.getElementById('servo-y-value'),
            servoCenter: document.getElementById('servo-center'),
            
            // Status
            connectionStatus: document.getElementById('connection-status'),
            statusDot: document.querySelector('.status-dot'),
            
            // Chat
            chatMessages: document.getElementById('chat-messages'),
            chatInput: document.getElementById('chat-input'),
            sendMessage: document.getElementById('send-message'),
            
            // WiFi
            refreshWifi: document.getElementById('refresh-wifi'),
            scanNetworks: document.getElementById('scan-networks'),
            wifiNetworks: document.getElementById('wifi-networks'),
            
            // Camera
            cameraCanvas: document.getElementById('camera-canvas'),
            cameraOverlay: document.getElementById('camera-overlay'),
            
            // App container
            app: document.getElementById('app'),
            loadingScreen: document.getElementById('loading-screen')
        };
    }

    /**
     * Setup event listeners
     */
    setupEventListeners() {
        // Core events
        this.core.on('websocket:connected', () => this.updateConnectionStatus('online'));
        this.core.on('websocket:disconnected', () => this.updateConnectionStatus('offline'));
        this.core.on('websocket:reconnecting', () => this.updateConnectionStatus('connecting'));
        this.core.on('chat:response', (data) => this.addChatMessage(data.message, 'bot'));
        this.core.on('camera:frame', (data) => this.updateCameraFrame(data));
        
        // Emergency stop
        if (this.elements.emergencyStop) {
            this.elements.emergencyStop.addEventListener('click', () => {
                this.handleEmergencyStop();
            });
        }
        
        // Voice control
        if (this.elements.voiceControl) {
            this.elements.voiceControl.addEventListener('click', () => {
                this.toggleVoiceControl();
            });
        }
        
        // Camera toggle
        if (this.elements.cameraToggle) {
            this.elements.cameraToggle.addEventListener('click', () => {
                this.toggleCamera();
            });
        }
        
        // Servo controls
        this.setupServoControls();
        
        // Chat
        this.setupChat();
        
        // WiFi
        this.setupWiFiControls();
        
        // Keyboard shortcuts
        this.setupKeyboardShortcuts();
        
        // Window events
        window.addEventListener('beforeunload', () => {
            this.cleanup();
        });
    }

    /**
     * Setup navigation system
     */
    setupNavigation() {
        // Sidebar toggle (mobile)
        if (this.elements.sidebarToggle) {
            this.elements.sidebarToggle.addEventListener('click', () => {
                this.toggleSidebar();
            });
        }
        
        // Sidebar close
        if (this.elements.sidebarClose) {
            this.elements.sidebarClose.addEventListener('click', () => {
                this.closeSidebar();
            });
        }
        
        // Sidebar overlay
        if (this.elements.sidebarOverlay) {
            this.elements.sidebarOverlay.addEventListener('click', () => {
                this.closeSidebar();
            });
        }
        
        // Navigation links
        this.elements.navLinks.forEach(link => {
            link.addEventListener('click', (e) => {
                e.preventDefault();
                const view = link.getAttribute('data-view');
                this.switchView(view);
                
                // Close sidebar on mobile after navigation
                if (window.innerWidth < 768) {
                    this.closeSidebar();
                }
            });
        });
    }

    /**
     * Setup motor and servo controls
     */
    setupControls() {
        // Initialize joystick for motor control
        this.setupJoystick();
    }

    /**
     * Setup joystick for motor control
     */
    setupJoystick() {
        const joystickElement = document.getElementById('motor-joystick');
        if (joystickElement && window.Joy) {
            this.joystick = new Joy(joystickElement, {
                title: 'Motor Control',
                width: 150,
                height: 150,
                internalFillColor: '#0d6efd',
                internalLineWidth: 2,
                internalStrokeColor: '#ffffff',
                externalLineWidth: 2,
                externalStrokeColor: '#ffffff',
                autoReturnToCenter: true
            });
            
            // Handle joystick movement
            let lastUpdate = 0;
            this.joystick.on('moved', (data) => {
                // Throttle updates to avoid overwhelming the ESP32
                const now = Date.now();
                if (now - lastUpdate < 100) return; // 10Hz max
                lastUpdate = now;
                
                const x = data.x; // -50 to 50
                const y = data.y; // -50 to 50
                
                // Convert to motor speeds (-100 to 100)
                const leftSpeed = Math.round((y + x) * 2);
                const rightSpeed = Math.round((y - x) * 2);
                
                // Clamp values
                const clampedLeft = Math.max(-100, Math.min(100, leftSpeed));
                const clampedRight = Math.max(-100, Math.min(100, rightSpeed));
                
                this.core.setMotorSpeed(clampedLeft, clampedRight)
                    .catch(error => console.error('Motor control error:', error));
            });
            
            this.joystick.on('released', () => {
                this.core.stopMotor()
                    .catch(error => console.error('Motor stop error:', error));
            });
        }
    }

    /**
     * Setup servo controls
     */
    setupServoControls() {
        // Servo X control
        if (this.elements.servoX) {
            this.elements.servoX.addEventListener('input', (e) => {
                const angle = parseInt(e.target.value);
                this.elements.servoXValue.textContent = `${angle}°`;
                
                // Debounced servo update
                this.debouncedServoUpdate('x', angle);
            });
        }
        
        // Servo Y control
        if (this.elements.servoY) {
            this.elements.servoY.addEventListener('input', (e) => {
                const angle = parseInt(e.target.value);
                this.elements.servoYValue.textContent = `${angle}°`;
                
                // Debounced servo update
                this.debouncedServoUpdate('y', angle);
            });
        }
        
        // Center servos button
        if (this.elements.servoCenter) {
            this.elements.servoCenter.addEventListener('click', () => {
                this.centerServos();
            });
        }
        
        // Create debounced servo update function
        this.debouncedServoUpdate = this.debounce((servo, angle) => {
            this.core.setServoPosition(servo, angle)
                .catch(error => console.error('Servo control error:', error));
        }, 200);
    }

    /**
     * Setup chat functionality
     */
    setupChat() {
        if (this.elements.chatInput && this.elements.sendMessage) {
            const sendMessage = () => {
                const message = this.elements.chatInput.value.trim();
                if (message) {
                    this.addChatMessage(message, 'user');
                    this.elements.chatInput.value = '';
                    
                    // Send to backend
                    this.core.sendChatMessage(message)
                        .catch(error => {
                            console.error('Chat error:', error);
                            this.addChatMessage('Sorry, I\'m having trouble connecting right now.', 'bot');
                        });
                }
            };
            
            this.elements.sendMessage.addEventListener('click', sendMessage);
            this.elements.chatInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    e.preventDefault();
                    sendMessage();
                }
            });
        }
    }

    /**
     * Setup WiFi controls
     */
    setupWiFiControls() {
        if (this.elements.refreshWifi) {
            this.elements.refreshWifi.addEventListener('click', () => {
                this.refreshWiFiStatus();
            });
        }
        
        if (this.elements.scanNetworks) {
            this.elements.scanNetworks.addEventListener('click', () => {
                this.scanWiFiNetworks();
            });
        }
    }

    /**
     * Setup keyboard shortcuts
     */
    setupKeyboardShortcuts() {
        document.addEventListener('keydown', (e) => {
            // Emergency stop: Spacebar
            if (e.code === 'Space' && !e.target.matches('input, textarea')) {
                e.preventDefault();
                this.handleEmergencyStop();
            }
            
            // Voice control: V key
            if (e.code === 'KeyV' && !e.target.matches('input, textarea')) {
                e.preventDefault();
                this.toggleVoiceControl();
            }
            
            // Navigation shortcuts
            if (e.altKey && !e.target.matches('input, textarea')) {
                switch (e.code) {
                    case 'Digit1':
                        e.preventDefault();
                        this.switchView('dashboard');
                        break;
                    case 'Digit2':
                        e.preventDefault();
                        this.switchView('sensors');
                        break;
                    case 'Digit3':
                        e.preventDefault();
                        this.switchView('wifi');
                        break;
                    case 'Digit4':
                        e.preventDefault();
                        this.switchView('chat');
                        break;
                }
            }
        });
    }

    /**
     * Switch between views
     */
    switchView(viewName) {
        // Update navigation
        this.elements.navLinks.forEach(link => {
            if (link.getAttribute('data-view') === viewName) {
                link.classList.add('active');
            } else {
                link.classList.remove('active');
            }
        });
        
        // Update views
        this.elements.views.forEach(view => {
            if (view.id === `${viewName}-view`) {
                view.classList.add('active');
            } else {
                view.classList.remove('active');
            }
        });
        
        this.currentView = viewName;
        
        // Trigger view-specific initialization
        this.onViewChanged(viewName);
    }

    /**
     * Handle view change events
     */
    onViewChanged(viewName) {
        switch (viewName) {
            case 'dashboard':
                this.requestSystemStatus();
                break;
            case 'sensors':
                // Sensor charts are already running
                break;
            case 'wifi':
                this.refreshWiFiStatus();
                break;
            case 'chat':
                this.elements.chatInput?.focus();
                break;
        }
    }

    /**
     * Toggle sidebar (mobile)
     */
    toggleSidebar() {
        this.isSidebarOpen = !this.isSidebarOpen;
        
        if (this.isSidebarOpen) {
            this.elements.sidebar?.classList.add('show');
            this.elements.sidebarOverlay?.classList.add('show');
        } else {
            this.elements.sidebar?.classList.remove('show');
            this.elements.sidebarOverlay?.classList.remove('show');
        }
    }

    /**
     * Close sidebar
     */
    closeSidebar() {
        this.isSidebarOpen = false;
        this.elements.sidebar?.classList.remove('show');
        this.elements.sidebarOverlay?.classList.remove('show');
    }

    /**
     * Update connection status indicator
     */
    updateConnectionStatus(status) {
        if (this.elements.statusDot) {
            this.elements.statusDot.className = `status-dot ${status}`;
        }
    }

    /**
     * Setup connection status monitoring
     */
    setupConnectionStatus() {
        // Initial status
        this.updateConnectionStatus('offline');
        
        // Request initial system status
        setTimeout(() => {
            this.requestSystemStatus();
        }, 1000);
    }

    /**
     * Request system status
     */
    requestSystemStatus() {
        this.core.getSystemStatus()
            .then(data => {
                if (data.success) {
                    this.core.emit('system:status', data.data);
                }
            })
            .catch(error => {
                console.error('Failed to get system status:', error);
            });
    }

    /**
     * Handle emergency stop
     */
    handleEmergencyStop() {
        // Visual feedback
        this.elements.emergencyStop?.classList.add('btn-outline-danger');
        setTimeout(() => {
            this.elements.emergencyStop?.classList.remove('btn-outline-danger');
        }, 1000);
        
        // Send emergency stop command
        this.core.emergencyStop()
            .then(() => {
                this.showNotification('Emergency stop activated', 'warning');
            })
            .catch(error => {
                console.error('Emergency stop failed:', error);
                this.showNotification('Emergency stop failed!', 'danger');
            });
    }

    /**
     * Toggle voice control
     */
    toggleVoiceControl() {
        this.core.toggleVoiceControl()
            .then(data => {
                this.isVoiceActive = data.active;
                this.updateVoiceControlUI();
            })
            .catch(error => {
                console.error('Voice control toggle failed:', error);
                this.showNotification('Voice control error', 'danger');
            });
    }

    /**
     * Update voice control UI
     */
    updateVoiceControlUI() {
        if (this.elements.voiceControl) {
            if (this.isVoiceActive) {
                this.elements.voiceControl.classList.add('active');
                this.elements.voiceControl.innerHTML = '<i class="bi bi-mic-fill"></i> Listening...';
            } else {
                this.elements.voiceControl.classList.remove('active');
                this.elements.voiceControl.innerHTML = '<i class="bi bi-mic"></i> Voice Control';
            }
        }
    }

    /**
     * Toggle camera feed
     */
    toggleCamera() {
        // Implementation depends on camera API
        console.log('Camera toggle requested');
    }

    /**
     * Center servos to default position
     */
    centerServos() {
        this.core.centerServos()
            .then(() => {
                // Update UI
                if (this.elements.servoX) this.elements.servoX.value = 90;
                if (this.elements.servoY) this.elements.servoY.value = 90;
                if (this.elements.servoXValue) this.elements.servoXValue.textContent = '90°';
                if (this.elements.servoYValue) this.elements.servoYValue.textContent = '90°';
                
                this.showNotification('Servos centered', 'success');
            })
            .catch(error => {
                console.error('Servo center failed:', error);
                this.showNotification('Servo center failed', 'danger');
            });
    }

    /**
     * Add chat message
     */
    addChatMessage(message, sender) {
        if (!this.elements.chatMessages) return;
        
        const messageDiv = document.createElement('div');
        messageDiv.className = `chat-message ${sender}`;
        
        const contentDiv = document.createElement('div');
        contentDiv.className = 'message-content';
        contentDiv.textContent = message;
        
        const timeDiv = document.createElement('div');
        timeDiv.className = 'message-time';
        timeDiv.textContent = new Date().toLocaleTimeString();
        
        messageDiv.appendChild(contentDiv);
        messageDiv.appendChild(timeDiv);
        
        this.elements.chatMessages.appendChild(messageDiv);
        this.elements.chatMessages.scrollTop = this.elements.chatMessages.scrollHeight;
    }

    /**
     * Refresh WiFi status
     */
    refreshWiFiStatus() {
        this.core.getWiFiStatus()
            .then(data => {
                if (data.success) {
                    this.updateWiFiStatus(data.data);
                }
            })
            .catch(error => {
                console.error('WiFi status refresh failed:', error);
            });
    }

    /**
     * Scan WiFi networks
     */
    scanWiFiNetworks() {
        if (this.elements.scanNetworks) {
            this.elements.scanNetworks.innerHTML = '<i class="bi bi-arrow-clockwise spinning"></i> Scanning...';
            this.elements.scanNetworks.disabled = true;
        }
        
        this.core.scanWiFiNetworks()
            .then(data => {
                if (data.success) {
                    this.displayWiFiNetworks(data.data.networks);
                }
            })
            .catch(error => {
                console.error('WiFi scan failed:', error);
            })
            .finally(() => {
                if (this.elements.scanNetworks) {
                    this.elements.scanNetworks.innerHTML = '<i class="bi bi-arrow-clockwise"></i> Scan Networks';
                    this.elements.scanNetworks.disabled = false;
                }
            });
    }

    /**
     * Update WiFi status display
     */
    updateWiFiStatus(data) {
        if (data.ssid) {
            const element = document.getElementById('current-ssid');
            if (element) element.textContent = data.ssid;
        }
        
        if (data.rssi) {
            const element = document.getElementById('signal-strength');
            if (element) element.textContent = `${data.rssi} dBm`;
        }
        
        if (data.ip) {
            const element = document.getElementById('ip-address');
            if (element) element.textContent = data.ip;
        }
    }

    /**
     * Display WiFi networks
     */
    displayWiFiNetworks(networks) {
        if (!this.elements.wifiNetworks) return;
        
        this.elements.wifiNetworks.innerHTML = '';
        
        networks.forEach(network => {
            const networkDiv = document.createElement('div');
            networkDiv.className = 'wifi-network-item';
            
            const nameSpan = document.createElement('span');
            nameSpan.textContent = network.ssid;
            
            const signalSpan = document.createElement('span');
            signalSpan.className = 'wifi-signal';
            signalSpan.textContent = `${network.rssi} dBm`;
            
            networkDiv.appendChild(nameSpan);
            networkDiv.appendChild(signalSpan);
            
            networkDiv.addEventListener('click', () => {
                this.connectToNetwork(network);
            });
            
            this.elements.wifiNetworks.appendChild(networkDiv);
        });
    }

    /**
     * Connect to WiFi network
     */
    connectToNetwork(network) {
        // This would typically show a password dialog
        console.log('Connect to network:', network.ssid);
    }

    /**
     * Update camera frame
     */
    updateCameraFrame(data) {
        // Implementation depends on camera data format
        // Could be base64 image or stream URL
        console.log('Camera frame update:', data);
    }

    /**
     * Show notification
     */
    showNotification(message, type = 'info') {
        // Simple toast notification
        const notification = document.createElement('div');
        notification.className = `alert alert-${type} position-fixed`;
        notification.style.cssText = 'top: 20px; right: 20px; z-index: 9999; min-width: 250px;';
        notification.textContent = message;
        
        document.body.appendChild(notification);
        
        // Auto-remove after 3 seconds
        setTimeout(() => {
            notification.remove();
        }, 3000);
    }

    /**
     * Hide loading screen
     */
    hideLoadingScreen() {
        setTimeout(() => {
            if (this.elements.loadingScreen) {
                this.elements.loadingScreen.classList.add('fade-out');
                setTimeout(() => {
                    this.elements.loadingScreen.style.display = 'none';
                    if (this.elements.app) {
                        this.elements.app.classList.remove('d-none');
                    }
                }, 500);
            }
        }, 1500); // Show loading for at least 1.5 seconds
    }

    /**
     * Utility: Debounce function
     */
    debounce(func, wait) {
        let timeout;
        return function executedFunction(...args) {
            const later = () => {
                clearTimeout(timeout);
                func(...args);
            };
            clearTimeout(timeout);
            timeout = setTimeout(later, wait);
        };
    }

    /**
     * Cleanup when page unloads
     */
    cleanup() {
        if (this.joystick) {
            this.joystick.destroy();
        }
    }
}

// Export for use in main app
window.CozmoUI = CozmoUI;
