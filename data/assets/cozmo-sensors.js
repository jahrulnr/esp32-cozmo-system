/**
 * Cozmo Robot Control - Sensor Visualization Module
 * Handles real-time sensor data visualization with canvas-based charts
 */

class CozmoSensors {
    constructor(core) {
        this.core = core;
        this.canvases = new Map();
        this.contexts = new Map();
        this.sensorData = new Map();
        this.animationFrames = new Map();
        
        // Chart configuration
        this.chartConfig = {
            backgroundColor: 'rgba(13, 110, 253, 0.1)',
            gridColor: 'rgba(255, 255, 255, 0.2)',
            primaryColor: '#0d6efd',
            secondaryColor: '#0dcaf0',
            dangerColor: '#dc3545',
            successColor: '#198754',
            textColor: '#ffffff',
            fontSize: 12,
            lineWidth: 2
        };

        this.init();
    }

    /**
     * Initialize sensor visualization
     */
    init() {
        this.setupCanvases();
        this.setupEventListeners();
        this.startAnimationLoop();
    }

    /**
     * Setup canvas elements
     */
    setupCanvases() {
        const canvasIds = [
            'accelerometer-canvas',
            'gyroscope-canvas',
            'distance-canvas',
            'cliff-canvas'
        ];

        canvasIds.forEach(id => {
            const canvas = document.getElementById(id);
            if (canvas) {
                const ctx = canvas.getContext('2d');
                this.canvases.set(id, canvas);
                this.contexts.set(id, ctx);
                
                // Set canvas size
                this.resizeCanvas(canvas);
                
                // Initialize with empty data
                this.initializeSensorData(id);
            }
        });

        // Handle window resize
        window.addEventListener('resize', this.debounce(() => {
            this.canvases.forEach((canvas, id) => {
                this.resizeCanvas(canvas);
            });
        }, 250));
    }

    /**
     * Resize canvas maintaining aspect ratio
     */
    resizeCanvas(canvas) {
        const container = canvas.parentElement;
        const rect = container.getBoundingClientRect();
        
        // Set display size
        canvas.style.width = '100%';
        canvas.style.height = 'auto';
        
        // Set actual size in memory (scaled up for crisp rendering)
        const scale = window.devicePixelRatio || 1;
        canvas.width = rect.width * scale;
        canvas.height = 150 * scale; // Fixed height
        
        // Scale context for crisp rendering
        const ctx = canvas.getContext('2d');
        ctx.scale(scale, scale);
    }

    /**
     * Setup event listeners for sensor data
     */
    setupEventListeners() {
        this.core.on('sensors:update', (data) => {
            this.updateSensorData(data);
        });

        this.core.on('system:status', (data) => {
            this.updateSystemData(data);
        });
    }

    /**
     * Initialize sensor data storage
     */
    initializeSensorData(canvasId) {
        switch (canvasId) {
            case 'accelerometer-canvas':
                this.sensorData.set('accelerometer', {
                    x: this.createDataBuffer(),
                    y: this.createDataBuffer(),
                    z: this.createDataBuffer(),
                    current: { x: 0, y: 0, z: 0 }
                });
                break;
            case 'gyroscope-canvas':
                this.sensorData.set('gyroscope', {
                    x: this.createDataBuffer(),
                    y: this.createDataBuffer(),
                    z: this.createDataBuffer(),
                    current: { x: 0, y: 0, z: 0 }
                });
                break;
            case 'distance-canvas':
                this.sensorData.set('distance', {
                    values: this.createDataBuffer(),
                    current: 0,
                    max: 400 // cm
                });
                break;
            case 'cliff-canvas':
                this.sensorData.set('cliff', {
                    detected: false,
                    confidence: 0,
                    history: this.createDataBuffer()
                });
                break;
        }
    }

    /**
     * Create circular buffer for time-series data
     */
    createDataBuffer(size = 50) {
        return {
            data: new Array(size).fill(0),
            index: 0,
            size: size,
            add: function(value) {
                this.data[this.index] = value;
                this.index = (this.index + 1) % this.size;
            },
            getArray: function() {
                return [...this.data.slice(this.index), ...this.data.slice(0, this.index)];
            }
        };
    }

