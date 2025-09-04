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
static const char* EVENT_AUTOMATION_PAUSE = "pause";
static const char* EVENT_AUTOMATION_RESUME = "resume";

// Display Events
static const char* NOTIFICATION_DISPLAY = "display";
static const char* EVENT_DISPLAY_WAKEWORD = "wakeword";
static const char* EVENT_DISPLAY_LOOK_LEFT = "look_left";
static const char* EVENT_DISPLAY_LOOK_RIGHT = "look_right";
static const char* EVENT_DISPLAY_CLOSE_EYE = "close_eye";

// SR Events
static const char* NOTIFICATION_WAKEWORD = "wakeword";
static const char* NOTIFICATION_SPEECH_RECOGNITION = "sr";
static const char* EVENT_SR_WAKEWORD = "sr_wakeword";
static const char* EVENT_SR_COMMAND = "sr_command";
static const char* EVENT_SR_TIMEOUT = "sr_timeout";
static const char* EVENT_SR_PAUSE = "pause_sr";
static const char* EVENT_SR_RESUME = "resume_sr";

#endif