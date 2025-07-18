#ifndef BAR_H
#define BAR_H

#include <Arduino.h>
#include <U8g2lib.h>

class MicBar {
	private:
		int _micLevel = 0;
		U8G2_SSD1306_128X64_NONAME_F_HW_I2C *_display;
		
	public:
		MicBar(U8G2_SSD1306_128X64_NONAME_F_HW_I2C *display);
		void drawBar();
};

#endif // BAR_H