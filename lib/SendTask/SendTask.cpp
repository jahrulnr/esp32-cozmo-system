#include "SendTask.h"

namespace SendTask {

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

	String createTask(TaskFunction function, const TaskConfig& config) {
		ensureMutexInitialized();

		String taskId = generateTaskId();

		// Create task info
		TaskInfo taskInfo;
		taskInfo.taskId = taskId;
		taskInfo.name = config.name;
		taskInfo.status = TaskStatus::WAITING;
		taskInfo.createdAt = millis();
		taskInfo.startedAt = 0;
		taskInfo.completedAt = 0;
		taskInfo.description = config.description.isEmpty() ? config.name : config.description;
		taskInfo.handle = nullptr;
		taskInfo.coreId = config.coreId;
		taskInfo.priority = config.priority;
		taskInfo.isLoop = false;
		taskInfo.stackSize = config.stackSize;
		taskInfo.stackFreeMin = 0;
		taskInfo.stackUsed = 0;

		// Store task info in registry
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			taskRegistry[taskId] = taskInfo;
			xSemaphoreGive(registryMutex);
		}

		// Prepare task parameters
		struct TaskParams {
			TaskFunction function;
			String taskId;
		};

		TaskParams* params = new TaskParams{function, taskId};

		// Create FreeRTOS task
		TaskHandle_t taskHandle;
		BaseType_t result;

