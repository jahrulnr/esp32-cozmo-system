#include <Arduino.h>
#include "app.h"

void setupCamera() {
  if (CAMERA_ENABLED) {
    camera = new Sensors::Camera();
    logger->info("Setting up camera...");
    if (camera->init()) {
      camera->setResolution(CAMERA_FRAME_SIZE);
      logger->info("Camera initialized successfully");
    } else {
      logger->error("Camera initialization failed");
    }
  }
}

// Global flags for camera streaming control
bool g_cameraStreaming = false;

/**
 * Start camera streaming
 */
void startCameraStreaming() {
  if (!g_cameraStreaming && camera) {
    g_cameraStreaming = true;
    logger->info("Camera streaming started");
  }
}

/**
 * Stop camera streaming
 */
void stopCameraStreaming() {
  if (g_cameraStreaming) {
    g_cameraStreaming = false;
    logger->info("Camera streaming stopped");
  }
}

/**
 * Check if camera is streaming
 */
bool isCameraStreaming() {
  return g_cameraStreaming;
}

/**
 * Camera streaming task
 * Captures frames from the camera and streams them via WebSocket
 */
void cameraStreamTask(void* parameter) {
    // Check if camera and WebSocket are initialized
    if (!camera || !webSocket) {
        if (logger) {
            logger->error("Camera streaming task failed: components not initialized");
        }
        cameraStreamTaskHandle = nullptr;
        vTaskDelete(NULL);
        return;
    }
    
    logger->info("Camera streaming task started");
    
    // Track memory and adjust streaming parameters as needed
    const uint32_t LOW_MEMORY_THRESHOLD = 30000; // 30KB
    uint32_t consecutiveLowMemory = 0;
    uint32_t adaptiveInterval = camera->getStreamingInterval();
    
    while (true) {
      // Only process frames when streaming is enabled and clients are connected
      if (g_cameraStreaming && webSocket->hasClients() && webSocket->hasClientsForCameraFrames()) {
          // Capture frame
          camera_fb_t* fb = camera->captureFrame();
          if (!fb) {
            fb = esp_camera_fb_get();
          }
          
          if (fb) {
              // Create a small JSON header with metadata
              Utils::SpiJsonDocument header;
              header["width"] = fb->width;
              header["height"] = fb->height;
              header["format"] = fb->format;
              header["size"] = fb->len;
              
              // First send the metadata as JSON text frame with DTO v1.0 format
              webSocket->sendJsonMessage(-1, "camera_frame_header", header);
              
              // Then send the binary frame data
              webSocket->sendBinary(-1, fb->buf, fb->len);
              
              // Return frame buffer immediately after use
              camera->returnFrame(fb);
              
              // Check memory status and adapt if needed
              uint32_t freeHeap = ESP.getFreeHeap();
              if (freeHeap < LOW_MEMORY_THRESHOLD) {
                  consecutiveLowMemory++;
                  
                  // If memory is consistently low, increase interval gradually
                  if (consecutiveLowMemory > 5) {
                      adaptiveInterval = _min(500u, adaptiveInterval + 20); // Cap at 500ms
                      logger->warning("Low memory detected, slowing camera stream to " + 
                                      String(adaptiveInterval) + "ms");
                      consecutiveLowMemory = 0;
                  }
              } else {
                  consecutiveLowMemory = 0;
                  
                  // If memory is good, gradually return to normal interval
                  if (adaptiveInterval > camera->getStreamingInterval()) {
                      adaptiveInterval = _max(camera->getStreamingInterval(), adaptiveInterval - 10);
                  }
              }
              logger->info("capturing image");
              // Wait for the next frame using adaptive interval
              vTaskDelay(adaptiveInterval / portTICK_PERIOD_MS);
          } else {
            logger->info("capture image failed");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
          }
          
      } else {
          // No streaming needed, sleep longer to save resources
          vTaskDelay(2000 / portTICK_PERIOD_MS);
          logger->info("camera stanby");
          logger->info("camera - g_cameraStreaming: " + String(g_cameraStreaming ? "active" : "inactive"));
          logger->info("camera - hasClients: " + String(webSocket->hasClients() ? "active" : "inactive"));
          logger->info("camera - hasClientsForCameraFrames: " + String(webSocket->hasClientsForCameraFrames() ? "active" : "inactive"));
      }
    }
}