#pragma once
#include <Arduino.h>
#include "setup/setup.h"
#include "handler/register.h"

// Task IDs for tracking
extern String updaterTaskId;
extern String cocoFeedTaskId;
extern String cocoHandlerTaskId;
extern String notePlayerTaskId;

void taskMonitorer(void* param);
void gptChatTask(void* parameter);
void updaterTask(void* parameter);
void cocoHandlerTask(void* param);
void cocoFeedTask(void* param);
void notePlayerTask(void* param);

// Task management utilities
void printTaskStatus();
void cleanupTasks();