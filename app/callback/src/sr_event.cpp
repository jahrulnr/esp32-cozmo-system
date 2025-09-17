#include "../register.h"

#if MICROPHONE_ENABLED

// Event callback for SR system
void sr_event_callback(void *arg, sr_event_t event, int command_id, int phrase_id) {
    static sr_mode_t lastMode = SR_MODE_WAKEWORD;
    static bool resetScreenWhenTimeout = true;

    float targetYaw = 0;
    switch (event) {
        case SR_EVENT_WAKEWORD:
            sayText("whats up?");
            resetScreenWhenTimeout = true;
            notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::PAUSE);
            notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::WAKEWORD);
            notification->send(NOTIFICATION_NOTE, (void*)Note::STOP);
            motors->stop();
            servos->setHand(0);
            servos->setHead(180);

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
                notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::FACE);

            lastMode = SR_MODE_WAKEWORD;
            SR::sr_set_mode(SR_MODE_WAKEWORD);
            break;

        case SR_EVENT_COMMAND:
            logger->info("Command detected! ID=%d, Phrase=%d\n", command_id, phrase_id);

            // Handle specific command groups based on command_id (from voice_commands array)
            switch (command_id) {
                case Commands::AUTOMATION_ACTIVE:
                    sayText("Thankyou!");
                    notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::RESUME);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::NOTHING);
                    resetScreenWhenTimeout = true;
                    break;
                case Commands::AUTOMATION_PAUSED:
                    sayText("Ok!");
                    servos->setHead(0);
                    notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::PAUSE);
                    resetScreenWhenTimeout = true;
                    break;
                case Commands::WEATHER:
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::WEATHER_STATUS);
                    sayText("Here is weather status!");
                    servos->setHead(180);
                    resetScreenWhenTimeout = false;
                    break;
                case Commands::REBOOT:
                    sayText("restart!");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    ESP.restart();
                    break;
                case Commands::ORIENTATION:
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::ORIENTATION_DISPLAY);
                    sayText("Here is orientation display!");
                    servos->setHead(180);
                    resetScreenWhenTimeout = false;
                    break;
                case Commands::GAME_SPACE:
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::SPACE_GAME);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    sayText("Starting space game!");
                    delay(100);
                    servos->setHead(180);
                    resetScreenWhenTimeout = false;
                    SR::sr_set_mode(SR_MODE_WAKEWORD);
                    return;
                    break;
                case Commands::RECORD_START:
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    if (!audioRecorder->isRecordingActive()) {
                        if (audioRecorder->startRecording()) {
                            resetScreenWhenTimeout = false;
                            notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::WAKEWORD);
                            logger->info("Recording started via voice command");
                            SR::sr_set_mode(SR_MODE_WAKEWORD);
                            return;
                        } else {
                            sayText("Recording failed to start!");
                        }
                    } else {
                        sayText("Recording already in progress!");
                    }
                    break;
                case Commands::SYSTEM_STATUS:
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::BASIC_STATUS);
                    servos->setHead(180);
                    sayText("Here my status!");
                    resetScreenWhenTimeout = true;
                    break;
                case Commands::NOTE_HAPPY_BIRTHDAY:
                    servos->setHead(180);
                    notification->send(NOTIFICATION_NOTE, (void*)Note::HAPPY_BIRTHDAY);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::FACE);
                    resetScreenWhenTimeout = true;
                    SR::sr_set_mode(SR_MODE_WAKEWORD);
                    return;
                    break;
                case Commands::NOTE_RANDOM:
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    notification->send(NOTIFICATION_NOTE, (void*)Note::RANDOM);
                    resetScreenWhenTimeout = true;
                    break;
                case Commands::SPEAKER_LOWER:
                    notePlayer->setVolume(30);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::FACE);

                    resetScreenWhenTimeout = true;
                    SR::sr_set_mode(SR_MODE_WAKEWORD);
                    return;
                    break;
                case Commands::SPEAKER_MIDDLE:
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    notePlayer->setVolume(55);
                    notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::FACE);

                    resetScreenWhenTimeout = true;
                    SR::sr_set_mode(SR_MODE_WAKEWORD);
                    return;
                    break;
                case Commands::SPEAKER_LOUD:
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    notePlayer->setVolume(80);
                    notification->send(NOTIFICATION_NOTE, (void*)Note::DOREMI_SCALE);
                    notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY::FACE);

                    resetScreenWhenTimeout = true;
                    SR::sr_set_mode(SR_MODE_WAKEWORD);
                    return;
                    break;

                default:
                    logger->info("Unknown command ID: %d", command_id);
                    servos->setHead(DEFAULT_HEAD_ANGLE);
                    sayText("Sorry, I not understand!");
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