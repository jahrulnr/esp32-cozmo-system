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
        // Octave 3 (low octave)
        C3 = 131, D3 = 147, E3 = 165, F3 = 175, G3 = 196, A3 = 220, B3 = 247,
        // Octave 4 (middle octave)
        C4 = 262, D4 = 294, E4 = 330, F4 = 349, G4 = 392, A4 = 440, B4 = 494,
        // Octave 5 (high octave)
        C5 = 523, D5 = 587, E5 = 659, F5 = 698, G5 = 784, A5 = 880, B5 = 988,
        // Special notes
        REST = 0    // Silence
    };

    // Note duration types
    enum Duration {
        WHOLE = 1000,       // 1 second
        HALF = 500,         // 0.5 second
        QUARTER = 250,      // 0.25 second
        EIGHTH = 125,       // 0.125 second
        SIXTEENTH = 62      // 0.0625 second
    };

    // Instrument sound types (simplified)
    enum SoundType {
        PIANO = 0,          // Clean sine wave (default)
        GUITAR,             // Plucked string
        ORGAN,              // Rich harmonics
        FLUTE,              // Pure tone
        BELL,               // Metallic tone
        SQUARE_WAVE,        // Classic 8-bit sound
        SAWTOOTH,           // Bright, buzzy sound
        TRIANGLE            // Soft, mellow tone
    };

    // Chord types (simplified)
    enum ChordType {
        MAJOR = 0,          // Major chord (1-3-5)
        MINOR,              // Minor chord (1-b3-5)
        DIMINISHED,         // Diminished chord
        SEVENTH,            // Dominant 7th chord
        MAJOR7,             // Major 7th chord
        MINOR7,             // Minor 7th chord
        SUSPENDED2,         // Sus2 chord
        SUSPENDED4          // Sus4 chord
    };

    // Predefined melodies
    enum Melody {
        DOREMI_SCALE = 1,
        HAPPY_BIRTHDAY,
        RANDOM,
        STOP
    };

    // Musical note structure
    struct MusicNote {
        Frequency frequency;
        Duration duration;
    };

private:
    I2SSpeaker* _speaker;
    Utils::Logger* _logger;
    uint16_t _amplitude;        // Volume control
    SoundType _soundType;       // Current instrument sound

    // Audio generation parameters
    static const uint32_t SAMPLE_RATE = 16000;  // 16kHz sample rate
    static const uint16_t DEFAULT_AMPLITUDE = 15000;  // Default volume level

    // Simplified wave generation
    void generateWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    void applyFade(int16_t* buffer, size_t sampleCount, size_t fadeInSamples, size_t fadeOutSamples);

    // Melody definitions
    const MusicNote* getMelodyNotes(Melody melody, size_t* noteCount);

public:
    // Constructor with dependency injection
    Note(I2SSpeaker* speaker, Utils::Logger* logger);
    ~Note();

    // Core audio playback
    bool playFrequency(uint16_t frequency, uint32_t durationMs);

    // Melody playback (simplified - no repeat handling for event-driven usage)
    bool playMelody(Melody melody, int repeatCount = 1);
    bool playCustomMelody(const MusicNote* notes, size_t noteCount, int repeatCount = 1);

    // Chord playback (simplified)
    bool playChord(const Frequency* frequencies, size_t noteCount, uint32_t durationMs);
    bool playChord(Frequency rootNote, ChordType type, uint32_t durationMs);

    // Simple sound effects
    bool playScale(Frequency startNote = C4, bool ascending = true);

    // Random melody generation (simplified)
    bool generateRandomMelody(size_t noteCount, MusicNote* outputBuffer);
    bool generateRandomMelody(size_t noteCount, MusicNote* outputBuffer, Frequency startNote, Frequency* endNote = nullptr);
    bool playRandomMelody(size_t noteCount, int repeatCount = 1);

    // Volume control
    void setVolume(uint8_t volumePercent);      // 0-100 percent
    void setVolumeRaw(uint16_t amplitude);      // Raw amplitude 0-32767
    uint8_t getVolume() const;                  // Returns 0-100 percent
    uint16_t getVolumeRaw() const;              // Returns raw amplitude

    // Sound type control
    void setSoundType(SoundType soundType);     // Set instrument sound
    SoundType getSoundType() const;             // Get current sound type
    const char* getSoundTypeName() const;       // Get sound type name as string

    // System status
    bool isReady() const;
    void stop();
    void interrupt();
};

#endif