    /**
     * Update sensor data from incoming WebSocket messages
     */
    updateSensorData(data) {
        // Accelerometer data
        if (data.accelerometer) {
            const accel = this.sensorData.get('accelerometer');
            accel.current = data.accelerometer;
            accel.x.add(data.accelerometer.x);
            accel.y.add(data.accelerometer.y);
            accel.z.add(data.accelerometer.z);
            
            // Update UI text values
            this.updateElementText('accel-x', data.accelerometer.x.toFixed(2));
            this.updateElementText('accel-y', data.accelerometer.y.toFixed(2));
            this.updateElementText('accel-z', data.accelerometer.z.toFixed(2));
        }

        // Gyroscope data
        if (data.gyroscope) {
            const gyro = this.sensorData.get('gyroscope');
            gyro.current = data.gyroscope;
            gyro.x.add(data.gyroscope.x);
            gyro.y.add(data.gyroscope.y);
            gyro.z.add(data.gyroscope.z);
            
            // Update UI text values
            this.updateElementText('gyro-x', data.gyroscope.x.toFixed(2));
            this.updateElementText('gyro-y', data.gyroscope.y.toFixed(2));
            this.updateElementText('gyro-z', data.gyroscope.z.toFixed(2));
        }

        // Distance sensor data
        if (data.distance !== undefined) {
            const distance = this.sensorData.get('distance');
            distance.current = data.distance;
            distance.values.add(data.distance);
            
            // Update UI text value
            this.updateElementText('distance', Math.round(data.distance));
        }

        // Cliff detector data
        if (data.cliff !== undefined) {
            const cliff = this.sensorData.get('cliff');
            cliff.detected = data.cliff.detected;
            cliff.confidence = data.cliff.confidence || 0;
            cliff.history.add(cliff.detected ? 1 : 0);
            
            // Update UI text value
            this.updateElementText('cliff-status', cliff.detected ? 'CLIFF!' : 'Safe');
        }
    }

    /**
     * Update system data (battery, memory, etc.)
     */
    updateSystemData(data) {
        if (data.battery) {
            this.updateElementText('battery-level', `${data.battery}%`);
        }
        if (data.memory) {
            this.updateElementText('memory-usage', `${data.memory}%`);
        }
        if (data.temperature) {
            this.updateElementText('temperature', `${data.temperature}°C`);
        }
        if (data.wifi) {
            this.updateElementText('wifi-strength', `${data.wifi.rssi}dBm`);
        }
    }

    /**
     * Update element text content safely
     */
    updateElementText(id, value) {
        const element = document.getElementById(id);
        if (element) {
            element.textContent = value;
        }
    }

    /**
     * Start animation loop for smooth chart updates
     */
    startAnimationLoop() {
        const animate = () => {
            this.renderAllCharts();
            requestAnimationFrame(animate);
        };
        requestAnimationFrame(animate);
    }

    /**
     * Render all charts
     */
    renderAllCharts() {
        this.renderAccelerometer();
        this.renderGyroscope();
        this.renderDistance();
        this.renderCliff();
    }

    /**
     * Render accelerometer chart
     */
    renderAccelerometer() {
        const canvas = this.canvases.get('accelerometer-canvas');
        const ctx = this.contexts.get('accelerometer-canvas');
        const data = this.sensorData.get('accelerometer');
        
        if (!canvas || !ctx || !data) return;

        this.clearCanvas(ctx, canvas);
        
        const width = canvas.width / (window.devicePixelRatio || 1);
        const height = canvas.height / (window.devicePixelRatio || 1);
        
        // Draw grid
        this.drawGrid(ctx, width, height);
        
        // Draw axes lines
        const centerY = height / 2;
        ctx.strokeStyle = this.chartConfig.gridColor;
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(0, centerY);
        ctx.lineTo(width, centerY);
        ctx.stroke();

        // Draw data lines
        this.drawTimeSeries(ctx, data.x.getArray(), width, height, this.chartConfig.primaryColor, -2, 2);
        this.drawTimeSeries(ctx, data.y.getArray(), width, height, this.chartConfig.secondaryColor, -2, 2);
        this.drawTimeSeries(ctx, data.z.getArray(), width, height, this.chartConfig.successColor, -2, 2);
        
        // Draw legend
        this.drawLegend(ctx, width, height, [
            { color: this.chartConfig.primaryColor, label: 'X' },
            { color: this.chartConfig.secondaryColor, label: 'Y' },
            { color: this.chartConfig.successColor, label: 'Z' }
        ]);
    }

