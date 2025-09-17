#pragma once

#include "vision/image/dl_image_jpeg.hpp"
#include <queue.h>

typedef enum {
  DL_MODE_OFF,
  DL_MODE_ANALYZE,
  DL_MODE_PROCCESS,
  DL_MODE_READY,
  DL_MODE_MAX
} dl_mode_t;

// Define bit flags for EventGroup
#define DL_EVENT_PAUSE     BIT0
#define DL_EVENT_RESUME    BIT1
#define DL_EVENT_STOP      BIT2
#define DL_EVENT_START     BIT3

typedef struct {
	dl::image::img_t imageData;
	QueueHandle_t resultQue;
	dl_mode_t mode;
  EventGroupHandle_t eventGroup;
} dl_data_t;