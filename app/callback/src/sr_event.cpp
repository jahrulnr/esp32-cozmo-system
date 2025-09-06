#include "../register.h"

#if MICROPHONE_ENABLED

// Event callback for SR system
void sr_event_callback(void *arg, sr_event_t event, int command_id, int phrase_id) {
    static bool automationStatus = automation->isEnabled();
    static sr_mode_t lastMode = SR_MODE_WAKEWORD;
    switch (event) {
        case SR_EVENT_WAKEWORD:
            notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::PAUSE);
            notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::WAKEWORD);

            sayText("whats up?");
            SR::sr_set_mode(SR_MODE_COMMAND);
            logger->info("Listening for commands...");
            lastMode = SR_MODE_WAKEWORD;
            break;
            
        case SR_EVENT_WAKEWORD_CHANNEL:
            logger->info("Wake word detected on channel: %d\n", command_id);
            SR::sr_set_mode(lastMode);
            break;
            
        case SR_EVENT_COMMAND:
            logger->info("Command detected! ID=%d, Phrase=%d\n", command_id, phrase_id);
            
            // Handle specific command groups based on command_id (from voice_commands array)
            switch (command_id) {
                case 0: // look to left
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::LOOK_LEFT);
                    motors->move(motors->LEFT);
                    delay(500);
                    motors->stop();
                    break;
                    
                case 1: // look to right
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::LOOK_RIGHT);
                    motors->move(motors->RIGHT);
                    delay(500);
                    motors->stop();
                    break;
                    
                case 2: // close your eyes
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    delay(100);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::CLOSE_EYE);
                    servos->setHead(180);
                    break;
                case 3: // you can play
                    sayText("Thankyou!");
                    automationStatus = true;
                    notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::RESUME);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::NOTHING);
                    break;
                case 4: // silent
                    sayText("Ok!");
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    automationStatus = false;
                    notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::PAUSE);
                    break;
                case 5: // show weather status
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::WEATHER_STATUS);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    sayText("Here is weather status!");
                    delay(100);
                    servos->setHead(180);
                    automationStatus = false;
                    break;
                case 6: // restart system
                    ESP.restart();
                    break;
                case 7: // show orientation
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::ORIENTATION_DISPLAY);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    sayText("Here is orientation display!");
                    delay(100);
                    servos->setHead(180);
                    automationStatus = false;
                    break;
                default: 
                    logger->info("Unknown command ID: %d", command_id);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    sayText("Sorry, I not understand!");
                    break;
            }
            SR::sr_set_mode(SR_MODE_COMMAND);
            lastMode = SR_MODE_COMMAND;
            break;
            
        case SR_EVENT_TIMEOUT:
            logger->info("⏰ Command timeout - returning to wake word mode");
            delay(5000);
            sayText("Call me again later!");
            SR::sr_set_mode(SR_MODE_WAKEWORD);
            lastMode = SR_MODE_WAKEWORD;

            if (automationStatus)
                notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::RESUME);
            break;
            
        default:
            logger->info("❓ Unknown SR event: %d\n", event);
            SR::sr_set_mode(lastMode);
            break;
    }
}

#endif