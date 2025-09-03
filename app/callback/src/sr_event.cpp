#include "../register.h"

#if MICROPHONE_ENABLED

// Event callback for SR system
void sr_event_callback(void *arg, sr_event_t event, int command_id, int phrase_id) {
    switch (event) {
        case SR_EVENT_WAKEWORD:
            logger->info("🎙️ Wake word 'Hi ESP' detected!");
            notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION_PAUSE);

            sayText("Hi, whats up?");
            delay(5000);
            SR::sr_set_mode(SR_MODE_COMMAND);
            logger->info("📞 Listening for commands...");
            break;
            
        case SR_EVENT_WAKEWORD_CHANNEL:
            logger->info("🎙️ Wake word detected on channel: %d\n", command_id);
            SR::sr_set_mode(SR_MODE_COMMAND);
            break;
            
        case SR_EVENT_COMMAND:
            logger->info("✅ Command detected! ID=%d, Phrase=%d\n", command_id, phrase_id);
            
            // Handle specific command groups based on command_id (from voice_commands array)
            switch (command_id) {
                case 0: // look to left
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_LOOK_LEFT);
                    motors->move(motors->LEFT);
                    delay(500);
                    motors->stop();
                    break;
                    
                case 1: // look to right
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_LOOK_RIGHT);
                    motors->move(motors->RIGHT);
                    delay(500);
                    motors->stop();
                    break;
                    
                case 2: // close your eyes
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    delay(100);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_CLOSE_EYE);
                    servos->setHead(180);
                    break;
                case 3: // you can play
                    sayText("Thankyou!");
                    notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION_RESUME);
                    break;
                case 4: // silent
                    sayText("Ok!");
                    notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION_PAUSE);
                    break;
                
                default: 
                    logger->info("❓ Unknown command ID: %d\n", command_id);
                    sayText("Sorry, I not understand!");
                    break;
            }
            // SR::sr_set_mode(SR_MODE_COMMAND);
            break;
            
        case SR_EVENT_TIMEOUT:
            logger->info("⏰ Command timeout - returning to wake word mode");
            delay(5000);
            notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION_RESUME);
            SR::sr_set_mode(SR_MODE_WAKEWORD);
            break;
            
        default:
            logger->info("❓ Unknown SR event: %d\n", event);
            break;
    }
}

#endif