#include "SendTask.h"

namespace Command {
	
	static std::map<String, TaskInfo> taskRegistry;
	static SemaphoreHandle_t registryMutex = nullptr;
	static uint32_t taskCounter = 0;
	
	// Initialize mutex if not already done
	static void ensureMutexInitialized() {
		if (registryMutex == nullptr) {
			registryMutex = xSemaphoreCreateMutex();
		}
	}
	
	// Generate unique task ID
	static String generateTaskId() {
		return "task_" + String(millis()) + "_" + String(++taskCounter);
	}
	
	// Update task status safely
	static void updateTaskStatus(const String& taskId, TaskStatus status) {
		ensureMutexInitialized();
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end()) {
				it->second.status = status;
				unsigned long currentTime = millis();
				
				switch (status) {
					case TaskStatus::INPROGRESS:
						it->second.startedAt = currentTime;
						break;
					case TaskStatus::DONE:
					case TaskStatus::FAILED:
						it->second.completedAt = currentTime;
						break;
					default:
						break;
				}
			}
			xSemaphoreGive(registryMutex);
		}
	}

	String Send(cmd command, int priority, const String& description, uint32_t stackSize) {
		ensureMutexInitialized();
		
		String taskId = generateTaskId();
		
		// Create task info
		TaskInfo taskInfo;
		taskInfo.taskId = taskId;
		taskInfo.status = TaskStatus::WAITING;
		taskInfo.createdAt = millis();
		taskInfo.startedAt = 0;
		taskInfo.completedAt = 0;
		taskInfo.description = description.isEmpty() ? "Command Task" : description;
		taskInfo.handle = nullptr;
		
		// Store task info in registry
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			taskRegistry[taskId] = taskInfo;
			xSemaphoreGive(registryMutex);
		}
		
		// Prepare task parameters
		struct TaskParams {
			cmd command;
			String taskId;
		};
		
		TaskParams* params = new TaskParams{command, taskId};
		
		// Create FreeRTOS task
		TaskHandle_t taskHandle;
		BaseType_t result = xTaskCreatePinnedToCore([](void* param) {
			TaskParams* taskParams = static_cast<TaskParams*>(param);
			String currentTaskId = taskParams->taskId;
			
			// Update status to in progress
			updateTaskStatus(currentTaskId, TaskStatus::INPROGRESS);
			
			try {
				// Execute the command
				taskParams->command();
				// Update status to done
				updateTaskStatus(currentTaskId, TaskStatus::DONE);
			} catch (...) {
				// Update status to failed
				updateTaskStatus(currentTaskId, TaskStatus::FAILED);
			}
			
			// Cleanup
			delete taskParams;
			vTaskDelete(NULL);
		}, 
		"CommandTask",
		stackSize, 
		params, 
		priority, 
		&taskHandle,
		1);
		
		// Update task handle in registry
		if (result == pdPASS && xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end()) {
				it->second.handle = taskHandle;
			}
			xSemaphoreGive(registryMutex);
		}
		
		return taskId;
	}
	
	TaskStatus GetTaskStatus(const String& taskId) {
		ensureMutexInitialized();
		TaskStatus status = TaskStatus::FAILED;
		
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end()) {
				status = it->second.status;
			}
			xSemaphoreGive(registryMutex);
		}
		
		return status;
	}
	
	TaskInfo GetTaskInfo(const String& taskId) {
		ensureMutexInitialized();
		TaskInfo taskInfo;
		taskInfo.taskId = "";
		taskInfo.status = TaskStatus::FAILED;
		
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end()) {
				taskInfo = it->second;
			}
			xSemaphoreGive(registryMutex);
		}
		
		return taskInfo;
	}
	
	std::vector<TaskInfo> GetAllTasks() {
		ensureMutexInitialized();
		std::vector<TaskInfo> tasks;
		
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			for (const auto& pair : taskRegistry) {
				tasks.push_back(pair.second);
			}
			xSemaphoreGive(registryMutex);
		}
		
		return tasks;
	}
	
	std::vector<TaskInfo> GetTasksByStatus(TaskStatus status) {
		ensureMutexInitialized();
		std::vector<TaskInfo> tasks;
		
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			for (const auto& pair : taskRegistry) {
				if (pair.second.status == status) {
					tasks.push_back(pair.second);
				}
			}
			xSemaphoreGive(registryMutex);
		}
		
		return tasks;
	}
	
	void CleanupCompletedTasks() {
		ensureMutexInitialized();
		
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.begin();
			while (it != taskRegistry.end()) {
				if (it->second.status == TaskStatus::DONE || it->second.status == TaskStatus::FAILED) {
					it = taskRegistry.erase(it);
				} else {
					++it;
				}
			}
			xSemaphoreGive(registryMutex);
		}
	}
	
	bool RemoveTask(const String& taskId) {
		ensureMutexInitialized();
		bool removed = false;
		
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end()) {
				// Only remove if task is completed or failed
				if (it->second.status == TaskStatus::DONE || it->second.status == TaskStatus::FAILED) {
					taskRegistry.erase(it);
					removed = true;
				}
			}
			xSemaphoreGive(registryMutex);
		}
		
		return removed;
	}
	
	int GetTaskCount() {
		ensureMutexInitialized();
		int count = 0;
		
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			count = taskRegistry.size();
			xSemaphoreGive(registryMutex);
		}
		
		return count;
	}
	
	int GetTaskCountByStatus(TaskStatus status) {
		ensureMutexInitialized();
		int count = 0;
		
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			for (const auto& pair : taskRegistry) {
				if (pair.second.status == status) {
					count++;
				}
			}
			xSemaphoreGive(registryMutex);
		}
		
		return count;
	}

}