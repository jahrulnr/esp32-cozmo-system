#include "setup/setup.h"

Sensors::CliffDetector *cliffLeftDetector = nullptr;
Sensors::CliffDetector *cliffRightDetector = nullptr;

void setupCliffDetector(){
	cliffLeftDetector = new Sensors::CliffDetector();
	cliffRightDetector = new Sensors::CliffDetector();

	#if CLIFF_DETECTOR_ENABLED
	#if CLIFF_IO_EXTENDER
	cliffRightDetector->initWithExtender(&iExpander, (int)CLIFF_RIGHT_DETECTOR_PIN);
	delay(10);
	cliffLeftDetector->initWithExtender(&iExpander, (int)CLIFF_LEFT_DETECTOR_PIN);
	#else
	cliffLeftDetector->init(CLIFF_LEFT_DETECTOR_PIN);
	cliffRightDetector->init(CLIFF_RIGHT_DETECTOR_PIN);
	#endif
	#endif
}