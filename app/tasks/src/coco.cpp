#include "tasks/register.h"
#include <esp_log.h>
#include <esp32-hal-log.h>
#include "vision/image/dl_image_draw.hpp"
#include "vision/image/dl_image_jpeg.hpp"

// Category name mapping
const char* getCategoryName(int category) {
    switch (category) {
        case 0: return "PERSON";
        // Add more categories as you expand the detection
        default: return "UNKNOWN";
    }
}

// Simple 5x7 bitmap font for text rendering
static const uint8_t font_5x7[][7] = {
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // A (65)
    {0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00, 0x00},
    // B (66) 
    {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00},
    // C (67)
    {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00},
    // D (68)
    {0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00},
    // E (69)
    {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00},
    // F (70)
    {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00, 0x00},
    // G (71)
    {0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00, 0x00},
    // H (72)
    {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x00},
    // I (73)
    {0x41, 0x7F, 0x41, 0x00, 0x00, 0x00, 0x00},
    // J (74)
    {0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, 0x00},
    // K (75)
    {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00},
    // L (76)
    {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00},
    // M (77)
    {0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00, 0x00},
    // N (78)
    {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, 0x00},
    // O (79)
    {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x00},
    // P (80)
    {0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, 0x00},
    // Q (81)
    {0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, 0x00},
    // R (82)
    {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, 0x00},
    // S (83)
    {0x46, 0x49, 0x49, 0x49, 0x31, 0x00, 0x00},
    // T (84)
    {0x01, 0x01, 0x7F, 0x01, 0x01, 0x00, 0x00},
    // U (85)
    {0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, 0x00},
    // V (86)
    {0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, 0x00},
    // W (87)
    {0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00, 0x00},
    // X (88)
    {0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00},
    // Y (89)
    {0x07, 0x08, 0x70, 0x08, 0x07, 0x00, 0x00},
    // Z (90)
    {0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00},
    // 0 (48)
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x00},
    // 1 (49)
    {0x42, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00},
    // 2 (50)
    {0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00},
    // 3 (51)
    {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00, 0x00},
    // 4 (52)
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00},
    // 5 (53)
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00},
    // 6 (54)
    {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00},
    // 7 (55)
    {0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00},
    // 8 (56)
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00},
    // 9 (57)
    {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00}
};

// Character index mapping
int getCharIndex(char c) {
    if (c == ' ') return 0;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 1;
    if (c >= 'a' && c <= 'z') return c - 'a' + 1; // Convert to uppercase
    if (c >= '0' && c <= '9') return c - '0' + 27;
    return 0; // Default to space
}

// Draw a single character using bitmap font
void drawChar(dl::image::img_t &imageData, int x, int y, char c, const std::vector<uint8_t> &color) {
    int charIndex = getCharIndex(c);
    const uint8_t* charData = font_5x7[charIndex];
    
    for (int row = 0; row < 7; row++) {
        uint8_t rowData = charData[row];
        for (int col = 0; col < 5; col++) {
            if (rowData & (0x01 << col)) {
                int px = x + col;
                int py = y + row;
                
                // Check bounds
                if (px >= 0 && px < imageData.width && py >= 0 && py < imageData.height) {
                    dl::image::draw_point(imageData, px, py, color, 1);
                }
            }
        }
    }
}

// Draw text string
void drawText(dl::image::img_t &imageData, int x, int y, const char* text, const std::vector<uint8_t> &color) {
    int currentX = x;
    
    while (*text) {
        if (currentX + 6 > imageData.width) break; // Check if we have space for next character
        
        drawChar(imageData, currentX, y, *text, color);
        currentX += 6; // 5 pixels for character + 1 pixel spacing
        text++;
    }
}

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
            ESP_LOGW("coco", "Unsupported pixel format for drawing");
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

            // Draw category label
            const char* categoryName = getCategoryName(res.category);
            
            // Calculate label position - above the bounding box
            int labelX = x1;
            int labelY = y1 - 10; // 10 pixels above the box
            
            // If label would be outside image bounds, place it inside the box
            if (labelY < 0) {
                labelY = y1 + 3; // Place inside the box, 3 pixels down from top
            }
            
            // Ensure label doesn't go outside image bounds
            if (labelY >= 0 && labelY + 7 < imageData.height && labelX >= 0) {
                // Draw background rectangle for text (optional, for better readability)
                int textWidth = strlen(categoryName) * 6; // 6 pixels per character (5 + 1 spacing)
                int textHeight = 7;
                
                // Draw semi-transparent background for text (using same color but dimmed)
                std::vector<uint8_t> bgColor = color;
                if (imageData.pix_type == dl::image::DL_IMAGE_PIX_TYPE_RGB888) {
                    // Dim the color for background
                    bgColor[0] = bgColor[0] / 3;
                    bgColor[1] = bgColor[1] / 3;
                    bgColor[2] = bgColor[2] / 3;
                } else if (imageData.pix_type == dl::image::DL_IMAGE_PIX_TYPE_RGB565) {
                    // For RGB565, create a dimmed version
                    uint16_t originalColor = (bgColor[1] << 8) | bgColor[0];
                    uint16_t dimmedColor = ((originalColor >> 1) & 0x7BEF); // Dim by half
                    bgColor[0] = dimmedColor & 0xFF;
                    bgColor[1] = (dimmedColor >> 8) & 0xFF;
                }
                
                // Draw background rectangle
                for (int bg_y = labelY - 1; bg_y < labelY + textHeight + 1 && bg_y < imageData.height; bg_y++) {
                    for (int bg_x = labelX - 1; bg_x < labelX + textWidth + 1 && bg_x < imageData.width; bg_x++) {
                        if (bg_x >= 0 && bg_y >= 0) {
                            dl::image::draw_point(imageData, bg_x, bg_y, bgColor, 1);
                        }
                    }
                }
                
                // Draw category text label
                drawText(imageData, labelX, labelY, categoryName, color);
                
                // Draw confidence score as percentage (optional)
                char scoreText[8];
                snprintf(scoreText, sizeof(scoreText), "%.0f%%", res.score * 100);
                
                // Draw score below the category name
                int scoreY = labelY + 9; // 9 pixels below category name
                if (scoreY + 7 < imageData.height) {
                    drawText(imageData, labelX, scoreY, scoreText, color);
                }
            }
        }
    }
}

