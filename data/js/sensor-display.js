// Update gyroscope data display
function updateGyroData(data) {
    if (data.x !== undefined && data.y !== undefined && data.z !== undefined) {
        gyroX.textContent = data.x.toFixed(2);
        gyroY.textContent = data.y.toFixed(2);
        gyroZ.textContent = data.z.toFixed(2);
    }
}

// Update accelerometer and gyroscope data display
function updateSensorData(data) {
    // Update gyroscope data
    if (data.gyro) {
        gyroX.textContent = data.gyro.x.toFixed(2);
        gyroY.textContent = data.gyro.y.toFixed(2);
        gyroZ.textContent = data.gyro.z.toFixed(2);
    }
    
    // Update accelerometer data if UI elements exist
    if (data.accel) {
        const accelX = document.getElementById('accel-x');
        const accelY = document.getElementById('accel-y');
        const accelZ = document.getElementById('accel-z');
        const accelMag = document.getElementById('accel-magnitude');
        
        if (accelX) accelX.textContent = data.accel.x.toFixed(2);
        if (accelY) accelY.textContent = data.accel.y.toFixed(2);
        if (accelZ) accelZ.textContent = data.accel.z.toFixed(2);
        if (accelMag) accelMag.textContent = data.accel.magnitude.toFixed(2);
    }
}
