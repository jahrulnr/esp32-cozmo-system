#pragma once

#include <Arduino.h>
#include <map>
#include <vector>

namespace Command {
	using cmd = std::function<void(void)>;
	
	enum class TaskStatus {
		WAITING,
		INPROGRESS,
		DONE,
		FAILED
	};
	
	struct TaskInfo {
		String taskId;
		TaskStatus status;
		unsigned long createdAt;
		unsigned long startedAt;
		unsigned long completedAt;
		String description;
		TaskHandle_t handle;
	};
	
	String Send(cmd command, int priority = configMAX_PRIORITIES - 1, const String& description = "");
	TaskStatus GetTaskStatus(const String& taskId);
	TaskInfo GetTaskInfo(const String& taskId);
	std::vector<TaskInfo> GetAllTasks();
	std::vector<TaskInfo> GetTasksByStatus(TaskStatus status);
	void CleanupCompletedTasks();
	bool RemoveTask(const String& taskId);
	int GetTaskCount();
	int GetTaskCountByStatus(TaskStatus status);
}