    /**
     * Render gyroscope chart
     */
    renderGyroscope() {
        const canvas = this.canvases.get('gyroscope-canvas');
        const ctx = this.contexts.get('gyroscope-canvas');
        const data = this.sensorData.get('gyroscope');
        
        if (!canvas || !ctx || !data) return;

        this.clearCanvas(ctx, canvas);
        
        const width = canvas.width / (window.devicePixelRatio || 1);
        const height = canvas.height / (window.devicePixelRatio || 1);
        
        // Draw grid
        this.drawGrid(ctx, width, height);
        
        // Draw center line
        const centerY = height / 2;
        ctx.strokeStyle = this.chartConfig.gridColor;
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(0, centerY);
        ctx.lineTo(width, centerY);
        ctx.stroke();

        // Draw data lines
        this.drawTimeSeries(ctx, data.x.getArray(), width, height, this.chartConfig.primaryColor, -200, 200);
        this.drawTimeSeries(ctx, data.y.getArray(), width, height, this.chartConfig.secondaryColor, -200, 200);
        this.drawTimeSeries(ctx, data.z.getArray(), width, height, this.chartConfig.successColor, -200, 200);
        
        // Draw legend
        this.drawLegend(ctx, width, height, [
            { color: this.chartConfig.primaryColor, label: 'X' },
            { color: this.chartConfig.secondaryColor, label: 'Y' },
            { color: this.chartConfig.successColor, label: 'Z' }
        ]);
    }

    /**
     * Render distance sensor chart
     */
    renderDistance() {
        const canvas = this.canvases.get('distance-canvas');
        const ctx = this.contexts.get('distance-canvas');
        const data = this.sensorData.get('distance');
        
        if (!canvas || !ctx || !data) return;

        this.clearCanvas(ctx, canvas);
        
        const width = canvas.width / (window.devicePixelRatio || 1);
        const height = canvas.height / (window.devicePixelRatio || 1);
        
        // Draw grid
        this.drawGrid(ctx, width, height);
        
        // Draw distance line chart
        this.drawTimeSeries(ctx, data.values.getArray(), width, height, this.chartConfig.primaryColor, 0, data.max);
        
        // Draw current distance as bar
        const currentPercent = data.current / data.max;
        const barHeight = height * 0.8;
        const barWidth = 20;
        const barX = width - barWidth - 10;
        const barY = height - (barHeight * currentPercent);
        
        // Background bar
        ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
        ctx.fillRect(barX, height - barHeight, barWidth, barHeight);
        
        // Current value bar
        const color = data.current < 20 ? this.chartConfig.dangerColor : this.chartConfig.primaryColor;
        ctx.fillStyle = color;
        ctx.fillRect(barX, barY, barWidth, barHeight * currentPercent);
        
        // Draw value text
        ctx.fillStyle = this.chartConfig.textColor;
        ctx.font = `${this.chartConfig.fontSize}px sans-serif`;
        ctx.textAlign = 'center';
        ctx.fillText(`${Math.round(data.current)}cm`, barX + barWidth/2, height - 5);
    }

