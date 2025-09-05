#include "../register.h"

void displayTask(void *param){
		TickType_t lastWakeTime = xTaskGetTickCount();
		TickType_t updateFrequency = pdMS_TO_TICKS(50);

		size_t updateDelay = 0;
		const char* lastEvent;
		display->enableMutex();
		while(1) {
				vTaskDelayUntil(&lastWakeTime, updateFrequency);

				if (notification->has(NOTIFICATION_DISPLAY)) {
					const char * event = (char *)notification->consume(NOTIFICATION_DISPLAY, updateFrequency);
					logger->info("Event Screen %s triggered", event);
					if (strcmp(event, EVENT_DISPLAY::WAKEWORD) == 0 && updateDelay == 0) {
							updateDelay = millis() + 3000;
							lastEvent = EVENT_DISPLAY::WAKEWORD;
							display->getFace()->LookFront();
							display->getFace()->Expression.GoTo_Happy();
					}
					else if (strcmp(event, EVENT_DISPLAY::LOOK_LEFT) == 0) {
							updateDelay = millis() + 6000;
							lastEvent = EVENT_DISPLAY::LOOK_LEFT;
							display->getFace()->LookLeft();
					}
					else if (strcmp(event, EVENT_DISPLAY::LOOK_RIGHT) == 0) {
							updateDelay = millis() + 6000;
							lastEvent = EVENT_DISPLAY::LOOK_RIGHT;
							display->getFace()->LookRight();
					}
					else if (strcmp(event, EVENT_DISPLAY::CLOSE_EYE) == 0) {
							updateDelay = millis() + 6000;
							lastEvent = EVENT_DISPLAY::CLOSE_EYE;
							display->getFace()->LookFront();
							display->getFace()->Expression.GoTo_Sleepy();
					}
					else if (strcmp(event, EVENT_DISPLAY::CLIFF_DETECTED) == 0) {
	            display->drawCenteredText(20, "Oops! Not a safe area.");
					}
					else if (strcmp(event, EVENT_DISPLAY::OBSTACLE_DETECTED) == 0) {
	            display->drawCenteredText(20, "Oops! Finding another way!");
					}
					else if (strcmp(event, EVENT_DISPLAY::STUCK_DETECTED) == 0) {
	            display->drawCenteredText(20, "I am stuck!");
					}
				}

				if (updateDelay > 0 && updateDelay <= millis()) {
					updateDelay = 0;
					lastEvent = "";
					display->getFace()->LookFront();
					display->getFace()->Expression.GoTo_Normal();
				}
				
		#if MICROPHONE_ENABLED
			#if MICROPHONE_ANALOG
				display->setMicLevel(amicrophone->readLevel());
			#elif MICROPHONE_I2S
				display->setMicLevel(microphone->readLevel());
			#endif
		#endif
			
			display->update();
		}
}