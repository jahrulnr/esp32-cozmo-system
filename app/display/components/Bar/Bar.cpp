// filepath: /apps/PlatformIO/cozmo-system/app/lib/Screen/Bar/Bar.cpp
#include "Bar.h"

MicBar::MicBar(U8G2_SSD1306_128X64_NONAME_F_HW_I2C *display): _display(display) {
	// Constructor implementation
}

// micLevel 0-4096 range
void MicBar::drawBar(int micLevel) {
	// Read current level from microphone sensor
	int barWidth = map(micLevel, 0, 4095, 0, 97);

	// Calculate center position and draw box from center
	int centerX = 15 + (97 / 2);
	int halfBarWidth = barWidth / 2;

	// Draw from center outwards
	_display->drawBox(centerX - halfBarWidth, 60, barWidth, 2);
}