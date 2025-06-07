#include "app.h"

void setupCliffDetector(){
	cliffLeftDetector = new Sensors::CliffDetector();
	cliffRightDetector = new Sensors::CliffDetector();

	#if CLIFF_DETECTOR_ENABLED
	cliffLeftDetector->init(CLIFF_LEFT_DETECTOR_PIN);
	cliffRightDetector->init(CLIFF_RIGHT_DETECTOR_PIN);
	#endif
}

bool cliffDetected() {
	bool leftCliff = cliffLeftDetector ? cliffLeftDetector->isCliffDetected() : false;
	bool rightCliff = cliffRightDetector ? cliffRightDetector->isCliffDetected() : false;
	return leftCliff || rightCliff;
}