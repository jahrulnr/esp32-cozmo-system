#pragma once

#include <Arduino.h>
#include <esp_camera.h>
#include "CameraConfig.h"

namespace Sensors {

/**
 * Camera class for ESP32-CAM
 * Handles camera initialization, configuration, and frame capture
 */
class Camera {
public:
    Camera();
    ~Camera();

    /**
     * Initialize the camera with the specified configuration
     * @return true if initialization was successful, false otherwise
     */
    bool init();

    /**
     * Capture a frame from the camera
     * @return Pointer to the camera frame buffer, or NULL on failure
     */
    camera_fb_t* captureFrame();

    /**
     * Return a frame buffer to the pool
     * @param fb Pointer to the frame buffer to return
     */
    void returnFrame(camera_fb_t* fb);

    /**
     * Set the camera resolution
     * @param resolution The desired resolution
     */
    void setResolution(framesize_t resolution);

    /**
     * Get the current camera resolution
     * @return The current resolution
     */
    framesize_t getResolution() const;
    
    /**
     * Get the streaming interval in milliseconds
     * @return Time in ms between frames
     */
    uint32_t getStreamingInterval() const;
    
    /**
     * Set streaming interval in milliseconds
     * @param interval Time in ms between frames
     */
    void setStreamingInterval(uint32_t interval);
    
    /**
     * Adjust camera settings
     * @param brightness Value from -2 to 2
     * @param contrast Value from -2 to 2
     * @param saturation Value from -2 to 2
     */
    void adjustSettings(int brightness, int contrast, int saturation);

private:
    framesize_t _resolution;
    bool _initialized;
    uint32_t _streamingInterval;
};

} // namespace Sensors
