#pragma once
#include <U8g2lib.h>
#include "../../Icons.h"
#include <WiFi.h>

class MicStatus {
private:
	U8G2_SSD1306_128X64_NONAME_F_HW_I2C *_display;
public:
	MicStatus(U8G2_SSD1306_128X64_NONAME_F_HW_I2C *display): _display(display){}
	~MicStatus() { delete [] _display; }

	// range 0~2
	void Draw(int micStatus = 0) {
		_display->setBitmapMode(1);

		switch (micStatus)
		{
		case 1: // idle
			_display->drawXBM(51, 10, 27, 45, full_icon::microphone_1_bits);
			break;
		case 2: // speak / noise
			_display->drawXBM(45, 10, 39, 45, full_icon::microphone_recording_bits);
			break;

		default: // disable
			_display->drawXBM(49, 17, 30, 30, big_icon::microphone_muted_bits);
			break;
		}

	}
};