/**
 * Cozmo Robot Control - Core Module
 * Handles API communication, WebSocket connections, and core functionality
 */

class CozmoCore {
    constructor() {
        this.apiBase = '/api/v1';
        this.ws = null;
        this.wsReconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        this.reconnectDelay = 1000;
        this.connectionStatus = 'offline';
        this.eventListeners = new Map();
        this.requestQueue = [];
        this.isOnline = false;
        
        // Initialize core
        this.init();
    }

    /**
     * Initialize the core system
     */
    init() {
        this.setupWebSocket();
        this.setupEventSystem();
        this.startHeartbeat();
        
        // Check online status
        window.addEventListener('online', () => {
            this.isOnline = true;
            this.emit('connection:online');
            this.setupWebSocket();
        });
        
        window.addEventListener('offline', () => {
            this.isOnline = false;
            this.emit('connection:offline');
        });
    }

    /**
     * Setup WebSocket connection
     */
    setupWebSocket() {
        if (!this.isOnline && navigator.onLine === false) return;
        
        try {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            const wsUrl = `${protocol}//${window.location.host}/ws`;
            
            this.ws = new WebSocket(wsUrl);
            
            this.ws.onopen = () => {
                console.log('WebSocket connected');
                this.connectionStatus = 'online';
                this.wsReconnectAttempts = 0;
                this.emit('websocket:connected');
                this.processRequestQueue();
            };
            
            this.ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    this.handleWebSocketMessage(data);
                } catch (error) {
                    console.error('Failed to parse WebSocket message:', error);
                }
            };
            
            this.ws.onclose = () => {
                console.log('WebSocket disconnected');
                this.connectionStatus = 'offline';
                this.emit('websocket:disconnected');
                this.handleReconnect();
            };
            
