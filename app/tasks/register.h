#pragma once
#include <Arduino.h>
#include "setup/setup.h"

// Task handles
extern TaskHandle_t speechRecognitionTaskHandle;
extern TaskHandle_t sensorMonitorTaskHandle;
extern TaskHandle_t gptTaskHandle;
extern TaskHandle_t weatherServiceTaskHandle;

void protectCozmoTask(void * param);
void displayTask(void* param);
void gptChatTask(void* parameter);
void sensorMonitorTask(void* parameter);
void ftpTask(void* param);
void weatherServiceTask(void* param);

// Function prototypes
void protectCozmo();