    /**
     * Render cliff detector chart
     */
    renderCliff() {
        const canvas = this.canvases.get('cliff-canvas');
        const ctx = this.contexts.get('cliff-canvas');
        const data = this.sensorData.get('cliff');
        
        if (!canvas || !ctx || !data) return;

        this.clearCanvas(ctx, canvas);
        
        const width = canvas.width / (window.devicePixelRatio || 1);
        const height = canvas.height / (window.devicePixelRatio || 1);
        
        // Draw background
        const bgColor = data.detected ? 
            'rgba(220, 53, 69, 0.2)' : 
            'rgba(25, 135, 84, 0.2)';
        ctx.fillStyle = bgColor;
        ctx.fillRect(0, 0, width, height);
        
        // Draw detection history
        this.drawTimeSeries(ctx, data.history.getArray(), width, height, 
            data.detected ? this.chartConfig.dangerColor : this.chartConfig.successColor, 0, 1);
        
        // Draw status indicator
        const centerX = width / 2;
        const centerY = height / 2;
        const radius = Math.min(width, height) * 0.15;
        
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI);
        ctx.fillStyle = data.detected ? this.chartConfig.dangerColor : this.chartConfig.successColor;
        ctx.fill();
        
        // Draw status text
        ctx.fillStyle = this.chartConfig.textColor;
        ctx.font = `bold ${this.chartConfig.fontSize}px sans-serif`;
        ctx.textAlign = 'center';
        ctx.fillText(data.detected ? 'CLIFF' : 'SAFE', centerX, centerY + 30);
    }

    /**
     * Draw time series line chart
     */
    drawTimeSeries(ctx, dataArray, width, height, color, minValue, maxValue) {
        if (dataArray.length < 2) return;
        
        ctx.strokeStyle = color;
        ctx.lineWidth = this.chartConfig.lineWidth;
        ctx.beginPath();
        
        const stepX = width / (dataArray.length - 1);
        const range = maxValue - minValue;
        
        dataArray.forEach((value, index) => {
            const x = index * stepX;
            const normalizedValue = (value - minValue) / range;
            const y = height - (normalizedValue * height);
            
            if (index === 0) {
                ctx.moveTo(x, y);
            } else {
                ctx.lineTo(x, y);
            }
        });
        
        ctx.stroke();
    }

    /**
     * Draw grid lines
     */
    drawGrid(ctx, width, height) {
        ctx.strokeStyle = this.chartConfig.gridColor;
        ctx.lineWidth = 0.5;
        ctx.beginPath();
        
        // Horizontal lines
        for (let i = 0; i <= 4; i++) {
            const y = (height / 4) * i;
            ctx.moveTo(0, y);
            ctx.lineTo(width, y);
        }
        
        // Vertical lines
        for (let i = 0; i <= 4; i++) {
            const x = (width / 4) * i;
            ctx.moveTo(x, 0);
            ctx.lineTo(x, height);
        }
        
        ctx.stroke();
    }

    /**
     * Draw legend
     */
    drawLegend(ctx, width, height, items) {
        const legendX = 10;
        const legendY = 15;
        const itemHeight = 12;
        
        ctx.font = `${this.chartConfig.fontSize}px sans-serif`;
        ctx.textAlign = 'left';
        
        items.forEach((item, index) => {
            const y = legendY + (index * itemHeight);
            
            // Draw color indicator
            ctx.fillStyle = item.color;
            ctx.fillRect(legendX, y - 8, 8, 8);
            
            // Draw label
            ctx.fillStyle = this.chartConfig.textColor;
            ctx.fillText(item.label, legendX + 12, y);
        });
    }

    /**
     * Clear canvas
     */
    clearCanvas(ctx, canvas) {
        const width = canvas.width / (window.devicePixelRatio || 1);
        const height = canvas.height / (window.devicePixelRatio || 1);
        
        ctx.fillStyle = this.chartConfig.backgroundColor;
        ctx.fillRect(0, 0, width, height);
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
     * Cleanup method
     */
    destroy() {
        // Cancel animation frames
        this.animationFrames.forEach(frame => {
            cancelAnimationFrame(frame);
        });
        
        // Clear event listeners
        this.core.off('sensors:update');
        this.core.off('system:status');
        
        // Clear data
        this.sensorData.clear();
        this.canvases.clear();
        this.contexts.clear();
    }
}

// Export for use in main app
window.CozmoSensors = CozmoSensors;
