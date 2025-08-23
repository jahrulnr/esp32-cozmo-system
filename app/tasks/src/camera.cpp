#include <Arduino.h>
#include "../register.h"

// Global flags for camera streaming control
bool _cameraStreaming = false;

/**
 * Start camera streaming
 */
void startCameraStreaming() {
  if (!_cameraStreaming && camera) {
    _cameraStreaming = true;
    logger->info("Camera streaming started");
  }
}

/**
 * Stop camera streaming
 */
void stopCameraStreaming() {
  if (_cameraStreaming) {
    _cameraStreaming = false;
    logger->info("Camera streaming stopped");
  }
}

/**
 * Check if camera is streaming
 */
bool isCameraStreaming() {
  return _cameraStreaming;
}

/**
 * Camera streaming task
 * Captures frames from the camera and streams them via WebSocket
 */
void cameraStreamTask(void* parameter) {
  // Check if camera and WebSocket are initialized
  if (!camera || !webSocket) {
    logger->error("Camera streaming task failed: components not initialized");
    cameraStreamTaskHandle = nullptr;
    vTaskDelete(NULL);
    return;
  }

  vTaskDelay(pdMS_TO_TICKS(15000));
  
  logger->info("Camera streaming task started");
  
  // Track memory and adjust streaming parameters as needed
  const uint32_t LOW_MEMORY_THRESHOLD = 30000; // 30KB
  uint32_t consecutiveLowMemory = 0;
  uint32_t adaptiveInterval = camera->getStreamingInterval();
  _cameraStreaming = false;
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  while (true) {
    vTaskDelayUntil(&lastWakeTime, adaptiveInterval / portTICK_PERIOD_MS);

    // Only process frames when streaming is enabled and clients are connected
    if (!_cameraStreaming) {
      vTaskDelay(500 / portTICK_PERIOD_MS);
      continue;
    }
      // Capture frame
    camera_fb_t* fb = esp_camera_fb_get();
    
    if (fb) {
      webSocket->sendBinary(-1, fb->buf, fb->len);
      esp_camera_fb_return(fb);
      logger->info("capturing image");
    } else {
      logger->info("capture image failed");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }
}