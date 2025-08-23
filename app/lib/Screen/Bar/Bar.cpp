// filepath: /apps/PlatformIO/cozmo-system/app/lib/Screen/Bar/Bar.cpp
#include "Bar.h"
#include "setup/setup.h"

MicBar::MicBar(U8G2_SSD1306_128X64_NONAME_F_HW_I2C *display): _display(display) {
	// Constructor implementation
}

void MicBar::drawBar() {
	if (_display == nullptr || microphoneSensor == nullptr) return;

	// Read current level from microphone sensor
	int micLevel = microphoneSensor->readLevel();
	int barWidth = map(micLevel, 0, 4095, 0, 97);
	
	// Calculate center position and draw box from center
	int centerX = 15 + (97 / 2);
	int halfBarWidth = barWidth / 2;
	
	// Draw from center outwards
	_display->drawBox(centerX - halfBarWidth, 62, barWidth, 2);
}