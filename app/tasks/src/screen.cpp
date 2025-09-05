#include "../register.h"

void displayTask(void *param){
		TickType_t lastWakeTime = xTaskGetTickCount();
		TickType_t updateFrequency = pdMS_TO_TICKS(50);

		size_t updateDelay = 0;
		EVENT_DISPLAY lastEvent = EVENT_DISPLAY::NOTHING;
		display->enableMutex();
		while(1) {
				vTaskDelayUntil(&lastWakeTime, updateFrequency);

				if (notification->has(NOTIFICATION_DISPLAY)){
					void* eventPtr = notification->consume(NOTIFICATION_DISPLAY, updateFrequency);
					EVENT_DISPLAY event = (EVENT_DISPLAY)(intptr_t)eventPtr;
					if (event >= 0 && event <= EVENT_DISPLAY::NOTHING) {
						lastEvent = event;
					}
					logger->info("Event Screen %d triggered", lastEvent);
				}

				if (lastEvent == EVENT_DISPLAY::WAKEWORD && updateDelay == 0) {
						updateDelay = millis() + 3000;
						display->getFace()->LookFront();
						display->getFace()->Expression.GoTo_Happy();
				}
				else if (lastEvent == EVENT_DISPLAY::LOOK_LEFT && updateDelay == 0) {
						updateDelay = millis() + 6000;
						display->getFace()->LookLeft();
				}
				else if (lastEvent == EVENT_DISPLAY::LOOK_RIGHT && updateDelay == 0) {
						updateDelay = millis() + 6000;
						display->getFace()->LookRight();
				}
				else if (lastEvent == EVENT_DISPLAY::CLOSE_EYE && updateDelay == 0) {
						updateDelay = millis() + 6000;
						lastEvent = EVENT_DISPLAY::CLOSE_EYE;
						display->getFace()->LookFront();
						display->getFace()->Expression.GoTo_Sleepy();
				}
				else if (lastEvent == EVENT_DISPLAY::CLIFF_DETECTED && updateDelay == 0) {
						display->drawCenteredText(20, "Oops! Not a safe area.");
						display->update();
						delay(3000);
						continue;
				}
				else if (lastEvent == EVENT_DISPLAY::OBSTACLE_DETECTED && updateDelay == 0) {
						display->drawCenteredText(20, "Oops! Finding another way!");
						display->update();
						delay(3000);
						continue;
				}
				else if (lastEvent == EVENT_DISPLAY::STUCK_DETECTED && updateDelay == 0) {
						display->drawCenteredText(20, "I am stuck!");
						display->update();
						delay(3000);
						continue;
				}

				if (updateDelay > 0 && updateDelay <= millis()) {
					updateDelay = 0;
					lastEvent = EVENT_DISPLAY::NOTHING;
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