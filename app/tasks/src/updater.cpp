#include "../register.h"
#include <SendTask.h>

void updaterTask(void* parameter) {
	logger->info("Updater task started");
	const int sendInterval = 10000;
	long currentUpdate = millis();

	float distance = -1.;
	float temperature = NAN;

	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(90);
	
  setupDistanceSensor();
  setupTouchDetector();
  setupTemperatureSensor();
  setupBatteryManager();

	while (true) {
		xTaskDelayUntil(&lastWakeTime, updateFrequency);
		ftpSrv.handleFTP();

		bool sendLog = millis() - currentUpdate > sendInterval;

		if (sendLog) {
			cleanupTasks();
			vTaskDelay(5);
		}

		// Gyroscope and accelerometer
		if (orientation) {
			orientation->update();

			if (sendLog)
				logger->info("gyro X: %.2f Y: %.2f Z: %.2f | accel X: %.2f Y: %.2f Z: %.2f | mag: %.2f",
					orientation->getX(), orientation->getY(), orientation->getZ(),
					orientation->getAccelX(), orientation->getAccelY(), orientation->getAccelZ(),
					orientation->getAccelMagnitude());
		}


		if (distanceSensor) {
			if (sendLog)
				logger->info("Distance: %.2f",
					distanceSensor->measureDistance());
		}

		// Cliff detectors
		if (cliffLeftDetector && cliffRightDetector) {
			cliffLeftDetector->update();
			cliffRightDetector->update();

			if (sendLog)
				logger->info("cliff R: %s L: %s",
					cliffRightDetector->isCliffDetected() ? "yes" : "no",
					cliffLeftDetector->isCliffDetected() ? "yes" : "no"
					);
		}

		if (touchDetector) {
			touchDetector->update();

			if (sendLog)
				logger->info("touched: %s", touchDetector->detected() ? "yes":"no");
		}

		if (temperatureSensor) {
			temperature = temperatureSensor->readTemperature();

			if (sendLog)
				logger->info("temperature: %.1fC", temperature);
		}

		if (batteryManager) {
			batteryHandler(sendLog);
		}

		if (display) {
			displayHandler();
		}

		if (weatherService && sendLog) {
			SendTask::createTaskOnCore([](void){
				weatherHandler();
				vTaskDeleteWithCaps(NULL);
			}, "weatherUpdate", 4096, 0, 0);
		}

		#if SPEAKER_ENABLED
			notePlayerHandler();
		#endif

		if (sendLog) {
			notification->send(NOTIFICATION_DL, dl_mode_t::DL_MODE_OFF);
		}

		if (sendLog) {
			currentUpdate = millis();
		}
	}
}