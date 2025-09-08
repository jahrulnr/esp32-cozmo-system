#ifndef NOTE_H
#define NOTE_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Config.h"
#include "Constants.h"
#include "Logger.h"
#include "SendTask.h"
#include "I2SSpeaker.h"
#include <cmath>

class Note {
public:
    // Musical notes frequencies (in Hz)
    enum Frequency {
        // Octave 4 (middle octave)
        C4 = 262,   // Do
        D4 = 294,   // Re
        E4 = 330,   // Mi
        F4 = 349,   // Fa
        G4 = 392,   // Sol
        A4 = 440,   // La
        B4 = 494,   // Si
        
        // Octave 5 (higher octave)
        C5 = 523,   // Do (high)
        D5 = 587,   // Re (high)
        E5 = 659,   // Mi (high)
        F5 = 698,   // Fa (high)
        G5 = 784,   // Sol (high)
        A5 = 880,   // La (high)
        B5 = 988,   // Si (high)
        
        // Special notes
        REST = 0,   // Silence
        C3 = 131,   // Low Do
        D3 = 147,   // Low Re
        E3 = 165,   // Low Mi
        F3 = 175,   // Low Fa
        G3 = 196,   // Low Sol
        A3 = 220,   // Low La
        B3 = 247    // Low Si
    };
    
    // Note duration types
    enum Duration {
        WHOLE = 1000,       // 1 second
        HALF = 500,         // 0.5 second
        QUARTER = 250,      // 0.25 second
        EIGHTH = 125,       // 0.125 second
        SIXTEENTH = 62      // 0.0625 second
    };
    
    // Predefined melodies
    enum Melody {
        DOREMI_SCALE,       // Do-Re-Mi-Fa-Sol-La-Si-Do
        HAPPY_BIRTHDAY,     // Happy Birthday melody
        TWINKLE_STAR,       // Twinkle Twinkle Little Star
        JINGLE_BELLS,       // Jingle Bells
        STARTUP_SOUND,      // Robot startup sound
        SUCCESS_SOUND,      // Success notification
        ERROR_SOUND,        // Error notification
        MARIO_COIN,         // Mario coin sound
        BEEP_SEQUENCE,      // Simple beep sequence
        SPACE_THEME,        // Space exploration theme
        STAR_WARS_THEME,    // Imperial March style
        UFO_LANDING,        // Sci-fi UFO landing
        COSMIC_JOURNEY,     // Deep space travel theme
        ALIEN_CONTACT,      // First contact melody
        WARP_SPEED          // Hyperspace jump sound
    };
    
    // Musical note structure
    struct MusicNote {
        Frequency frequency;
        Duration duration;
    };

private:
    I2SSpeaker* _speaker;
    Utils::Logger* _logger;
    
    // Audio generation parameters
    static const uint32_t SAMPLE_RATE = 16000;  // 16kHz sample rate
    static const uint16_t AMPLITUDE = 30000;    // Volume level (about 90% of max 32767)
    static const uint8_t CHANNELS = 1;          // Mono output
    
    // Wave generation
    void generateSineWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    void generateSquareWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t bufferSize);
    void generateSilence(uint32_t durationMs, int16_t* buffer, size_t bufferSize);
    void applyFade(int16_t* buffer, size_t sampleCount, size_t fadeInSamples, size_t fadeOutSamples);
    
    // Internal playback methods
    bool playFrequencyInternal(uint16_t frequency, uint32_t durationMs, bool checkPlaying = true);
    
    // Melody definitions
    const MusicNote* getMelodyNotes(Melody melody, size_t* noteCount);
    void playNoteTask(Frequency note, Duration duration);
    
public:
    // Constructor with dependency injection
    Note(I2SSpeaker* speaker, Utils::Logger* logger);
    ~Note();
    
    // Single note playback
    bool playNote(Frequency note, Duration duration = QUARTER);
    bool playFrequency(uint16_t frequency, uint32_t durationMs);
    
    // Melody playback
    bool playMelody(Melody melody);
    bool playCustomMelody(const MusicNote* notes, size_t noteCount);
    
    // Simple sound effects
    bool playBeep(uint16_t frequency = A4, uint32_t durationMs = 100);
    bool playChord(Frequency note1, Frequency note2, Duration duration = QUARTER);
    bool playScale(Frequency startNote = C4, bool ascending = true);
    
    // Volume and tone control
    void setVolume(uint8_t volume); // 0-100
    void setWaveform(bool useSquareWave = false); // false = sine, true = square
    
    // System status
    bool isReady() const;
    
    // Musical helper functions
    static Frequency getNoteFromString(const char* noteStr); // "C4", "D#4", etc.
    static const char* getNoteString(Frequency note);
    static uint16_t getFrequency(Frequency note) { return (uint16_t)note; }
};

#endif
