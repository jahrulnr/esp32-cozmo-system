#include "../register.h"

void callbackNotePlayer(void* data) {
    if (!data) {
        logger->error("Note callback: No data received");
        return;
    }
    
    if (!notePlayer) {
        logger->error("Note callback: notePlayer is null");
        return;
    }
    
    logger->info("Note callback triggered");
    
    const char* event = (const char*)data;
    
    if (strcmp(event, EVENT_NOTE::PLAY_DOREMI) == 0) {
        logger->info("Playing DoReMi scale");
        notePlayer->playMelody(Note::DOREMI_SCALE);
        
    } else if (strcmp(event, EVENT_NOTE::PLAY_HAPPY_BIRTHDAY) == 0) {
        logger->info("Playing Happy Birthday");
        notePlayer->playMelody(Note::HAPPY_BIRTHDAY);
        
    } else if (strcmp(event, EVENT_NOTE::PLAY_TWINKLE_STAR) == 0) {
        logger->info("Playing Twinkle Star");
        notePlayer->playMelody(Note::TWINKLE_STAR);
        
    } else if (strcmp(event, EVENT_NOTE::PLAY_NOTE) == 0) {
        logger->info("Playing single note");
        notePlayer->playFrequency(Note::C4, 500); // 500ms duration
        
    } else if (strcmp(event, EVENT_NOTE::STOP_MUSIC) == 0) {
        logger->info("Music stop requested (simplified system - no stop needed)");
        
    } else if (strcmp(event, EVENT_NOTE::PLAY_SPACE_THEME) == 0) {
        logger->info("Playing space theme");
        notePlayer->playMelody(Note::SPACE_THEME);
        
    } else if (strcmp(event, EVENT_NOTE::PLAY_STAR_WARS) == 0) {
        logger->info("Playing Star Wars theme");
        notePlayer->playMelody(Note::STAR_WARS_THEME);
        
    } else if (strcmp(event, EVENT_NOTE::PLAY_ALIEN_CONTACT) == 0) {
        logger->info("Playing alien contact sound");
        notePlayer->playMelody(Note::ALIEN_CONTACT);
        
    } else {
        logger->warning("Unknown Note event: %s", event);
    }
}
