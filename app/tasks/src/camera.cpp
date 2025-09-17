#include "../register.h"

Sensors::Camera *camera;

// we start camera after 20s
// this will reduce startup memory

void cameraTask(void * param) {
  const char* TAG = "cameraTask";
  int startDelay = 10000;
	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(1000);
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

      sensor_t *s = esp_camera_sensor_get();
      s->set_gain_ctrl(s, 1);     // auto gain on
      vTaskDelay(10);
      s->set_exposure_ctrl(s, 1); // auto exposure on
      vTaskDelay(10);
      s->set_awb_gain(s, 1);
      vTaskDelay(10);
      s->set_hmirror(s, 1);
      vTaskDelay(10);             // the config sometime not applied, seem like caused by mutex
      s->set_vflip(s, 1);

      vTaskDelay(pdMS_TO_TICKS(1000));

      const char* fileName = "/cache/frame.jpg";
      if (fileManager->exists(fileName)) {
        fileManager->deleteFile(fileName);
      }
      camera_fb_t *fb = nullptr;
      while (1){
        if (fileManager->exists(fileName)) {
          vTaskDelay(3000);
          continue;
        }

        if (esp_camera_available_frames()) {
          esp_camera_return_all();
        }

		    vTaskDelayUntil(&lastWakeTime, updateFrequency);
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
      }
    } else {
      camera = nullptr;
      logger->error("Camera initialization failed");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  SendTask::removeTask(cameraTaskId);
}