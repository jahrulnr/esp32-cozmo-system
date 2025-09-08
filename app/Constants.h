#ifndef BOOT_CONSTANTS_H
#define BOOT_CONSTANTS_H

#include "csr.h"

static const char* deviceName = "pio-esp32-cam";

// for generate phonetic, run `python3 tools/multinet_g2p.py --text="new command"`
// Define voice commands (phonetic representations)
static const csr_cmd_t voice_commands[] = {
	{0, "look to left", "LwK To LfFT"},
	{1, "look to right", "LwK To RiT"},
	{2, "close your eyes", "KLbS YeR iZ"},
	{3, "you can play", "Yo KaN PLd"},
	{4, "silent", "SiLcNT"},
	{5, "show weather", "sb Wfjk"},
	{5, "show weather status", "sb Wfjk STaTcS"},
	{6, "reboot", "RgBoT"},
	{6, "restart", "RmSTnRT"},
	{7, "show orientation", "sb eRmfNTdscN"},
	{8, "play a game", "PLd c GdM"},
	{9, "record audio", "RfKkD nDmb"},
	{10, "show status", "sb STaTcS"},
	{10, "Tumbil cun status", "TcMBcL KcN STaTcS"},
	{11, "battery status", "BaTkm STaTcS"},
	{11, "show battery", "sb BaTkm"},
	{12, "do re mi", "Do Rd Mm"},
	{13, "happy birthday", "haPm BkvDd"},
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
typedef enum  {
	WAKEWORD,
	HAPPY,
	LOOK_LEFT,
	LOOK_RIGHT,
	CLOSE_EYE,
	CLIFF_DETECTED,
	OBSTACLE_DETECTED,
	STUCK_DETECTED,
	TOUCH_DETECTED,
	BASIC_STATUS,
	WEATHER_STATUS,
	ORIENTATION_DISPLAY,
	SPACE_GAME,
	RECORDING_STARTED,
	RECORDING_STOPPED,
	BATTERY_CRITICAL,
	BATTERY_LOW,
	BATTERY_STATUS,
	NOTHING,
} EVENT_DISPLAY;

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

// PicoTTS Events
static const char* NOTIFICATION_TTS = "tts";
namespace EVENT_TTS {
	static const char* PAUSE = "pause_tts";
	static const char* RESUME = "resume_tts";
}

// Audio Recording Events
static const char* NOTIFICATION_AUDIO = "audio";
namespace EVENT_AUDIO {
	static const char* START_RECORDING = "start_recording";
	static const char* STOP_RECORDING = "stop_recording";
	static const char* RECORDING_COMPLETE = "recording_complete";
}

// Note Music Events
static const char* NOTIFICATION_NOTE = "note";

#endif