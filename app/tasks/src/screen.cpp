#include "../register.h"

void displayTask(void *param){
		TickType_t lastWakeTime = xTaskGetTickCount();
		TickType_t updateFrequency = pdMS_TO_TICKS(50);
		const char* TAG = "displayTask";

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
					ESP_LOGI(TAG, "Reset Event Screen %d triggered", lastEvent);
				}

				if (notification->has(NOTIFICATION_DISPLAY)){
					void* eventPtr = notification->consume(NOTIFICATION_DISPLAY, updateFrequency);
					EVENT_DISPLAY event = (EVENT_DISPLAY)(intptr_t)eventPtr;
					if (event >= 0 && event <= EVENT_DISPLAY::NOTHING) {
						lastEvent = event;
						updateDelay = 0;
					}
					ESP_LOGI(TAG, "Event Screen %d triggered", lastEvent);
				}

				if (lastEvent != EVENT_DISPLAY::NOTHING && updateDelay == 0){
					switch(lastEvent) {
						case EVENT_DISPLAY::WAKEWORD:
								display->setState(Display::STATE_MIC);
								// update display will triggered after esp-sr timeout
						break;
						case EVENT_DISPLAY::FACE:
								display->setState(Display::STATE_FACE);
								display->getFace()->Expression.GoTo_Happy();
								display->autoFace(true);
						break;
						case EVENT_DISPLAY::BASIC_STATUS:
								display->setState(Display::STATE_STATUS);
								updateDelay = millis() + 6000;
						break;
						case EVENT_DISPLAY::WEATHER_STATUS:
								display->setState(Display::STATE_WEATHER);
						break;
						case EVENT_DISPLAY::ORIENTATION_DISPLAY:
								display->setState(Display::STATE_ORIENTATION);
						break;
						case EVENT_DISPLAY::SPACE_GAME:
								display->setState(Display::STATE_SPACE_GAME);
						break;
						case EVENT_DISPLAY::RECORDING_STARTED:
								display->setState(Display::STATE_TEXT);
								display->clearBuffer();
								display->drawCenteredText(20, "Recording...");
								display->drawCenteredText(40, "10 seconds");
								updateDelay = millis() + 2000;
						break;
						case EVENT_DISPLAY::RECORDING_STOPPED:
								display->setState(Display::STATE_TEXT);
								display->clearBuffer();
								display->drawCenteredText(20, "Recording");
								display->drawCenteredText(40, "Complete!");
								updateDelay = millis() + 2000; // Show for 2 seconds then return to face
						break;
						case EVENT_DISPLAY::BATTERY_STATUS:
								display->setState(Display::STATE_BATTERY);
								updateDelay = millis() + 5000; // Show for 5 seconds
						break;
						default:
							lastEvent = EVENT_DISPLAY::NOTHING;
					}
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