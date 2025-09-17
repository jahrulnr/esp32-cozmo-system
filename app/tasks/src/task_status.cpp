#include "setup/setup.h"
#include <SendTask.h>

void taskMonitorer(void* param){
	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(10000);

    vTaskDelay(pdMS_TO_TICKS(10000));
    do {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);

        cleanupTasks();
		printTaskStatus();
    }
    while(1);
}

/**
 * Print status of all tasks managed by SendTask library
 */
void printTaskStatus() {
    logger->info("=== Task Status Report ===");

    // Scan for external tasks before getting all tasks
    SendTask::scanExternalTasks();

    // Update memory usage for all tasks
    SendTask::updateAllTasksMemoryUsage();

    auto allTasks = SendTask::getAllTasks();

    if (allTasks.empty()) {
        logger->info("No tasks registered in SendTask library");
        return;
    }

    logger->info("Total tasks: %d", allTasks.size());

    // Count external vs internal tasks
    int externalCount = 0;
    int internalCount = 0;
    for (const auto& task : allTasks) {
        if (task.isExternal) {
            externalCount++;
        } else {
            internalCount++;
        }
    }

    logger->info("Task Types - Internal: %d, External: %d", internalCount, externalCount);

    // Print status summary
    int waiting = SendTask::getTaskCountByStatus(SendTask::TaskStatus::WAITING);
    int inProgress = SendTask::getTaskCountByStatus(SendTask::TaskStatus::INPROGRESS);
    int done = SendTask::getTaskCountByStatus(SendTask::TaskStatus::DONE);
    int failed = SendTask::getTaskCountByStatus(SendTask::TaskStatus::FAILED);
    int paused = SendTask::getTaskCountByStatus(SendTask::TaskStatus::PAUSED);
    int external = SendTask::getTaskCountByStatus(SendTask::TaskStatus::EXTERNAL_TASK);

    logger->info("Status Summary - Waiting: %d, Running: %d, Done: %d, Failed: %d, Paused: %d, External: %d",
                waiting, inProgress, done, failed, paused, external);

    // Print tasks by core
    auto cpu0Tasks = SendTask::getTasksByCore(0);
    auto cpu1Tasks = SendTask::getTasksByCore(1);
    auto anyCoreTasks = SendTask::getTasksByCore(tskNO_AFFINITY);

    logger->info("CPU 0 tasks: %d, CPU 1 tasks: %d, Any core tasks: %d",
                cpu0Tasks.size(), cpu1Tasks.size(), anyCoreTasks.size());

    // Calculate total memory usage
    uint32_t totalStackAllocated = 0;
    uint32_t totalStackUsed = 0;
    for (const auto& task : allTasks) {
        totalStackAllocated += task.stackSize;
        totalStackUsed += task.stackUsed;
    }

    logger->info("Memory Usage - Total Stack: %u bytes, Used: %u bytes (%.1f%%)",
                totalStackAllocated, totalStackUsed,
                totalStackAllocated > 0 ? (float)totalStackUsed * 100.0f / totalStackAllocated : 0.0f);

    // Print detailed task information
    for (const auto& task : allTasks) {
        const char* statusStr = "UNKNOWN";
        switch (task.status) {
            case SendTask::TaskStatus::WAITING: statusStr = "WAITING"; break;
            case SendTask::TaskStatus::INPROGRESS: statusStr = "RUNNING"; break;
            case SendTask::TaskStatus::DONE: statusStr = "DONE"; break;
            case SendTask::TaskStatus::FAILED: statusStr = "FAILED"; break;
            case SendTask::TaskStatus::PAUSED: statusStr = "PAUSED"; break;
            case SendTask::TaskStatus::EXTERNAL_TASK: statusStr = "EXTERNAL"; break;
        }

        unsigned long runtime = 0;
        if (task.startedAt > 0) {
            if (task.completedAt > 0) {
                runtime = task.completedAt - task.startedAt;
            } else {
                runtime = millis() - task.startedAt;
            }
        }

        const char* taskType = task.isExternal ? "EXT" : "INT";

        // Calculate memory usage percentage
        float memUsagePercent = 0.0f;
        if (task.stackSize > 0) {
            memUsagePercent = (float)task.stackUsed * 100.0f / task.stackSize;
        }

        logger->info("Task: %s [%s] (%s) - Status: %s, Core: %d, Priority: %d, Runtime: %lums, Memory: %u/%u bytes (%.1f%% used), Free: %u bytes%s%s",
                task.name.c_str(), task.taskId.c_str(), taskType, statusStr,
                task.coreId, task.priority, runtime,
                task.stackUsed, task.stackSize, memUsagePercent, task.stackFreeMin,
                (task.isExternal && (task.name == "cam_task" || task.name.indexOf("camera") >= 0)) ? " [CAMERA]" : "",
                (memUsagePercent > 80.0f) ? " [HIGH MEM!]" : "");
    }

    logger->info("=== End Task Status Report ===");
}

/**
 * Clean up completed and failed tasks to free memory
 */
void cleanupTasks() {
    int beforeCount = SendTask::getTaskCount();
    SendTask::cleanupCompletedTasks();
    int afterCount = SendTask::getTaskCount();

    int cleanedUp = beforeCount - afterCount;
    if (cleanedUp > 0) {
        logger->info("Cleaned up %d completed/failed tasks", cleanedUp);
    }
}
