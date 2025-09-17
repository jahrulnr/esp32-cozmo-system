#include "tasks/register.h"
#include <esp_log.h>
#include <esp32-hal-log.h>
#include "vision/image/dl_image_draw.hpp"
#include "vision/image/dl_image_jpeg.hpp"

// Helper function to get color for different scores
std::vector<uint8_t> getScoreColor(float score, dl::image::pix_type_t pix_type) {
    std::vector<uint8_t> color;

    if (pix_type == dl::image::DL_IMAGE_PIX_TYPE_RGB565) {
        // RGB565 format: 2 bytes per pixel
        uint16_t rgb565_color;
        if (score > 0.8) {
            // High confidence: Green (0x07E0)
            rgb565_color = 0x07E0;
        } else if (score > 0.5) {
            // Medium confidence: Yellow (0xFFE0)
            rgb565_color = 0xFFE0;
        } else {
            // Low confidence: Red (0xF800)
            rgb565_color = 0xF800;
        }
        color.push_back(rgb565_color & 0xFF);         // Low byte
        color.push_back((rgb565_color >> 8) & 0xFF);  // High byte
    } else if (pix_type == dl::image::DL_IMAGE_PIX_TYPE_RGB888) {
        // RGB888 format: 3 bytes per pixel
        if (score > 0.8) {
            // High confidence: Green
            color = {0, 255, 0};
        } else if (score > 0.5) {
            // Medium confidence: Yellow
            color = {255, 255, 0};
        } else {
            // Low confidence: Red
            color = {255, 0, 0};
        }
    }

    return color;
}

// Helper function to draw bounding boxes with scores
void drawDetectionResults(dl::image::img_t &imageData, const std::list<dl::detect::result_t> &detect_results) {
    for (const auto &res : detect_results) {
        // Get color based on confidence score
        std::vector<uint8_t> color = getScoreColor(res.score, imageData.pix_type);

        if (color.empty()) {
            ESP_LOGW("pedestrian", "Unsupported pixel format for drawing");
            continue;
        }

        // Draw bounding box
        int x1 = res.box[0];
        int y1 = res.box[1];
        int x2 = res.box[2];
        int y2 = res.box[3];

        // Ensure coordinates are within image bounds
        x1 = std::max(0, std::min(x1, (int)imageData.width - 1));
        y1 = std::max(0, std::min(y1, (int)imageData.height - 1));
        x2 = std::max(0, std::min(x2, (int)imageData.width - 1));
        y2 = std::max(0, std::min(y2, (int)imageData.height - 1));

        if (x2 > x1 && y2 > y1) {
            // Draw hollow rectangle for bounding box
            dl::image::draw_hollow_rectangle(imageData, x1, y1, x2, y2, color, 2);

            // Draw a filled circle at top-left corner to indicate confidence
            int radius = (res.score > 0.8) ? 4 : (res.score > 0.5) ? 3 : 2;
            if (x1 + radius < imageData.width && y1 + radius < imageData.height) {
                dl::image::draw_point(imageData, x1 + radius, y1 + radius, color, radius);
            }
        }
    }
}

void pedestrianHandlerTask(void* param) {
	static const char* tag = "pedestrianHandlerTask";

	const char* fileName = "/cache/annotated_frame.jpg";
	if (fileManager->exists(fileName)) {
			fileManager->deleteFile(fileName);
	}

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

    // Log detection results
    for (const auto &res : detect_results) {
        ESP_LOGI(tag,
                 "[score: %f, x1: %d, y1: %d, x2: %d, y2: %d]",
                 res.score,
                 res.box[0],
                 res.box[1],
                 res.box[2],
                 res.box[3]);
    }

		if (fileManager->exists(fileName)) {
			pedestrianData->mode = DL_MODE_STANBY;
			continue;
		}


    // Draw bounding boxes on the image
    if (!detect_results.empty()) {
        ESP_LOGI(tag, "Drawing %d detection boxes on image", detect_results.size());
        drawDetectionResults(imageData, detect_results);
        // Encode annotated image as JPEG and save
				try {
						// Use software JPEG encoding (more compatible)
						dl::image::jpeg_img_t encoded = dl::image::sw_encode_jpeg(imageData, 0, 85);

						if (encoded.data && encoded.data_len > 0) {
								// Write JPEG data to file
								auto file = fileManager->openFileForWriting(fileName);
								if (file) {
										size_t written = fileManager->writeBinary(file,
																														static_cast<const uint8_t*>(encoded.data),
																														encoded.data_len);
										fileManager->closeFile(file);

										if (written == encoded.data_len) {
												ESP_LOGI(tag, "Saved annotated image: %s (%d bytes)", fileName, encoded.data_len);
										} else {
												ESP_LOGW(tag, "Failed to write complete annotated image");
										}
								}

								// Free encoded data
								heap_caps_free(encoded.data);
						} else {
								ESP_LOGW(tag, "Failed to encode annotated image as JPEG");
						}
				} catch (...) {
						ESP_LOGE(tag, "Exception occurred while saving annotated image");
				}
    }

    // heap_caps_free(imageData.data);
		if (pedestrianData->mode != DL_MODE_OFF)
			pedestrianData->mode = DL_MODE_STANBY;
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
		if (pedestrianData->mode != DL_MODE_STANBY) {
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
		if (!fb) {
			pedestrianData->mode = DL_MODE_STANBY;
			continue;
		}

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

		if (pedestrianData->mode != DL_MODE_OFF)
			pedestrianData->mode = DL_MODE_READY;

		// Send image data to handler task via queue
		if (xQueueSend(pedestrianData->resultQue, &imageToProcess, pdMS_TO_TICKS(100)) != pdTRUE) {
			ESP_LOGW(tag, "Failed to send image data to queue");
			pedestrianData->mode = DL_MODE_STANBY;
			if (fb->format != PIXFORMAT_RGB565) {
				heap_caps_free(img.data); // Free decoded JPEG data
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
}