#include "../register.h"

void displayTask(void *param){
		TickType_t lastWakeTime = xTaskGetTickCount();
		TickType_t updateFrequency = pdMS_TO_TICKS(50);

		size_t updateDelay = 0;
		EVENT_DISPLAY lastEvent = EVENT_DISPLAY::NOTHING;
		display->enableMutex();
		while(1) {
				vTaskDelayUntil(&lastWakeTime, updateFrequency);

				if (updateDelay > 0 && updateDelay <= millis()) {
					updateDelay = 0;
					lastEvent = EVENT_DISPLAY::NOTHING;
					display->setState(Display::STATE_FACE);
					display->getFace()->LookFront();
					display->getFace()->Expression.GoTo_Normal();
					display->autoFace(true);
				}

				if (notification->has(NOTIFICATION_DISPLAY)){
					void* eventPtr = notification->consume(NOTIFICATION_DISPLAY, updateFrequency);
					EVENT_DISPLAY event = (EVENT_DISPLAY)(intptr_t)eventPtr;
					if (event >= 0 && event <= EVENT_DISPLAY::NOTHING) {
						lastEvent = event;
					}
					logger->info("Event Screen %d triggered", lastEvent);
				}

				if (lastEvent == EVENT_DISPLAY::WAKEWORD && updateDelay == 0) {
						display->setState(Display::STATE_FACE);
						updateDelay = millis() + 3000;
						display->getFace()->LookFront();
						display->getFace()->Expression.GoTo_Happy();
						display->autoFace(false);
				}
				else if (lastEvent == EVENT_DISPLAY::LOOK_LEFT && updateDelay == 0) {
						display->setState(Display::STATE_FACE);
						display->getFace()->Expression.GoTo_Happy();
						display->autoFace(false);
				}
				else if (lastEvent == EVENT_DISPLAY::LOOK_LEFT && updateDelay == 0) {
						display->setState(Display::STATE_FACE);
						updateDelay = millis() + 6000;
						display->getFace()->LookLeft();
				}
				else if (lastEvent == EVENT_DISPLAY::LOOK_RIGHT && updateDelay == 0) {
						display->setState(Display::STATE_FACE);
						updateDelay = millis() + 6000;
						display->getFace()->LookRight();
				}
				else if (lastEvent == EVENT_DISPLAY::CLOSE_EYE && updateDelay == 0) {
						display->setState(Display::STATE_FACE);
						updateDelay = millis() + 6000;
						lastEvent = EVENT_DISPLAY::CLOSE_EYE;
						display->getFace()->LookFront();
						display->getFace()->Expression.GoTo_Sleepy();
				}
				else if (lastEvent == EVENT_DISPLAY::CLIFF_DETECTED && updateDelay == 0) {
						display->drawCenteredText(20, "Oops! Not a safe area.");
						updateDelay = millis() + 3000;
				}
				else if (lastEvent == EVENT_DISPLAY::OBSTACLE_DETECTED && updateDelay == 0) {
						display->drawCenteredText(20, "Oops! Finding another way!");
						updateDelay = millis() + 3000;
				}
				else if (lastEvent == EVENT_DISPLAY::STUCK_DETECTED && updateDelay == 0) {
						display->drawCenteredText(20, "I am stuck!");
						updateDelay = millis() + 3000;
				} 
				else if (lastEvent == EVENT_DISPLAY::TOUCH_DETECTED && updateDelay == 0) {
						display->setState(Display::STATE_MOCHI);
						updateDelay = 1; // for trigger default screen
				}
				else if (lastEvent == EVENT_DISPLAY::WEATHER_STATUS && updateDelay == 0) {
						display->setState(Display::STATE_WEATHER);
				}
				else if (lastEvent == EVENT_DISPLAY::ORIENTATION_DISPLAY && updateDelay == 0) {
						display->setState(Display::STATE_ORIENTATION);
				}
				else if (lastEvent == EVENT_DISPLAY::SPACE_GAME && updateDelay == 0) {
						display->setState(Display::STATE_SPACE_GAME);
				}
				else if (lastEvent == EVENT_DISPLAY::RECORDING_STARTED && updateDelay == 0) {
						display->setState(Display::STATE_TEXT);
						display->clearBuffer();
						display->drawCenteredText(20, "Recording...");
						display->drawCenteredText(40, "10 seconds");
						updateDelay = millis() + 2000;
				}
				else if (lastEvent == EVENT_DISPLAY::RECORDING_STOPPED && updateDelay == 0) {
						display->setState(Display::STATE_TEXT);
						display->clearBuffer();
						display->drawCenteredText(20, "Recording");
						display->drawCenteredText(40, "Complete!");
						updateDelay = millis() + 2000; // Show for 2 seconds then return to face
				}
				
		#if MICROPHONE_ENABLED
			#if MICROPHONE_ANALOG
				display->setMicLevel(amicrophone->readLevel());
			#elif MICROPHONE_I2S
				display->setMicLevel(microphone->readLevel());
			#endif
		#endif

		// Update orientation data if orientation sensor is available and display is in orientation mode
		#if ORIENTATION_ENABLED
			if (orientation && display) {
				display->updateOrientation(orientation);
			}
		#endif
			
			display->update();
		}
}