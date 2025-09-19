#ifndef BOOT_CONSTANTS_H
#define BOOT_CONSTANTS_H

#include "csr.h"

static const char* deviceName = "pio-esp32-cam";

typedef enum {
	AUTOMATION_ACTIVE = 0,
	AUTOMATION_PAUSED,
	WEATHER,
	REBOOT,
	ORIENTATION,
	GAME_SPACE,
	RECORD_START,
	RECORD_STOP,
	SYSTEM_STATUS,
	NOTE_TEST,
	NOTE_HAPPY_BIRTHDAY,
	NOTE_RANDOM,
	SPEAKER_LOWER,
	SPEAKER_MIDDLE,
	SPEAKER_LOUD,
} Commands;

// for generate phonetic, run `python3 tools/multinet_g2p.py --text="new command"`
// Define voice commands (phonetic representations)
static const csr_cmd_t voice_commands[] = {
	{Commands::AUTOMATION_ACTIVE, "you can play", "Yo KaN PLd"},
	{Commands::AUTOMATION_PAUSED, "silent", "SiLcNT"},
	{Commands::WEATHER, "show weather", "sb Wfjk"},
	{Commands::REBOOT, "reboot", "RgBoT"},
	{Commands::REBOOT, "restart", "RmSTnRT"},
	{Commands::ORIENTATION, "show orientation", "sb eRmfNTdscN"},
	{Commands::GAME_SPACE, "space game", "PLd c GdM"},
	{Commands::RECORD_START, "record audio", "RfKkD nDmb"},
	{Commands::SYSTEM_STATUS, "show status", "sb STaTcS"},
	{Commands::SYSTEM_STATUS, "Tumbil cun status", "TcMBcL KcN STaTcS"},
	{Commands::NOTE_TEST, "do re mi", "Do Rd Mm"},
	{Commands::NOTE_HAPPY_BIRTHDAY, "happy birthday", "haPm BkvDd"},
	{Commands::NOTE_RANDOM, "play a music", "PLd c MYoZgK"},
	// Volume control commands
	{Commands::SPEAKER_LOWER, "set lower sound", "SfT Lbk StND"},
	{Commands::SPEAKER_MIDDLE, "set middle sound", "SfT MgDcL StND"},
	{Commands::SPEAKER_LOUD, "set full sound", "SfT FwL StND"},
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
	FACE,
	TOUCH_DETECTED,
	BASIC_STATUS,
	WEATHER_STATUS,
	ORIENTATION_DISPLAY,
	SPACE_GAME,
	RECORDING_STARTED,
	RECORDING_STOPPED,
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

// DL_EVENTS
static const char* NOTIFICATION_DL = "dl";
#endif