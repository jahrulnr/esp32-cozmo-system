#pragma once
#include <U8g2lib.h>
#include "../../Icons.h"
#include <WiFi.h>

class Status {
private:
	U8G2_SSD1306_128X64_NONAME_F_HW_I2C *_display;
public:
	Status(U8G2_SSD1306_128X64_NONAME_F_HW_I2C *display): _display(display){}
	~Status() { delete [] _display; }

	void Draw() {
		_display->setFontMode(1);
		_display->setBitmapMode(1);
		_display->setFont(u8g2_font_6x13_tr);

		_display->drawXBM(5, 4, 16, 16, big_icon::file_save_bits);
		_display->drawStr(32, 6, "LittleS");
		_display->drawXBM(6, 24, 14, 16, big_icon::bluetooth_bits);
		_display->drawStr(32, 26, "Disable");
		_display->drawXBM(4, 44, 19, 16, big_icon::wifi_5_bars_bits);
		_display->drawStr(32, 50, WiFi.localIP().toString().c_str());

	}
};