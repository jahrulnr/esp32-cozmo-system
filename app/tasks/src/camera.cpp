#include "../register.h"

Sensors::Camera *camera;

// we start camera after 10s
// this will reduce startup memory

void cameraTask(void * param) {
  const char* TAG = "cameraTask";
  int startDelay = 10000;
  vTaskDelay(pdMS_TO_TICKS(startDelay));

  #ifdef CONFIG_CAMERA_TASK_STACK_SIZE
  ESP_LOGI(TAG, "cam_task stack size: %d", CONFIG_CAMERA_TASK_STACK_SIZE);
  #undef CONFIG_CAMERA_TASK_STACK_SIZE
  #endif

  #define CONFIG_CAMERA_TASK_STACK_SIZE 4096
  if (CAMERA_ENABLED) {
    camera = new Sensors::Camera();
    if (camera->init()) {
      camera->setResolution(CAMERA_FRAME_SIZE);
      logger->info("Camera initialized successfully");
    } else {
      camera = nullptr;
      logger->error("Camera initialization failed");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    camera->~Camera();
  }

  ESP_LOGI(TAG, "cam_task stack size after initiated: %d", CONFIG_CAMERA_TASK_STACK_SIZE);

  vTaskDelete(NULL);
}