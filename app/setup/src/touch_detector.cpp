#include <setup/setup.h>

Sensors::TouchDetector* touchDetector;

void setupTouchDetector() {
	if (!touchDetector) {
		touchDetector = new Sensors::TouchDetector();
		touchDetector->init(48);
	}
}