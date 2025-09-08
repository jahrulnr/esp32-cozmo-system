#pragma once
#include <Arduino.h>
#include "setup/setup.h"

// Task IDs for tracking
extern String taskMonitorerId;
extern String displayTaskId;
extern String sensorMonitorTaskId;
extern String cameraTaskId;
extern String protectCozmoTaskId;
extern String ftpTaskId;
extern String weatherServiceTaskId;
extern String srControlTaskId;
extern String notePlayerTaskId;

void taskMonitorer(void* param);
void protectCozmoTask(void * param);
void displayTask(void* param);
void gptChatTask(void* parameter);
void sensorMonitorTask(void* parameter);
void ftpTask(void* param);
void weatherServiceTask(void* param);
void srControlTask(void* param);
void cameraTask(void* param);
void notePlayerTask(void* param);

// Function prototypes
void protectCozmo();

// Task management utilities
void printTaskStatus();
void cleanupTasks();