            this.ws.onerror = (error) => {
                console.error('WebSocket error:', error);
                this.emit('websocket:error', error);
            };
            
        } catch (error) {
            console.error('Failed to setup WebSocket:', error);
            this.handleReconnect();
        }
    }

    /**
     * Handle WebSocket reconnection
     */
    handleReconnect() {
        if (this.wsReconnectAttempts < this.maxReconnectAttempts) {
            this.wsReconnectAttempts++;
            this.connectionStatus = 'connecting';
            this.emit('websocket:reconnecting', this.wsReconnectAttempts);
            
            setTimeout(() => {
                this.setupWebSocket();
            }, this.reconnectDelay * this.wsReconnectAttempts);
        } else {
            this.connectionStatus = 'offline';
            this.emit('websocket:failed');
        }
    }

    /**
     * Handle incoming WebSocket messages
     */
    handleWebSocketMessage(data) {
        switch (data.type) {
            case 'sensor_data':
                this.emit('sensors:update', data.payload);
                break;
            case 'system_status':
                this.emit('system:status', data.payload);
                break;
            case 'camera_frame':
                this.emit('camera:frame', data.payload);
                break;
            case 'voice_command':
                this.emit('voice:command', data.payload);
                break;
            case 'motor_status':
                this.emit('motor:status', data.payload);
                break;
            case 'servo_position':
                this.emit('servo:position', data.payload);
                break;
            case 'chat_response':
                this.emit('chat:response', data.payload);
                break;
            default:
                this.emit('websocket:message', data);
        }
    }

    /**
     * Send WebSocket message
     */
    sendWebSocketMessage(type, payload) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            const message = {
                type,
                payload,
                timestamp: Date.now()
            };
            this.ws.send(JSON.stringify(message));
            return true;
        } else {
            console.warn('WebSocket not connected, queuing message');
            this.requestQueue.push({ type, payload });
            return false;
        }
    }

    /**
     * Process queued requests when connection is restored
     */
    processRequestQueue() {
        while (this.requestQueue.length > 0) {
            const request = this.requestQueue.shift();
            this.sendWebSocketMessage(request.type, request.payload);
        }
    }

    /**
     * Make API request
     */
    async apiRequest(endpoint, options = {}) {
        const url = `${this.apiBase}${endpoint}`;
        const defaultOptions = {
            headers: {
                'Content-Type': 'application/json',
            },
        };

        try {
            const response = await fetch(url, { ...defaultOptions, ...options });
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const data = await response.json();
            return data;
        } catch (error) {
            console.error(`API request failed for ${endpoint}:`, error);
            this.emit('api:error', { endpoint, error });
            throw error;
        }
    }

    /**
     * Motor control methods
     */
    async moveMotor(direction, speed = 50) {
        return this.apiRequest('/robot/motor/move', {
            method: 'POST',
            body: JSON.stringify({ direction, speed })
        });
    }

    async stopMotor() {
        return this.apiRequest('/robot/motor/stop', { method: 'POST' });
    }

    async setMotorSpeed(left, right) {
        return this.apiRequest('/robot/motor/speed', {
            method: 'POST',
            body: JSON.stringify({ left, right })
        });
    }

    /**
     * Servo control methods
     */
    async setServoPosition(servo, angle) {
        return this.apiRequest('/robot/servo/position', {
            method: 'POST',
            body: JSON.stringify({ servo, angle })
        });
    }

    async centerServos() {
        return this.apiRequest('/robot/servo/center', { method: 'POST' });
    }

    /**
     * Camera control methods
     */
    async getCameraFrame() {
        return this.apiRequest('/camera/frame');
    }

    async setCameraSettings(settings) {
        return this.apiRequest('/camera/settings', {
            method: 'POST',
            body: JSON.stringify(settings)
        });
    }

    /**
     * System information methods
     */
    async getSystemStatus() {
        return this.apiRequest('/system/stats');
    }

    async getWiFiStatus() {
        return this.apiRequest('/wifi/status');
    }

    async scanWiFiNetworks() {
        return this.apiRequest('/wifi/scan');
    }

    /**
     * Voice and chat methods
     */
    async sendChatMessage(message) {
        return this.apiRequest('/robot/chat/message', {
            method: 'POST',
            body: JSON.stringify({ message })
        });
    }

    async toggleVoiceControl() {
        return this.apiRequest('/robot/voice/toggle', { method: 'POST' });
    }

    /**
     * Emergency stop
     */
    async emergencyStop() {
        // Send both API request and WebSocket message for immediate response
        this.sendWebSocketMessage('emergency_stop', {});
        return this.apiRequest('/robot/emergency/stop', { method: 'POST' });
    }

    /**
     * Event system methods
     */
    setupEventSystem() {
        // Custom event system for loose coupling
    }

    on(event, callback) {
        if (!this.eventListeners.has(event)) {
            this.eventListeners.set(event, []);
        }
        this.eventListeners.get(event).push(callback);
    }

    off(event, callback) {
        if (this.eventListeners.has(event)) {
            const listeners = this.eventListeners.get(event);
            const index = listeners.indexOf(callback);
            if (index > -1) {
                listeners.splice(index, 1);
            }
        }
    }

    emit(event, data) {
        if (this.eventListeners.has(event)) {
            this.eventListeners.get(event).forEach(callback => {
                try {
                    callback(data);
                } catch (error) {
                    console.error(`Error in event listener for ${event}:`, error);
                }
            });
        }
    }

    /**
     * Heartbeat to maintain connection
     */
    startHeartbeat() {
        setInterval(() => {
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                this.sendWebSocketMessage('ping', { timestamp: Date.now() });
            }
        }, 30000); // 30 seconds
    }

    /**
     * Utility methods
     */
    getConnectionStatus() {
        return this.connectionStatus;
    }

    isConnected() {
        return this.connectionStatus === 'online';
    }

    formatTimestamp(timestamp) {
        return new Date(timestamp).toLocaleTimeString();
    }

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

    throttle(func, limit) {
        let inThrottle;
        return function() {
            const args = arguments;
            const context = this;
            if (!inThrottle) {
                func.apply(context, args);
                inThrottle = true;
                setTimeout(() => inThrottle = false, limit);
            }
        };
    }
}

// Create global instance
window.CozmoCore = CozmoCore;

// Export for module systems
if (typeof module !== 'undefined' && module.exports) {
    module.exports = CozmoCore;
}
