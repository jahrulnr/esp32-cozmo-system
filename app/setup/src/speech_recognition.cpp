#include "setup/setup.h"
#include "tasks/register.h"

bool sr_system_running = false;

void setupSpeechRecognition() {
		esp_err_t ret = SR::sr_start(
				sr_fill_callback,                                  // data fill callback
				NULL,																							 // data fill callback argument
				SR_CHANNELS_MONO,                                  // Single channel I2S input
				SR_MODE_WAKEWORD,                                  // Start in wake word mode
				voice_commands,                                    // Commands array
				sizeof(voice_commands) / sizeof(csr_cmd_t),         // Number of commands
				sr_event_callback,                                 // Event callback
				NULL                                               // Event callback argument
		);
		
		if (ret == ESP_OK) {
				sr_system_running = true;
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
				sr_system_running = false;
		}
}