		if (config.coreId == tskNO_AFFINITY) {
			result = xTaskCreateWithCaps([](void* param) {
				TaskParams* taskParams = static_cast<TaskParams*>(param);
				String currentTaskId = taskParams->taskId;

				// Update status to in progress
				updateTaskStatus(currentTaskId, TaskStatus::INPROGRESS);

				try {
					// Execute the function
					taskParams->function();
					// Update status to done
					updateTaskStatus(currentTaskId, TaskStatus::DONE);
				} catch (...) {
					// Update status to failed
					updateTaskStatus(currentTaskId, TaskStatus::FAILED);
				}

				// Cleanup
				delete taskParams;
				vTaskDeleteWithCaps(NULL);
			},
			config.name.c_str(),
			config.stackSize,
			params,
			config.priority,
			&taskHandle,
			MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
		} else {
			result = xTaskCreatePinnedToCoreWithCaps([](void* param) {
				TaskParams* taskParams = static_cast<TaskParams*>(param);
				String currentTaskId = taskParams->taskId;

				// Update status to in progress
				updateTaskStatus(currentTaskId, TaskStatus::INPROGRESS);

				try {
					// Execute the function
					taskParams->function();
					// Update status to done
					updateTaskStatus(currentTaskId, TaskStatus::DONE);
				} catch (...) {
					// Update status to failed
					updateTaskStatus(currentTaskId, TaskStatus::FAILED);
				}

				// Cleanup
				delete taskParams;
				vTaskDeleteWithCaps(NULL);
			},
			config.name.c_str(),
			config.stackSize,
			params,
			config.priority,
			&taskHandle,
			config.coreId,
			MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
		}

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

	String createLoopTask(LoopTaskFunction function, const TaskConfig& config) {
		ensureMutexInitialized();

		String taskId = generateTaskId();

		// Create task info
		TaskInfo taskInfo;
		taskInfo.taskId = taskId;
		taskInfo.name = config.name;
		taskInfo.status = TaskStatus::WAITING;
		taskInfo.createdAt = millis();
		taskInfo.startedAt = 0;
		taskInfo.completedAt = 0;
		taskInfo.description = config.description.isEmpty() ? config.name : config.description;
		taskInfo.handle = nullptr;
		taskInfo.coreId = config.coreId;
		taskInfo.priority = config.priority;
		taskInfo.isLoop = true;
		taskInfo.stackSize = config.stackSize;
		taskInfo.stackFreeMin = 0;
		taskInfo.stackUsed = 0;

		// Store task info in registry
		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			taskRegistry[taskId] = taskInfo;
			xSemaphoreGive(registryMutex);
		}

		// Prepare task parameters
		struct LoopTaskParams {
			LoopTaskFunction function;
			String taskId;
			void* userParams;
		};

		LoopTaskParams* params = new LoopTaskParams{function, taskId, config.params};

		// Create FreeRTOS task
		TaskHandle_t taskHandle;
		BaseType_t result;

		if (config.coreId == tskNO_AFFINITY) {
			result = xTaskCreateWithCaps([](void* param) {
				LoopTaskParams* taskParams = static_cast<LoopTaskParams*>(param);
				String currentTaskId = taskParams->taskId;

				// Update status to in progress
				updateTaskStatus(currentTaskId, TaskStatus::INPROGRESS);

				try {
					// Execute the loop function (this should run indefinitely)
					taskParams->function(taskParams->userParams);
					// If we reach here, the task finished normally
					updateTaskStatus(currentTaskId, TaskStatus::DONE);
				} catch (...) {
					// Update status to failed
					updateTaskStatus(currentTaskId, TaskStatus::FAILED);
				}

				// Cleanup
				delete taskParams;
				vTaskDeleteWithCaps(NULL);
			},
			config.name.c_str(),
			config.stackSize,
			params,
			config.priority,
			&taskHandle,
			MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT
			);
		} else {
			result = xTaskCreatePinnedToCore([](void* param) {
				LoopTaskParams* taskParams = static_cast<LoopTaskParams*>(param);
				String currentTaskId = taskParams->taskId;

				// Update status to in progress
				updateTaskStatus(currentTaskId, TaskStatus::INPROGRESS);

				try {
					// Execute the loop function (this should run indefinitely)
					taskParams->function(taskParams->userParams);
					// If we reach here, the task finished normally
					updateTaskStatus(currentTaskId, TaskStatus::DONE);
				} catch (...) {
					// Update status to failed
					updateTaskStatus(currentTaskId, TaskStatus::FAILED);
				}

				// Cleanup
				delete taskParams;
				vTaskDelete(NULL);
			},
			config.name.c_str(),
			config.stackSize,
			params,
			config.priority,
			&taskHandle,
			config.coreId);
		}

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

	String createTaskOnCore(TaskFunction function, const String& name, uint32_t stackSize,
	                       UBaseType_t priority, BaseType_t coreId, const String& description) {
		TaskConfig config;
		config.name = name;
		config.stackSize = stackSize;
		config.priority = priority;
		config.coreId = coreId;
		config.description = description;
		return createTask(function, config);
	}

	String createLoopTaskOnCore(LoopTaskFunction function, const String& name, uint32_t stackSize,
	                           UBaseType_t priority, BaseType_t coreId, const String& description, void* params) {
		TaskConfig config;
		config.name = name;
		config.stackSize = stackSize;
		config.priority = priority;
		config.coreId = coreId;
		config.description = description;
		config.params = params;
		config.isLoop = true;
		return createLoopTask(function, config);
	}

	TaskStatus getTaskStatus(const String& taskId) {
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

	TaskInfo getTaskInfo(const String& taskId) {
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

	std::vector<TaskInfo> getAllTasks() {
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

	std::vector<TaskInfo> getTasksByStatus(TaskStatus status) {
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

	std::vector<TaskInfo> getTasksByCore(BaseType_t coreId) {
		ensureMutexInitialized();
		std::vector<TaskInfo> tasks;

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			for (const auto& pair : taskRegistry) {
				if (pair.second.coreId == coreId) {
					tasks.push_back(pair.second);
				}
			}
			xSemaphoreGive(registryMutex);
		}

		return tasks;
	}

	bool stopTask(const String& taskId, bool removeFromRegistry) {
		ensureMutexInitialized();
		bool stopped = false;

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end() && it->second.handle != nullptr) {
				// Allow stopping external tasks with safety checks
				if (it->second.isExternal) {
					// Check if it's a critical system task that shouldn't be stopped
					String taskName = it->second.name;
					taskName.toLowerCase();
					if (taskName.indexOf("idle") >= 0 || taskName.indexOf("timer") >= 0 ||
					    taskName.indexOf("ipc") >= 0 || taskName.indexOf("sys_evt") >= 0) {
						// Don't stop critical system tasks
						xSemaphoreGive(registryMutex);
						return false;
					}

					// Stop external task
					vTaskDelete(it->second.handle);
					it->second.handle = nullptr;
					it->second.status = TaskStatus::FAILED;
					it->second.completedAt = millis();
					stopped = true;

					if (removeFromRegistry) {
						taskRegistry.erase(it);
					}
				} else {
					// Normal SendTask created tasks
					if (it->second.status == TaskStatus::INPROGRESS ||
					    it->second.status == TaskStatus::WAITING ||
					    it->second.status == TaskStatus::PAUSED) {

						// Stop the task
						vTaskDeleteWithCaps(it->second.handle);
						it->second.handle = nullptr;
						it->second.status = TaskStatus::FAILED;
						it->second.completedAt = millis();
						stopped = true;

						// Optionally remove from registry
						if (removeFromRegistry) {
							taskRegistry.erase(it);
						}
					}
				}
			}
			xSemaphoreGive(registryMutex);
		}

		return stopped;
	}

	bool pauseTask(const String& taskId) {
		ensureMutexInitialized();
		bool paused = false;

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end() && it->second.handle != nullptr) {
				// Allow pausing external tasks with safety checks
				if (it->second.isExternal) {
					String taskName = it->second.name;
					taskName.toLowerCase();
					if (taskName.indexOf("idle") >= 0 || taskName.indexOf("timer") >= 0 ||
					    taskName.indexOf("ipc") >= 0 || taskName.indexOf("sys_evt") >= 0) {
						// Don't pause critical system tasks
						xSemaphoreGive(registryMutex);
						return false;
					}
				}

				if (it->second.status == TaskStatus::INPROGRESS || it->second.status == TaskStatus::EXTERNAL_TASK) {
					vTaskSuspend(it->second.handle);
					it->second.status = TaskStatus::PAUSED;
					paused = true;
				}
			}
			xSemaphoreGive(registryMutex);
		}

		return paused;
	}

	bool resumeTask(const String& taskId) {
		ensureMutexInitialized();
		bool resumed = false;

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end() && it->second.handle != nullptr) {
				if (it->second.status == TaskStatus::PAUSED) {
					vTaskResume(it->second.handle);
					// Restore previous status
					if (it->second.isExternal) {
						it->second.status = TaskStatus::EXTERNAL_TASK;
					} else {
						it->second.status = TaskStatus::INPROGRESS;
					}
					resumed = true;
				}
			}
			xSemaphoreGive(registryMutex);
		}

