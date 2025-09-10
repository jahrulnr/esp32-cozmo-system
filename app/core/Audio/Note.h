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
        // Octave 3 (low octave - was middle)
        C3 = 131,   // Do (low)
        D3 = 147,   // Re (low)
        E3 = 165,   // Mi (low)
        F3 = 175,   // Fa (low)
        G3 = 196,   // Sol (low)
        A3 = 220,   // La (low)
        B3 = 247,   // Si (low)
        
        // Octave 4 (middle octave - was high)
        C4 = 262,   // Do
        D4 = 294,   // Re
        E4 = 330,   // Mi
        F4 = 349,   // Fa
        G4 = 392,   // Sol
        A4 = 440,   // La
        B4 = 494,   // Si
        
        // Octave 5 (high octave - for occasional high notes)
        C5 = 523,   // Do (high)
        D5 = 587,   // Re (high)
        E5 = 659,   // Mi (high)
        F5 = 698,   // Fa (high)
        G5 = 784,   // Sol (high)
        A5 = 880,   // La (high)
        B5 = 988,   // Si (high)
        
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
    
    // Instrument sound types
    enum SoundType {
        PIANO = 0,          // Clean sine wave (default)
        GUITAR,             // Plucked string with decay
        ORGAN,              // Rich harmonics
        FLUTE,              // Pure tone with vibrato
        BELL,               // Metallic with long decay
        SQUARE_WAVE,        // Classic 8-bit sound
        SAWTOOTH,           // Bright, buzzy sound
        TRIANGLE            // Soft, mellow tone
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
    volatile bool _interrupt; // Make volatile for thread safety
    uint16_t _amplitude; // Dynamic amplitude control
    SoundType _soundType; // Current instrument sound
    
    // Audio generation parameters
    static const uint32_t SAMPLE_RATE = 16000;  // 16kHz sample rate
    static const uint16_t DEFAULT_AMPLITUDE = 15000;  // Default volume level (about 45% of max 32767)
    static const uint8_t CHANNELS = 1;          // Mono output
    
    // Wave generation
    void generateSineWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    void generateSquareWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t bufferSize);
    void generateSilence(uint32_t durationMs, int16_t* buffer, size_t bufferSize);
    void applyFade(int16_t* buffer, size_t sampleCount, size_t fadeInSamples, size_t fadeOutSamples);
    
    // Instrument-specific wave generation
    void generateInstrumentWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount, SoundType soundType);
    void generateGuitarWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    void generateOrganWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    void generateFluteWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    void generateBellWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    void generateSawtoothWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    void generateTriangleWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount);
    
    // Internal playback methods
    bool playFrequencyInternal(uint16_t frequency, uint32_t durationMs, bool checkPlaying = true);
    
    // Melody definitions
    const MusicNote* getMelodyNotes(Melody melody, size_t* noteCount);
    
public:
    // Constructor with dependency injection
    Note(I2SSpeaker* speaker, Utils::Logger* logger);
    ~Note();
    
    // Single note playback
    bool playFrequency(uint16_t frequency, uint32_t durationMs);
    
    // Melody playback
    bool playMelody(Melody melody, int repeatCount = 1); // 0 = once, -1 = forever
    bool playCustomMelody(const MusicNote* notes, size_t noteCount, int repeatCount = 1);
    
    // Simple sound effects
    bool playScale(Frequency startNote = C4, bool ascending = true);
    
    // Random melody generation
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
