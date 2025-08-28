#pragma once
#include <Arduino.h>
#include "setup/setup.h"

// Task handles
extern TaskHandle_t speechRecognitionTaskHandle;
extern TaskHandle_t cameraStreamTaskHandle;
extern TaskHandle_t sensorMonitorTaskHandle;
extern TaskHandle_t gptTaskHandle;

void protectCozmoTask(void * param);
void screenTask(void* param);
void gptChatTask(void* parameter);
void cameraStreamTask(void* parameter);
void sensorMonitorTask(void* parameter);
void speechRecognitionTask(void* param);

// Function prototypes
void protectCozmo();
bool cliffDetected();
void sendGPT(const String &prompt, Communication::GPTAdapter::ResponseCallback callback);
