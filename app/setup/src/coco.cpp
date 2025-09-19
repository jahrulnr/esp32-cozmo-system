#include <Arduino.h>
#include "setup/setup.h"
#include <queue.h>

COCODetect *cocoDetect;
dl_data_t* cocoData;
dl::detect::result_t* cocoResult;

void setupCoco() {
	cocoDetect = new COCODetect();
  // cocoData = (dl_data_t*) heap_caps_calloc(1, sizeof(dl_data_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  cocoData = (dl_data_t*) heap_caps_calloc(1, sizeof(dl_data_t), MALLOC_CAP_SPIRAM);
	cocoData->resultQue = xQueueCreateWithCaps(1, sizeof(dl_data_t), MALLOC_CAP_SPIRAM);
	cocoData->mode = DL_MODE_WAITING;
}