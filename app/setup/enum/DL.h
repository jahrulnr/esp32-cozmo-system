#pragma once

#include "vision/image/dl_image_jpeg.hpp"
#include <queue.h>

typedef enum {
  DL_MODE_OFF,
  DL_MODE_ANALYZE,
  DL_MODE_PROCCESS,
  DL_MODE_READY,
  DL_MODE_STANBY,
  DL_MODE_WAITING,
  DL_MODE_MAX
} dl_mode_t;

typedef struct {
	dl::image::img_t imageData;
	QueueHandle_t resultQue;
	dl_mode_t mode;
} dl_data_t;