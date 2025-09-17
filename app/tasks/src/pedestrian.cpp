#include "tasks/register.h"
#include <esp_log.h>
#include <esp32-hal-log.h>

void pedestrianHandlerTask(void* param) {
	static const char* tag = "pedestrianHandlerTask";
	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(100);
	while(1){
		vTaskDelayUntil(&lastWakeTime, updateFrequency);

		// Check EventGroup for pause/stop events
		EventBits_t bits = xEventGroupGetBits(pedestrianData->eventGroup);
    if (DL_EVENT_PAUSE & bits) {
			// Wait for resume or stop event
			xEventGroupWaitBits(pedestrianData->eventGroup, 
												  DL_EVENT_RESUME | DL_EVENT_STOP, 
												  pdTRUE,  // Clear bits on exit
												  pdFALSE, // Wait for any bit (OR operation)
												  portMAX_DELAY);
			continue;
    }
    
    if (DL_EVENT_STOP & bits) {
			ESP_LOGI(tag, "Pedestrian handler task stopping");
			xEventGroupClearBits(pedestrianData->eventGroup, DL_EVENT_STOP);
			break;
    }

		if (pedestrianData->mode != DL_MODE_READY) {
			continue;
		}

		// Wait for image data from feed task
		dl::image::img_t imageData;
		if (xQueueReceive(pedestrianData->resultQue, &imageData, pdMS_TO_TICKS(100)) != pdTRUE) {
      continue;
    }

		pedestrianData->mode = DL_MODE_ANALYZE;
    auto &detect_results = pedestrianDetect->run(imageData);
    for (const auto &res : detect_results) {
        ESP_LOGI(tag,
                 "[score: %f, x1: %d, y1: %d, x2: %d, y2: %d]",
                 res.score,
                 res.box[0],
                 res.box[1],
                 res.box[2],
                 res.box[3]);
    }

    // heap_caps_free(imageData.data);
		if (pedestrianData->mode != DL_MODE_OFF)
			pedestrianData->mode = DL_MODE_READY;
	}
}

void pedestrianFeedTask(void* param) {
	static const char* tag = "pedestrianFeedTask";
	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(100);

	camera_fb_t* fb;
	dl::image::img_t img;
	while(1){
		vTaskDelayUntil(&lastWakeTime, updateFrequency);
		std::list<dl::detect::result_t> result;
		if (pedestrianData->mode == DL_MODE_ANALYZE || pedestrianData->mode == DL_MODE_PROCCESS) {
      continue;
    }

		// Check EventGroup bits correctly
		EventBits_t bits = xEventGroupGetBits(pedestrianData->eventGroup);
    if (DL_EVENT_PAUSE & bits) {
			// Wait for resume or stop event
			xEventGroupWaitBits(pedestrianData->eventGroup, 
												  DL_EVENT_RESUME | DL_EVENT_STOP, 
												  pdTRUE,  // Clear bits on exit
												  pdFALSE, // Wait for any bit (OR operation)
												  portMAX_DELAY);
			continue;
    }
    
    if (DL_EVENT_STOP & bits) {
			ESP_LOGI(tag, "Pedestrian feed task stopping");
			xEventGroupClearBits(pedestrianData->eventGroup, DL_EVENT_STOP);
			break;
    }

		if (camera == nullptr) {
			continue;
		}

		pedestrianData->mode = DL_MODE_PROCCESS;

		camera->returnFrame(fb);
		vTaskDelay(pdMS_TO_TICKS(10));
		fb = camera->captureFrame(1);

    dl::image::img_t imageToProcess = {
			.data = fb->buf, // void * 
			.width = (uint16_t)fb->width, // uint16_t
			.height = (uint16_t)fb->height, // uint16_t
			.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565 // pix_type_t
		};

		switch (fb->format)
		{
		case PIXFORMAT_RGB565:
			imageToProcess.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565;
			break;
		
		default: // if jpeg
			dl::image::jpeg_img_t jpeg_img = {.data = (void *)fb->buf,
                                      .data_len = fb->len};
			img = dl::image::sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB565);
			imageToProcess.data = img.data;
			imageToProcess.height = img.height;
			imageToProcess.width = img.width;
			imageToProcess.pix_type = img.pix_type;
			break;
		}

		// Send image data to handler task via queue
		if (xQueueSend(pedestrianData->resultQue, &imageToProcess, pdMS_TO_TICKS(100)) != pdTRUE) {
			ESP_LOGW(tag, "Failed to send image data to queue");
			if (fb->format != PIXFORMAT_RGB565) {
				heap_caps_free(img.data); // Free decoded JPEG data
			}
		}

		if (pedestrianData->mode != DL_MODE_OFF)
			pedestrianData->mode = DL_MODE_READY;
	}
}