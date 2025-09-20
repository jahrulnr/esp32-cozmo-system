#include "../register.h"

void notePlayerHandler() {
    bool event = notification->hasSignal(NOTIFICATION_NOTE);
    static int count = 0;
    static Note::Melody lastEvent = Note::Melody::STOP;
    int newEvent = event ? notification->signal(NOTIFICATION_NOTE) : lastEvent;
    if (newEvent < 0 || newEvent >= Note::Melody::STOP) {
        lastEvent = Note::Melody::STOP;
        if (newEvent != Note::Melody::STOP)
            ESP_LOGI("notePlayerHandler", "music id invalid: %d", newEvent);
        count = 0;
    } else if (newEvent != Note::Melody::STOP && count == 0) {
        SR::sr_pause();
        ESP_LOGI("notePlayerHandler", "music %d on air", newEvent);
        lastEvent = (Note::Melody)newEvent;
        vTaskDelay(1000);
    }

    if (lastEvent == Note::STOP) {
        lastEvent = Note::Melody::STOP;
        SR::sr_resume();
    }
    else if (lastEvent == Note::DOREMI_SCALE) {
        notePlayer->playMelody(Note::DOREMI_SCALE);
        count++;
        lastEvent = Note::Melody::STOP;
    }
    else if (lastEvent == Note::HAPPY_BIRTHDAY) {
        notePlayer->playMelody(Note::HAPPY_BIRTHDAY);
        count++;
        lastEvent = Note::Melody::STOP;
    }
    else if (lastEvent == Note::RANDOM) {
        notePlayer->playMelody(Note::RANDOM);          // Now plays chord combinations, not single melody
        count++;
        lastEvent = Note::Melody::STOP;
    }
    // else if (lastEvent == Note::RANDOM) {
    //     const size_t melodyLength = 128;
    //     Note::MusicNote melodyBuffer[melodyLength];
    //     // Check if notePlayer is initialized before using it
    //     if (notePlayer && notePlayer->isReady()) {
    //         Note::Frequency endingNote = (Note::Frequency)0; // Start with auto-chosen note
    //         if (notePlayer->generateRandomMelody(melodyLength, melodyBuffer, endingNote, &endingNote)) {
    //             // Play the melody - this will be interrupted if stop() is called
    //             if (!notePlayer->playCustomMelody(melodyBuffer, melodyLength, 1)) {
    //                 lastEvent = Note::Melody::STOP;
    //                 return; // Exit if playback failed
    //             }

    //             count++;
    //         } else {
    //             logger->error("Failed to generate random melody");
    //             lastEvent = Note::Melody::STOP;
    //             return;
    //         }

    //         if (count > 100) {
    //             lastEvent = Note::Melody::STOP;
    //         }
    //     }
    // }
}
