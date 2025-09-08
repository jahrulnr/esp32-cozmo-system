#include "../register.h"

bool interruptNotePlayer = false;
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
        notePlayer->stop();
        interruptNotePlayer = true;

    }
    else if (event == Note::DOREMI_SCALE) {
        notePlayer->playMelody(Note::DOREMI_SCALE);   
        interruptNotePlayer = false;

    }
    else if (event == Note::HAPPY_BIRTHDAY) {
        notePlayer->playMelody(Note::HAPPY_BIRTHDAY);   
        interruptNotePlayer = false;

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
                
                interruptNotePlayer = false; // Reset interrupt flag
                logger->info("Starting random melody loop, interrupt = false");
                
                // Generate and play continuous random melodies until interrupted
                while (!interruptNotePlayer) {
                    if (notePlayer->generateRandomMelody(melodyLength, melodyBuffer1, endingNote, &endingNote)) {
                        // Play the melody - this will be interrupted if stop() is called
                        if (!notePlayer->playCustomMelody(melodyBuffer1, melodyLength, 1)) {
                            logger->info("Melody playback failed - exiting loop");
                            break; // Exit if playback failed
                        }
                        
                        // Check interrupt flag again after playback
                        if (interruptNotePlayer) {
                            logger->info("Interrupt detected after melody playback - exiting loop");
                            break;
                        }
                        
                        // Short pause between melodies
                        vTaskDelay(pdMS_TO_TICKS(300));
                    } else {
                        logger->error("Failed to generate random melody");
                        break;
                    }
                }
            }
            logger->info("Random melody loop ended, interrupt = %s", interruptNotePlayer ? "true" : "false");

            SendTask::stopTask(noteRandomPlayerId);
            noteRandomPlayerId = "";
        }, "RandomMusicTask");
        
    } else {
        logger->warning("Unknown Note event: %s", event);
        interruptNotePlayer = false;
    }
}
