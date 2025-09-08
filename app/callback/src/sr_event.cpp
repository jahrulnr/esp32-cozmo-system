#include "../register.h"

#if MICROPHONE_ENABLED

// Event callback for SR system
void sr_event_callback(void *arg, sr_event_t event, int command_id, int phrase_id) {
    static bool automationStatus = automation->isEnabled();
    static sr_mode_t lastMode = SR_MODE_WAKEWORD;
    bool resetScreenWhenTimeout = true;
    switch (event) {
        case SR_EVENT_WAKEWORD:
            resetScreenWhenTimeout = true;
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
            
        case SR_EVENT_TIMEOUT:
            sayText("Call me again later!");
            logger->info("⏰ Command timeout - returning to wake word mode");
            if (resetScreenWhenTimeout)
                notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::HAPPY);

            lastMode = SR_MODE_WAKEWORD;
            SR::sr_set_mode(SR_MODE_WAKEWORD);
            if (automationStatus)
                notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::RESUME);
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
                    resetScreenWhenTimeout = true;
                    break;
                    
                case 1: // look to right
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::LOOK_RIGHT);
                    motors->move(motors->RIGHT);
                    delay(500);
                    motors->stop();
                    resetScreenWhenTimeout = true;
                    break;
                    
                case 2: // close your eyes
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    delay(100);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::CLOSE_EYE);
                    servos->setHead(180);
                    resetScreenWhenTimeout = true;
                    break;
                case 3: // you can play
                    sayText("Thankyou!");
                    automationStatus = true;
                    notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::RESUME);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::NOTHING);
                    resetScreenWhenTimeout = true;
                    break;
                case 4: // silent
                    sayText("Ok!");
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    automationStatus = false;
                    notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::PAUSE);
                    resetScreenWhenTimeout = false;
                    break;
                case 5: // show weather status
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::WEATHER_STATUS);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    sayText("Here is weather status!");
                    delay(100);
                    servos->setHead(180);
                    automationStatus = false;
                    resetScreenWhenTimeout = false;
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
                    resetScreenWhenTimeout = false;
                    break;
                case 8: // play space game
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::SPACE_GAME);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    sayText("Starting space game!");
                    delay(100);
                    servos->setHead(180);
                    automationStatus = false;
                    resetScreenWhenTimeout = false;
                    break;
                case 9: // start recording
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    if (audioRecorder && !audioRecorder->isRecordingActive()) {
                        sayText("Starting recording for 10 seconds!");
                        delay(500);
                        if (audioRecorder->startRecording()) {
                            logger->info("Recording started via voice command");
                        } else {
                            sayText("Recording failed to start!");
                        }
                    } else {
                        sayText("Recording already in progress!");
                    }
                    break;
                case 10: // show status
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::BASIC_STATUS);
                    servos->setHead(180);
                    sayText("Here my status!");
                    break;
                case 11: // battery status  
                    if (batteryManager) {
                        batteryManager->update();
                        float voltage = batteryManager->getVoltage();
                        int level = batteryManager->getLevel();
                        
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        
                        // Send notification to display task to show battery status
                        notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::BATTERY_STATUS);
                        
                        // Speak battery status
                        char batteryMessage[128];
                        snprintf(batteryMessage, sizeof(batteryMessage), 
                            "Battery level is %d percent, voltage %.1f volts", level, voltage);
                        sayText(batteryMessage);
                        
                        delay(100);
                        servos->setHead(180);
                        automationStatus = false;
                        resetScreenWhenTimeout = false;
                    } else {
                        sayText("Battery monitoring not available!");
                    }
                    break;
                case 12: // play music
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Playing music!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::PLAY_DOREMI);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 13: // do re mi
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Do Re Mi!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::PLAY_DOREMI);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 14: // happy birthday
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Happy Birthday!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::PLAY_HAPPY_BIRTHDAY);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 15: // twinkle star
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Twinkle little star!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::PLAY_TWINKLE_STAR);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 16: // play note
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Playing note!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::PLAY_NOTE);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 17: // stop music
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Stopping music!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::STOP_MUSIC);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 18: // space music
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Playing space theme!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::PLAY_SPACE_THEME);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 19: // star wars
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Imperial March!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::PLAY_STAR_WARS);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 20: // alien sound
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        sayText("Alien contact!");
                        delay(100);
                        notification->send(NOTIFICATION_NOTE, (void*)EVENT_NOTE::PLAY_ALIEN_CONTACT);
                        resetScreenWhenTimeout = true;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                default: 
                    logger->info("Unknown command ID: %d", command_id);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    sayText("Sorry, I not understand!");
                    automationStatus = true;
                    resetScreenWhenTimeout = true;
                    break;
            }
            SR::sr_set_mode(SR_MODE_COMMAND);
            lastMode = SR_MODE_COMMAND;
            break;
            
        default:
            logger->info("❓ Unknown SR event: %d\n", event);
            SR::sr_set_mode(lastMode);
            break;
    }
}

#endif