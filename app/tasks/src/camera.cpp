#include "../register.h"

Sensors::Camera *camera;

// we start camera after 20s
// this will reduce startup memory

void cameraTask(void * param) {
  const char* TAG = "cameraTask";
  int startDelay = 20000;
  vTaskDelay(pdMS_TO_TICKS(startDelay));

  #ifdef CONFIG_CAMERA_TASK_STACK_SIZE
  ESP_LOGI(TAG, "cam_task stack size: %d", CONFIG_CAMERA_TASK_STACK_SIZE);
  #undef CONFIG_CAMERA_TASK_STACK_SIZE
  #endif

  #define CONFIG_CAMERA_TASK_STACK_SIZE 4096
  ESP_LOGI(TAG, "cam_task stack size after redefinition: %d", CONFIG_CAMERA_TASK_STACK_SIZE);
  if (CAMERA_ENABLED) {
    camera = new Sensors::Camera();
    camera->setResolution(CAMERA_FRAME_SIZE);
    if (camera->init()) {
      esp_camera_set_psram_mode(true);
      logger->info("Camera initialized successfully");

      vTaskDelay(pdMS_TO_TICKS(1000));

      const char* fileName = "/cache/frame.jpg";
      if (fileManager->exists(fileName)) {
        fileManager->deleteFile(fileName);
      }
      camera_fb_t *fb = nullptr;
      while (true){
        if (fileManager->exists(fileName)) {
          vTaskDelay(3000);
          continue;
        }

        fb = camera->captureFrame();

        if (!fb) {
          ESP_LOGE(TAG, "failed to capture image");
          vTaskDelay(pdMS_TO_TICKS(100));
          continue;
        }

        File fopen = fileManager->openFileForWriting(fileName);
        if (fopen){
          size_t writted = fileManager->writeBinary(fopen, fb->buf, fb->len);

          if (writted > 0) {
            ESP_LOGI(TAG, "success to create image file: size=%d", writted);
          } else {
            ESP_LOGE(TAG, "failed to create image file: size=%d, fblen=%d", writted, fb->len);
          }
        } else {
          ESP_LOGE(TAG, "failed to create image file");
        }

        ESP_LOGI(TAG, "success create image file");
        if (fopen) fileManager->closeFile(fopen);
        camera->returnFrame(fb);
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
      
      printTaskStatus();
    } else {
      camera = nullptr;
      logger->error("Camera initialization failed");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }


  // vTaskDelete(NULL);
  SendTask::removeTask(cameraTaskId);
}