#pragma once

namespace Perception {

class Status {

typedef enum {
	OK = 0,
	LEFT_CLIFF_DETECTED,
	RIGHT_CLIFF_DETECTED,
	FRONT_CLIFF_DETECTED,
	STUCT_DETECTED,
	MAX
} Condition;

};

}; // end namespace