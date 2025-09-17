#include "../register.h"

String noteRandomPlayerId;

void callbackNotePlayer(void* data) {
    if (!data) {
        logger->error("Note callback: No data received");
        return;
    }

    if (!notePlayer) {
        logger->error("Note callback: notePlayer is null");
        return;
    }

    const Note::Melody event = (Note::Melody)(intptr_t)data;
    logger->info("Note callback received event: %d", (int)event);

    if (event == Note::STOP) {
        logger->info("STOP command received - setting interrupt and calling notePlayer->stop()");
        SendTask::stopTask(noteRandomPlayerId);
        noteRandomPlayerId = "";

    }
    else if (event == Note::DOREMI_SCALE) {
        notePlayer->playMelody(Note::DOREMI_SCALE);

    }
    else if (event == Note::HAPPY_BIRTHDAY) {
        notePlayer->playMelody(Note::HAPPY_BIRTHDAY);

    }
    else if (event == Note::RANDOM) {
        if(!noteRandomPlayerId.isEmpty()) {
            logger->warning("RANDOM command already played");
            return;
        }

        logger->info("RANDOM command received - starting random melody loop");
        noteRandomPlayerId = SendTask::createTaskOnCore([](){
            const size_t melodyLength = 64;
            Note::MusicNote melodyBuffer1[melodyLength];

            // Check if notePlayer is initialized before using it
            if (notePlayer && notePlayer->isReady()) {
                Note::Frequency endingNote = (Note::Frequency)0; // Start with auto-chosen note

                logger->info("Starting random melody loop, interrupt = false");

                // Generate and play continuous random melodies until interrupted
                while (true) {
                    if (notePlayer->generateRandomMelody(melodyLength, melodyBuffer1, endingNote, &endingNote)) {
                        // Play the melody - this will be interrupted if stop() is called
                        if (!notePlayer->playCustomMelody(melodyBuffer1, melodyLength, 1)) {
                            logger->info("Melody playback failed - exiting loop");
                            break; // Exit if playback failed
                        }

                        // Short pause between melodies
                        vTaskDelay(pdMS_TO_TICKS(300));
                    } else {
                        logger->error("Failed to generate random melody");
                        break;
                    }
                }
            }
            logger->info("Random melody loop ended");

            String id = noteRandomPlayerId;
            noteRandomPlayerId = "";
            SendTask::stopTask(id);
        }, "RandomMusicTask");

    } else {
        logger->warning("Unknown Note event: %s", event);
    }
}
