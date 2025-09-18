#include <setup/setup.h>

Sensors::Camera* camera = nullptr;

void setupCamera() {
  static const char* TAG = "camera";
  // #ifdef CONFIG_CAMERA_TASK_STACK_SIZE
  // ESP_LOGI(TAG, "cam_task stack size: %d", CONFIG_CAMERA_TASK_STACK_SIZE);
  // #undef CONFIG_CAMERA_TASK_STACK_SIZE
  // #endif

  // #define CONFIG_CAMERA_TASK_STACK_SIZE 4096
	camera = new Sensors::Camera();
	if (camera->init()) {
		logger->info("Camera initialized successfully");

		vTaskDelay(pdMS_TO_TICKS(777));
	} else {
		logger->error("Camera initialization failed");
	}
}