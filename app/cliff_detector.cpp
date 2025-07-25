#include "app.h"

void setupCliffDetector(){
	cliffLeftDetector = new Sensors::CliffDetector();
	cliffRightDetector = new Sensors::CliffDetector();

	#if CLIFF_DETECTOR_ENABLED
	#if CLIFF_IO_EXTENDER
	cliffLeftDetector->initWithExtender(&ioExpander, CLIFF_LEFT_DETECTOR_PIN);
	cliffRightDetector->initWithExtender(&ioExpander, CLIFF_RIGHT_DETECTOR_PIN);
	#else
	cliffLeftDetector->init(CLIFF_LEFT_DETECTOR_PIN);
	cliffRightDetector->init(CLIFF_RIGHT_DETECTOR_PIN);
	#endif
	#endif
}

bool cliffDetected() {
	bool leftCliff = cliffLeftDetector ? cliffLeftDetector->isCliffDetected() : false;
	bool rightCliff = cliffRightDetector ? cliffRightDetector->isCliffDetected() : false;
	return leftCliff || rightCliff;
}