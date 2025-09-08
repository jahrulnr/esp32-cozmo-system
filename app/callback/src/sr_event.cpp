#include "../register.h"

#if MICROPHONE_ENABLED

// Event callback for SR system
void sr_event_callback(void *arg, sr_event_t event, int command_id, int phrase_id) {
    static bool automationStatus = automation->isEnabled();
    static sr_mode_t lastMode = SR_MODE_WAKEWORD;
    static bool resetScreenWhenTimeout = true;
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
                    notification->send(NOTIFICATION_NOTE, (void*)Note::STOP);
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
                    resetScreenWhenTimeout = true;
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
                    SR::sr_set_mode(SR_MODE_WAKEWORD);
                    return;
                    break;
                case 9: // start recording
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    if (audioRecorder && !audioRecorder->isRecordingActive()) {
                        sayText("Starting recording for 10 seconds!");
                        automationStatus = false;
                        resetScreenWhenTimeout = true;
                        delay(500);
                        if (audioRecorder->startRecording()) {
                            logger->info("Recording started via voice command");
                            SR::sr_set_mode(SR_MODE_WAKEWORD);
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
                    resetScreenWhenTimeout = true;
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

                // music
                case 12: // do re mi
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::HAPPY);
                        resetScreenWhenTimeout = true;
                        SR::sr_set_mode(SR_MODE_WAKEWORD);
                        sayText("Doremi play!");
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 13: // happy birthday
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::HAPPY_BIRTHDAY);
                        notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::HAPPY);
                        resetScreenWhenTimeout = true;
                        SR::sr_set_mode(SR_MODE_WAKEWORD);
                        sayText("Happy birthday play!");
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 14: // play music
                    if (notePlayer) {
                        servos->setHead(DEFAULT_HEAD_ANGLE);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::RANDOM);
                        notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::HAPPY);
                        resetScreenWhenTimeout = true;
                        SR::sr_set_mode(SR_MODE_WAKEWORD);
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;

                // Sound type commands
                case 15: // use piano sound
                    if (notePlayer) {
                        notePlayer->setSoundType(Note::PIANO);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Sound type changed to: Piano");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 16: // use guitar sound
                    if (notePlayer) {
                        notePlayer->setSoundType(Note::GUITAR);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Sound type changed to: Guitar");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 17: // use organ sound
                    if (notePlayer) {
                        notePlayer->setSoundType(Note::ORGAN);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Sound type changed to: Organ");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 18: // use flute sound
                    if (notePlayer) {
                        notePlayer->setSoundType(Note::FLUTE);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Sound type changed to: Flute");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 19: // use bell sound
                    if (notePlayer) {
                        notePlayer->setSoundType(Note::BELL);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Sound type changed to: Bell");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 20: // use square wave sound
                    if (notePlayer) {
                        notePlayer->setSoundType(Note::SQUARE_WAVE);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Sound type changed to: Square Wave");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 21: // use sawtooth sound
                    if (notePlayer) {
                        notePlayer->setSoundType(Note::SAWTOOTH);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Sound type changed to: Sawtooth");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 22: // use triangle sound
                    if (notePlayer) {
                        notePlayer->setSoundType(Note::TRIANGLE);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Sound type changed to: Triangle");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;

                // Volume control commands
                case 23: // set lower sound
                    if (notePlayer) {
                        notePlayer->setVolume(20);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Volume set to: 20%% (lower)");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 24: // set middle sound
                    if (notePlayer) {
                        notePlayer->setVolume(50);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Volume set to: 50%% (middle)");
                        resetScreenWhenTimeout = true;
                        return;
                    } else {
                        sayText("Music system not available!");
                    }
                    break;
                case 25: // set full sound
                    if (notePlayer) {
                        notePlayer->setVolume(100);
                        notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                        logger->info("Volume set to: 100%% (full)");
                        resetScreenWhenTimeout = true;
                        return;
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