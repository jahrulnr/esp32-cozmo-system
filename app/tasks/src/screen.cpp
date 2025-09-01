#include "../register.h"

void screenTask(void *param){
		TickType_t lastWakeTime = xTaskGetTickCount();
		TickType_t updateFrequency = pdMS_TO_TICKS(50);

		size_t updateDelay = 0;
		const char* lastEvent;
		while(1) {
				vTaskDelayUntil(&lastWakeTime, updateFrequency);

				if (notification->has(NOTIFICATION_DISPLAY)) {
					const char * event = (char *)notification->consume(NOTIFICATION_DISPLAY, updateFrequency);
					logger->info("Event Screen %s triggered", event);
					if (strcmp(event, EVENT_DISPLAY_WAKEWORD) == 0 && updateDelay == 0) {
							updateDelay = millis() + 3000;
							lastEvent = EVENT_DISPLAY_WAKEWORD;
							screen->getFace()->LookFront();
							screen->getFace()->Expression.GoTo_Happy();
					}
					else if (strcmp(event, EVENT_DISPLAY_LOOK_LEFT) == 0) {
							updateDelay = millis() + 6000;
							lastEvent = EVENT_DISPLAY_LOOK_LEFT;
							screen->getFace()->LookLeft();
					}
					else if (strcmp(event, EVENT_DISPLAY_LOOK_RIGHT) == 0) {
							updateDelay = millis() + 6000;
							lastEvent = EVENT_DISPLAY_LOOK_RIGHT;
							screen->getFace()->LookRight();
					}
					else if (strcmp(event, EVENT_DISPLAY_CLOSE_EYE) == 0) {
							updateDelay = millis() + 6000;
							lastEvent = EVENT_DISPLAY_CLOSE_EYE;
							screen->getFace()->LookFront();
							screen->getFace()->Expression.GoTo_Sleepy();
					}
				}

				if (updateDelay > 0 && updateDelay <= millis()) {
					updateDelay = 0;
					lastEvent = "";
					screen->getFace()->LookFront();
					screen->getFace()->Expression.GoTo_Normal();
				}
				
		#if MICROPHONE_ENABLED
			#if MICROPHONE_ANALOG
				screen->setMicLevel(amicrophone->readLevel());
			#elif MICROPHONE_I2S
				screen->setMicLevel(microphone->readLevel());
			#endif
		#endif
				screen->mutexUpdate();
		}
}