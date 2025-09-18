/**
 * Cozmo Robot Control - Main Application
 * Initializes and coordinates all modules
 */

class CozmoApp {
    constructor() {
        this.core = null;
        this.ui = null;
        this.sensors = null;
        this.isInitialized = false;
        
        // Wait for DOM to be ready
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', () => this.init());
        } else {
            this.init();
        }
    }

    /**
     * Initialize the application
     */
    async init() {
        try {
            console.log('Initializing Cozmo Robot Control App...');
            
            // Initialize core system
            this.core = new CozmoCore();
            
            // Wait a moment for core to establish connections
            await this.waitForCoreReady();
            
            // Initialize UI system
            this.ui = new CozmoUI(this.core);
            
            // Initialize sensor visualization
            this.sensors = new CozmoSensors(this.core);
            
            // Setup inter-module communication
            this.setupModuleCommunication();
            
            // Mark as initialized
            this.isInitialized = true;
            
            console.log('Cozmo Robot Control App initialized successfully');
            
            // Start periodic tasks
            this.startPeriodicTasks();
            
            // Setup error handling
            this.setupErrorHandling();
            
        } catch (error) {
            console.error('Failed to initialize Cozmo App:', error);
            this.handleInitializationError(error);
        }
    }

    /**
     * Wait for core system to be ready
     */
    async waitForCoreReady() {
        return new Promise((resolve) => {
            if (this.core.isConnected()) {
                resolve();
            } else {
                const checkConnection = () => {
                    if (this.core.isConnected()) {
                        resolve();
                    } else {
                        setTimeout(checkConnection, 100);
                    }
                };
                
                // Don't wait forever
                setTimeout(() => {
                    console.log('Core connection timeout, continuing anyway...');
                    resolve();
                }, 5000);
                
                checkConnection();
            }
        });
    }

    /**
     * Setup communication between modules
     */
    setupModuleCommunication() {
        // Core events that UI should handle
        this.core.on('websocket:connected', () => {
            console.log('Connected to Cozmo robot');
            this.requestInitialData();
        });

        this.core.on('websocket:disconnected', () => {
            console.log('Disconnected from Cozmo robot');
        });

        this.core.on('websocket:failed', () => {
            console.error('Failed to connect to Cozmo robot');
            this.ui?.showNotification('Failed to connect to robot', 'danger');
        });

        // Handle emergency situations
        this.core.on('sensors:update', (data) => {
            this.handleSensorEmergency(data);
        });

        // Voice command handling
        this.core.on('voice:command', (data) => {
            this.handleVoiceCommand(data);
        });

        // System alerts
        this.core.on('system:alert', (data) => {
            this.handleSystemAlert(data);
        });
    }

    /**
     * Request initial data when connected
     */
    async requestInitialData() {
        try {
            // Get system status
            const systemStatus = await this.core.getSystemStatus();
            if (systemStatus.success) {
                this.core.emit('system:status', systemStatus.data);
            }

            // Get WiFi status
            const wifiStatus = await this.core.getWiFiStatus();
            if (wifiStatus.success) {
                this.ui?.updateWiFiStatus(wifiStatus.data);
            }

        } catch (error) {
            console.error('Failed to get initial data:', error);
        }
    }

    /**
     * Handle sensor-based emergency situations
     */
    handleSensorEmergency(data) {
        // Cliff detection
        if (data.cliff && data.cliff.detected) {
            console.warn('Cliff detected! Stopping robot.');
            this.core.emergencyStop();
            this.ui?.showNotification('Cliff detected! Robot stopped.', 'danger');
        }

        // Obstacle detection (very close distance)
        if (data.distance !== undefined && data.distance < 5) {
            console.warn('Obstacle too close! Stopping robot.');
            this.core.stopMotor();
            this.ui?.showNotification('Obstacle detected! Motors stopped.', 'warning');
        }

        // Battery low
        if (data.battery && data.battery < 10) {
            console.warn('Battery critically low!');
            this.ui?.showNotification('Battery critically low!', 'danger');
        }

        // Temperature high
        if (data.temperature && data.temperature > 70) {
            console.warn('Temperature too high!');
            this.ui?.showNotification('Temperature warning!', 'warning');
        }
    }

    /**
     * Handle voice commands
     */
    handleVoiceCommand(data) {
        console.log('Voice command received:', data);
        
        const command = data.command?.toLowerCase();
        
        switch (command) {
            case 'stop':
            case 'halt':
                this.core.emergencyStop();
                break;
                
            case 'forward':
                this.core.moveMotor('forward', 50);
                break;
                
            case 'backward':
                this.core.moveMotor('backward', 50);
                break;
                
            case 'left':
                this.core.moveMotor('left', 50);
                break;
                
            case 'right':
                this.core.moveMotor('right', 50);
                break;
                
            case 'center':
                this.core.centerServos();
                break;
                
            case 'look left':
                this.core.setServoPosition('x', 45);
                break;
                
            case 'look right':
                this.core.setServoPosition('x', 135);
                break;
                
            case 'look up':
                this.core.setServoPosition('y', 45);
                break;
                
            case 'look down':
                this.core.setServoPosition('y', 135);
                break;
                
            default:
                console.log('Unknown voice command:', command);
        }
        
        // Show feedback in UI
        this.ui?.showNotification(`Voice command: ${command}`, 'info');
    }

    /**
     * Handle system alerts
     */
    handleSystemAlert(data) {
        console.log('System alert:', data);
        
        const alertType = data.level || 'info';
        const message = data.message || 'System alert';
        
        this.ui?.showNotification(message, alertType);
        
        // Handle critical alerts
        if (data.level === 'critical') {
            this.core.emergencyStop();
        }
    }

    /**
     * Start periodic background tasks
     */
    startPeriodicTasks() {
        // Periodic system status check (every 30 seconds)
        setInterval(() => {
            if (this.core.isConnected()) {
                this.core.getSystemStatus()
                    .then(data => {
                        if (data.success) {
                            this.core.emit('system:status', data.data);
                        }
                    })
                    .catch(error => {
                        console.error('Periodic status check failed:', error);
                    });
            }
        }, 30000);

        // Connection health check (every 10 seconds)
        setInterval(() => {
            if (!this.core.isConnected()) {
                console.log('Connection lost, attempting to reconnect...');
                // Core will handle reconnection automatically
            }
        }, 10000);

        // Memory cleanup (every 5 minutes)
        setInterval(() => {
            this.performMemoryCleanup();
        }, 300000);
    }

    /**
     * Perform memory cleanup
     */
    performMemoryCleanup() {
        // Clear old sensor data
        if (this.sensors) {
            // Sensor module handles its own cleanup
        }

        // Clear old chat messages (keep last 50)
        const chatMessages = document.getElementById('chat-messages');
        if (chatMessages && chatMessages.children.length > 50) {
            while (chatMessages.children.length > 50) {
                chatMessages.removeChild(chatMessages.firstChild);
            }
        }

        // Trigger garbage collection if available
        if (window.gc) {
            window.gc();
        }

        console.log('Memory cleanup performed');
    }

    /**
     * Setup global error handling
     */
    setupErrorHandling() {
        // Handle unhandled promise rejections
        window.addEventListener('unhandledrejection', (event) => {
            console.error('Unhandled promise rejection:', event.reason);
            this.ui?.showNotification('An unexpected error occurred', 'danger');
            event.preventDefault();
        });

        // Handle JavaScript errors
        window.addEventListener('error', (event) => {
            console.error('JavaScript error:', event.error);
            this.ui?.showNotification('An error occurred in the application', 'warning');
        });

        // Handle WebSocket errors
        this.core.on('websocket:error', (error) => {
            console.error('WebSocket error:', error);
            this.ui?.showNotification('Communication error with robot', 'warning');
        });

        // Handle API errors
        this.core.on('api:error', (data) => {
            console.error('API error:', data);
            this.ui?.showNotification(`API error: ${data.endpoint}`, 'warning');
        });
    }

    /**
     * Handle initialization errors
     */
    handleInitializationError(error) {
        // Show error message
        const errorMessage = document.createElement('div');
        errorMessage.className = 'alert alert-danger position-fixed';
        errorMessage.style.cssText = 'top: 50%; left: 50%; transform: translate(-50%, -50%); z-index: 10000; max-width: 400px;';
        errorMessage.innerHTML = `
            <h5>Initialization Error</h5>
            <p>Failed to initialize the Cozmo control interface.</p>
            <p><small>${error.message}</small></p>
            <button class="btn btn-outline-danger" onclick="location.reload()">Retry</button>
        `;
        
        document.body.appendChild(errorMessage);

        // Hide loading screen
        const loadingScreen = document.getElementById('loading-screen');
        if (loadingScreen) {
            loadingScreen.style.display = 'none';
        }
    }

    /**
     * Shutdown the application gracefully
     */
    shutdown() {
        console.log('Shutting down Cozmo Robot Control App...');
        
        // Stop all motors
        if (this.core) {
            this.core.emergencyStop().catch(() => {});
        }

        // Cleanup modules
        if (this.sensors) {
            this.sensors.destroy();
        }

        if (this.ui) {
            this.ui.cleanup();
        }

        // Close WebSocket
        if (this.core && this.core.ws) {
            this.core.ws.close();
        }

        this.isInitialized = false;
        console.log('Application shutdown complete');
    }

    /**
     * Get application status
     */
    getStatus() {
        return {
            initialized: this.isInitialized,
            connected: this.core?.isConnected() || false,
            modules: {
                core: !!this.core,
                ui: !!this.ui,
                sensors: !!this.sensors
            }
        };
    }
}

// Initialize the application
const app = new CozmoApp();

// Make app globally available for debugging
window.CozmoApp = app;

// Handle page unload
window.addEventListener('beforeunload', () => {
    app.shutdown();
});

// Export for module systems
if (typeof module !== 'undefined' && module.exports) {
    module.exports = CozmoApp;
}
