#include "../setup.h"

Sensors::Camera *camera;

void setupCamera() {
  if (CAMERA_ENABLED) {
    camera = new Sensors::Camera();
    if (camera->init()) {
      camera->setResolution(CAMERA_FRAME_SIZE);
      logger->info("Camera initialized successfully");
    } else {
      camera = nullptr;
      logger->error("Camera initialization failed");
    }
  }
}