		return resumed;
	}

	void cleanupCompletedTasks() {
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

	bool removeTask(const String& taskId) {
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

	int getTaskCount() {
		ensureMutexInitialized();
		int count = 0;

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			count = taskRegistry.size();
			xSemaphoreGive(registryMutex);
		}

		return count;
	}

	int getTaskCountByStatus(TaskStatus status) {
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

	void scanExternalTasks() {
		ensureMutexInitialized();

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			// Get number of tasks
			UBaseType_t taskCount = uxTaskGetNumberOfTasks();

			// Allocate memory for task status array
			TaskStatus_t* taskStatusArray = (TaskStatus_t*)malloc(taskCount * sizeof(TaskStatus_t));
			if (taskStatusArray != nullptr) {
				// Get task list
				UBaseType_t actualCount = uxTaskGetSystemState(taskStatusArray, taskCount, nullptr);

				for (UBaseType_t i = 0; i < actualCount; i++) {
					TaskHandle_t handle = taskStatusArray[i].xHandle;
					const char* taskName = taskStatusArray[i].pcTaskName;
					String taskId = "ext_" + String(taskName) + "_" + String((uint32_t)handle);

					// Check if this task is already in our registry
					bool alreadyTracked = false;
					for (const auto& pair : taskRegistry) {
						if (pair.second.handle == handle || pair.second.taskId == taskId) {
							alreadyTracked = true;
							break;
						}
					}

					// If not tracked and not our own tasks, add as external
					if (!alreadyTracked && String(taskName) != "SendTaskInternal") {
						TaskInfo externalTask;
						externalTask.taskId = taskId;
						externalTask.name = String(taskName);

						// Map FreeRTOS state to our task status
						switch (taskStatusArray[i].eCurrentState) {
							case eRunning:
								externalTask.status = TaskStatus::INPROGRESS;
								break;
							case eReady:
								externalTask.status = TaskStatus::WAITING;
								break;
							case eBlocked:
								externalTask.status = TaskStatus::WAITING;
								break;
							case eSuspended:
								externalTask.status = TaskStatus::PAUSED;
								break;
							case eDeleted:
								externalTask.status = TaskStatus::DONE;
								break;
							default:
								externalTask.status = TaskStatus::EXTERNAL_TASK;
								break;
						}

						externalTask.createdAt = millis();
						externalTask.startedAt = millis();
						externalTask.completedAt = 0;
						externalTask.description = "External FreeRTOS task";
						externalTask.handle = handle;
						externalTask.coreId = taskStatusArray[i].xCoreID;
						externalTask.priority = taskStatusArray[i].uxCurrentPriority;
						externalTask.isLoop = true; // Most external tasks are loops
						externalTask.isExternal = true;
						externalTask.stackSize = 0; // Will be updated by memory tracking
						externalTask.stackFreeMin = 0;
						externalTask.stackUsed = 0;

						taskRegistry[taskId] = externalTask;
					} else if (alreadyTracked) {
						// Update existing external task state
						for (auto& pair : taskRegistry) {
							if (pair.second.handle == handle && pair.second.isExternal) {
								// Update the status based on current FreeRTOS state
								switch (taskStatusArray[i].eCurrentState) {
									case eRunning:
										pair.second.status = TaskStatus::INPROGRESS;
										break;
									case eReady:
										pair.second.status = TaskStatus::WAITING;
										break;
									case eBlocked:
										pair.second.status = TaskStatus::WAITING;
										break;
									case eSuspended:
										pair.second.status = TaskStatus::PAUSED;
										break;
									case eDeleted:
										pair.second.status = TaskStatus::DONE;
										break;
									default:
										pair.second.status = TaskStatus::EXTERNAL_TASK;
										break;
								}

								// Update priority and core (in case they changed)
								pair.second.priority = taskStatusArray[i].uxCurrentPriority;
								pair.second.coreId = taskStatusArray[i].xCoreID;
								break;
							}
						}
					}
				}

				free(taskStatusArray);
			}
			xSemaphoreGive(registryMutex);
		}
	}

	std::vector<TaskInfo> getExternalTasks() {
		ensureMutexInitialized();
		std::vector<TaskInfo> externalTasks;

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			for (const auto& pair : taskRegistry) {
				if (pair.second.isExternal) {
					externalTasks.push_back(pair.second);
				}
			}
			xSemaphoreGive(registryMutex);
		}

		return externalTasks;
	}

	bool isTaskExternal(const String& taskId) {
		ensureMutexInitialized();
		bool isExternal = false;

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end()) {
				isExternal = it->second.isExternal;
			}
			xSemaphoreGive(registryMutex);
		}

		return isExternal;
	}

	void updateTaskMemoryUsage(const String& taskId) {
		ensureMutexInitialized();

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end() && it->second.handle != nullptr) {
				TaskHandle_t handle = it->second.handle;

				// Get stack high water mark (minimum free stack)
				UBaseType_t freeStackWords = uxTaskGetStackHighWaterMark(handle);
				it->second.stackFreeMin = freeStackWords * sizeof(StackType_t);

				// For external tasks, try to get stack size from task creation
				if (it->second.isExternal && it->second.stackSize == 0) {
					// Estimate stack size based on typical ESP32 task stacks
					// This is an approximation since we can't get exact allocated size
					if (it->second.name.indexOf("SR") >= 0) {
						it->second.stackSize = 8192; // Speech recognition tasks typically use 8KB
					} else if (it->second.name.indexOf("wifi") >= 0 || it->second.name.indexOf("tcp") >= 0) {
						it->second.stackSize = 4096; // Network tasks typically 4KB
					} else if (it->second.name.indexOf("IDLE") >= 0) {
						it->second.stackSize = 1536; // IDLE tasks use minimal stack
					} else {
						it->second.stackSize = 2048; // Default estimation
					}
				}

				// Calculate used stack - make sure we don't get invalid values
				if (it->second.stackSize > 0 && it->second.stackFreeMin <= it->second.stackSize) {
					it->second.stackUsed = it->second.stackSize - it->second.stackFreeMin;
				} else {
					// Fallback: if we can't calculate properly, use high water mark as used
					it->second.stackUsed = it->second.stackFreeMin;
					if (it->second.stackSize == 0) {
						it->second.stackSize = it->second.stackFreeMin + 1024; // Estimate
					}
				}
			}
			xSemaphoreGive(registryMutex);
		}
	}

	void updateAllTasksMemoryUsage() {
		ensureMutexInitialized();

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			for (auto& pair : taskRegistry) {
				if (pair.second.handle != nullptr) {
					TaskHandle_t handle = pair.second.handle;

					// Get stack high water mark
					UBaseType_t freeStackWords = uxTaskGetStackHighWaterMark(handle);
					pair.second.stackFreeMin = freeStackWords * sizeof(StackType_t);

					// Update stack size estimates for external tasks
					if (pair.second.isExternal && pair.second.stackSize == 0) {
						if (pair.second.name.indexOf("SR") >= 0) {
							pair.second.stackSize = 8192;
						} else if (pair.second.name.indexOf("wifi") >= 0 || pair.second.name.indexOf("tcp") >= 0) {
							pair.second.stackSize = 4096;
						} else if (pair.second.name.indexOf("IDLE") >= 0) {
							pair.second.stackSize = 1536;
						} else {
							pair.second.stackSize = 2048;
						}
					}

					// Calculate used stack - ensure valid values
					if (pair.second.stackSize > 0 && pair.second.stackFreeMin <= pair.second.stackSize) {
						pair.second.stackUsed = pair.second.stackSize - pair.second.stackFreeMin;
					} else {
						// Fallback for invalid calculations
						pair.second.stackUsed = pair.second.stackFreeMin;
						if (pair.second.stackSize == 0) {
							pair.second.stackSize = pair.second.stackFreeMin + 1024; // Estimate
						}
					}
				}
			}
			xSemaphoreGive(registryMutex);
		}
	}

	bool deleteExternalTask(const String& taskId) {
		ensureMutexInitialized();
		bool deleted = false;

		if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdTRUE) {
			auto it = taskRegistry.find(taskId);
			if (it != taskRegistry.end() && it->second.isExternal && it->second.handle != nullptr) {
				// Enhanced safety checks for critical system tasks
				String taskName = it->second.name;
				taskName.toLowerCase();
				if (taskName.indexOf("idle") >= 0 ||
				    taskName.indexOf("timer") >= 0 ||
				    taskName.indexOf("ipc") >= 0 ||
				    taskName.indexOf("sys_evt") >= 0 ||
				    taskName.indexOf("arduino_events") >= 0) {
					// Absolutely don't delete critical system tasks
					xSemaphoreGive(registryMutex);
					return false;
				}

				// Safe to delete non-critical external tasks
				vTaskDelete(it->second.handle);
				taskRegistry.erase(it);
				deleted = true;
			}
			xSemaphoreGive(registryMutex);
		}

		return deleted;
	}

}