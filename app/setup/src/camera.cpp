#include <setup/setup.h>

void setupCamera() {
  static const char* TAG = "camera";
  #ifdef CONFIG_CAMERA_TASK_STACK_SIZE
  ESP_LOGI(TAG, "cam_task stack size: %d", CONFIG_CAMERA_TASK_STACK_SIZE);
  #undef CONFIG_CAMERA_TASK_STACK_SIZE
  #endif

  #define CONFIG_CAMERA_TASK_STACK_SIZE 4096
	camera = new Sensors::Camera();
	camera->setResolution(CAMERA_FRAME_SIZE);
	if (camera->init()) {
		esp_camera_set_psram_mode(true);
		logger->info("Camera initialized successfully");

		sensor_t *s = esp_camera_sensor_get();
		s->set_gain_ctrl(s, 1);     // auto gain on
		vTaskDelay(10);							// the config sometime not applied, seem like caused by mutex
		s->set_exposure_ctrl(s, 1); // auto exposure on
		vTaskDelay(10);
		s->set_awb_gain(s, 1);
		vTaskDelay(10);
		s->set_hmirror(s, 1);
		vTaskDelay(10);
		s->set_vflip(s, 1);

		vTaskDelay(pdMS_TO_TICKS(1000));
	} else {
		logger->error("Camera initialization failed");
	}
}