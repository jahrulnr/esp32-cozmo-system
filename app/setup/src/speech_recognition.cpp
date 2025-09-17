#include "setup/setup.h"
#include "tasks/register.h"

void setupSpeechRecognition() {
		float volume = 1.f;
		esp_err_t ret = SR::sr_setup(
				mic_fill_callback,                                 // data fill callback
				&volume,																							 // data fill callback argument
#if MICROPHONE_I2S
				SR_CHANNELS_STEREO,
#else
				SR_CHANNELS_MONO,                                  // Single channel I2S input
#endif
				SR_MODE_WAKEWORD,                                  // Start in wake word mode
				voice_commands,                                    // Commands array
				sizeof(voice_commands) / sizeof(csr_cmd_t),        // Number of commands
				sr_event_callback,                                 // Event callback
				NULL                                               // Event callback argument
		);

		if (ret == ESP_OK) {
				logger->info("✅ Speech Recognition started successfully!");
				logger->info("📋 Loaded %d voice commands:", sizeof(voice_commands) / sizeof(csr_cmd_t));
				for (int i = 0; i < (sizeof(voice_commands) / sizeof(csr_cmd_t)); i++) {
						logger->info("   [%d] Group %d: '%s' -> '%s'",
												i,
												voice_commands[i].command_id,
												voice_commands[i].str,
												voice_commands[i].phoneme);
				}
		} else {
				logger->error("❌ Failed to start Speech Recognition: %s\n", esp_err_to_name(ret));
		}
}