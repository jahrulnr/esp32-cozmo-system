#include "../setup.h"

Note* notePlayer = nullptr;

void setupNotePlayer() {
    if (!SPEAKER_ENABLED) {
        logger->info("Note: Speaker disabled");
        return;
    }

    if (!i2sSpeaker) {
        logger->error("Note: I2S Speaker not initialized");
        return;
    }

    logger->info("Note: Initializing musical note system");
    logger->info("Note: Speaker channel mode: %d", i2sSpeaker->getChannelMode());
    logger->info("Note: Speaker active: %s", i2sSpeaker->isActive() ? "true" : "false");
    
    notePlayer = new Note(i2sSpeaker, logger);
    
    if (notePlayer) {
        notePlayer->setVolume(SPEAKER_VOLUME * 0.3 * 100);
        notePlayer->setSoundType(Note::GUITAR);
        logger->info("Note: Musical system ready");
    } else {
        logger->error("Note: Failed to initialize musical system");
    }
}