void cocoHandlerTask(void* param) {
	static const char* tag = "cocoHandlerTask";

	const char* fileName = "/cache/annotated_frame.jpg";
	if (fileManager->exists(fileName)) {
			fileManager->deleteFile(fileName);
	}

	ESP_LOGI(tag, "set feed task to stanby mode");
	cocoData->mode = DL_MODE_STANBY;

	TickType_t updateFrequency = pdMS_TO_TICKS(1000);
	while(1){
		vTaskDelay(updateFrequency);

		if (cocoData->mode != DL_MODE_READY) {
			continue;
		}

		// Wait for image data from feed task
		dl::image::img_t imageData;
		if (xQueueReceive(cocoData->resultQue, &imageData, updateFrequency) != pdTRUE) {
      continue;
    }

		if (imageData.data == nullptr) {
			ESP_LOGI(tag, "camera data received, but empty. skipping");
			cocoData->mode = DL_MODE_STANBY;
			continue;
		}

		ESP_LOGI(tag, "camera data received");
		cocoData->mode = DL_MODE_ANALYZE;
		ESP_LOGI(tag, "analyze camera data");
    auto &detect_results = cocoDetect->run(imageData);

    // Log detection results
		ESP_LOGI(tag, "analyze done, result: %d", detect_results.size());
    for (const auto &res : detect_results) {
        ESP_LOGI(tag,
                 "category: %2d [score: %f, x1: %d, y1: %d, x2: %d, y2: %d]",
								 res.category,
                 res.score,
                 res.box[0],
                 res.box[1],
                 res.box[2],
                 res.box[3]);
    }

		if (fileManager->exists(fileName)) {
			heap_caps_free(imageData.data);
			cocoData->mode = DL_MODE_STANBY;
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

    heap_caps_free(imageData.data);
		if (cocoData->mode != DL_MODE_OFF)
			cocoData->mode = DL_MODE_STANBY;
	}
}

void cocoFeedTask(void* param) {
	static const char* tag = "cocoFeedTask";
	const char* fileName = "/cache/frame.jpg";

	TickType_t updateFrequency = pdMS_TO_TICKS(3000);

	if (fileManager->exists(fileName)) {
			fileManager->deleteFile(fileName);
	}

	camera_fb_t* fb;
	while(1){
		vTaskDelay(updateFrequency);
		if (cocoData->mode == DL_MODE_WAITING) {
			ESP_LOGI(tag, "waiting mode");
			continue;
		}

		if (cocoData->mode != DL_MODE_STANBY) {
            continue;
        }

		std::list<dl::detect::result_t> result;
		if (camera == nullptr) {
			continue;
		}

		cocoData->mode = DL_MODE_PROCCESS;

		camera->returnFrame(fb);
		sensor_t *s = esp_camera_sensor_get();
		s->set_hmirror(s, 1);
		s->set_vflip(s, 1);

		delay(100);
		if (!fileManager->exists(fileName)) {
			fb = camera->captureFrame();
			if (!fb) {
				ESP_LOGW(tag, "failed to get camera data");
				cocoData->mode = DL_MODE_STANBY;
				continue;
			}
			auto file = fileManager->openFileForWriting(fileName);
			if (file) {
					size_t written = fileManager->writeBinary(file, fb->buf, fb->len);
					fileManager->closeFile(file);

					if (written == fb->len) {
							ESP_LOGI(tag, "Saved image: %s (%d bytes)", fileName, fb->len);
					} else {
							ESP_LOGW(tag, "Failed to write complete annotated image");
					}
			}
	
			camera->returnFrame(fb);
			vTaskDelay(pdMS_TO_TICKS(500));
		}

		fb = camera->captureFrame(1);
		if (!fb) {
			ESP_LOGW(tag, "failed to get camera data");
			cocoData->mode = DL_MODE_STANBY;
			continue;
		}

    dl::image::img_t imageToProcess = {
			.width = uint16_t(fb->width),
			.height = uint16_t(fb->height),
			.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888 // pix_type_t
		};

		uint8_t* imgRes = (uint8_t*) heap_caps_malloc(fb->width*fb->height*3, MALLOC_CAP_SPIRAM);
        if (fb->format != PIXFORMAT_RGB888) {
    		fmt2rgb888(fb->buf, fb->len, fb->format, imgRes);
        } else {
            memcpy(imgRes, fb->buf, fb->len);
        }

        imageToProcess.data = imgRes;
		if (imageToProcess.data == nullptr) {
			ESP_LOGW(tag, "failed to decode camera data");
			cocoData->mode = DL_MODE_STANBY;
			continue;
		}

		if (cocoData->mode != DL_MODE_OFF){
			ESP_LOGI(tag, "camera data ready to proccess");
			cocoData->mode = DL_MODE_READY;
		}

		// Send image data to handler task via queue
		if (xQueueSend(cocoData->resultQue, &imageToProcess, updateFrequency) != pdTRUE) {
			ESP_LOGW(tag, "Failed to send image data to queue");
			cocoData->mode = DL_MODE_STANBY;
			if (imgRes) {
				heap_caps_free(imgRes); // Free decoded JPEG data
			}
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}