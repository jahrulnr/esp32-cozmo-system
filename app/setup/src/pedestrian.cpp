#include <Arduino.h>
#include "setup/setup.h"
#include <queue.h>
#include <core/AI/Pedestrian/pedestrian_detect.hpp>

PedestrianDetect *pedestrianDetect;
dl_data_t* pedestrianData;
dl::detect::result_t* pedestrianResult;

void setupPedestrian() {
	pedestrianDetect = new PedestrianDetect();
  pedestrianData = (dl_data_t*) heap_caps_calloc(1, sizeof(dl_data_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
	pedestrianData->resultQue = xQueueCreate(3, sizeof(dl_data_t));
	pedestrianData->mode = DL_MODE_WAITING;
}