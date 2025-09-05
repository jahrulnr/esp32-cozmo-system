#ifndef BOOT_CONSTANTS_H
#define BOOT_CONSTANTS_H

#include "csr.h"

// Define voice commands (phonetic representations)
static const csr_cmd_t voice_commands[] = {
	{0, "look to left", "LwK To LfFT"},
	{1, "look to right", "LwK To RiT"},
	{2, "close your eyes", "KLbS YeR iZ"},
	{3, "you can play", "Yo KaN PLd"},
	{4, "silent", "SiLcNT"}
};

static const char* NOTIFICATION_SPEAKER = "speaker";
static const char* NOTIFICATION_COMMAND = "command";

// Automation Events
static const char* NOTIFICATION_AUTOMATION = "automation";
namespace EVENT_AUTOMATION {
	static const char* PAUSE = "pause";
	static const char* RESUME = "resume";
}

// Display Events
static const char* NOTIFICATION_DISPLAY = "display";
namespace EVENT_DISPLAY {
	static const char* WAKEWORD = "wakeword";
	static const char* LOOK_LEFT = "look_left";
	static const char* LOOK_RIGHT = "look_right";
	static const char* CLOSE_EYE = "close_eye";
	static const char* CLIFF_DETECTED = "cliff_detected";
	static const char* OBSTACLE_DETECTED = "obstacle_detected";
	static const char* STUCK_DETECTED = "obstacle_detected";
}

// SR Events
static const char* NOTIFICATION_WAKEWORD = "wakeword";
static const char* NOTIFICATION_SR = "sr";
namespace EVENT_SR {
	static const char* WAKEWORD = "sr_wakeword";
	static const char* COMMAND = "sr_command";
	static const char* TIMEOUT = "sr_timeout";
	static const char* PAUSE = "pause_sr";
	static const char* RESUME = "resume_sr";
}

#endif