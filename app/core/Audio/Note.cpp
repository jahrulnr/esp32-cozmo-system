#include "Note.h"
#include <algorithm>

// ============================================================================
// MELODY DEFINITIONS - Simplified and organized
// ============================================================================

static const Note::MusicNote DOREMI_SCALE_NOTES[] = {
    {Note::C4, Note::QUARTER},  // Do (middle C)
    {Note::D4, Note::QUARTER},  // Re
    {Note::E4, Note::QUARTER},  // Mi
    {Note::F4, Note::QUARTER},  // Fa
    {Note::G4, Note::QUARTER},  // Sol
    {Note::A4, Note::QUARTER},  // La
    {Note::B4, Note::QUARTER},  // Si
    {Note::C5, Note::HALF}      // Do (high C)
};

static const Note::MusicNote HAPPY_BIRTHDAY_NOTES[] = {
    // "Happy Birth-day to you" (first line)
    {Note::G4, Note::EIGHTH}, {Note::G4, Note::EIGHTH}, {Note::A4, Note::QUARTER},
    {Note::G4, Note::QUARTER}, {Note::C5, Note::QUARTER}, {Note::B4, Note::HALF},
    // "Happy Birth-day to you" (second line)
    {Note::G4, Note::EIGHTH}, {Note::G4, Note::EIGHTH}, {Note::A4, Note::QUARTER},
    {Note::G4, Note::QUARTER}, {Note::D5, Note::QUARTER}, {Note::C5, Note::HALF},
    // "Happy Birth-day dear [name]" (third line)
    {Note::G4, Note::EIGHTH}, {Note::G4, Note::EIGHTH}, {Note::G5, Note::QUARTER},
    {Note::E5, Note::QUARTER}, {Note::C5, Note::QUARTER}, {Note::B4, Note::QUARTER}, {Note::A4, Note::HALF},
    // "Happy Birth-day to you" (final line)
    {Note::F5, Note::EIGHTH}, {Note::F5, Note::EIGHTH}, {Note::E5, Note::QUARTER},
    {Note::C5, Note::QUARTER}, {Note::D5, Note::QUARTER}, {Note::C5, Note::HALF}
};

// Random Main Theme - Chord-based version for rich harmonic sound
static const Note::MusicNote RANDOM_NOTES[] = {
    // Opening - slow build with rich chords instead of single notes
    {Note::C4, Note::WHOLE}, {Note::E4, Note::WHOLE}, {Note::G4, Note::WHOLE}, {Note::REST, Note::QUARTER},
    {Note::C4, Note::HALF}, {Note::E4, Note::HALF}, {Note::G3, Note::HALF}, {Note::B3, Note::HALF}, 
    {Note::C4, Note::HALF}, {Note::E4, Note::HALF},
    {Note::F4, Note::HALF}, {Note::A4, Note::HALF}, {Note::C4, Note::QUARTER}, {Note::E4, Note::QUARTER}, 
    {Note::G3, Note::QUARTER}, {Note::B3, Note::QUARTER},
    
    // Main melodic phrase - chord harmonies with the iconic rising pattern
    {Note::A3, Note::HALF}, {Note::C4, Note::HALF}, {Note::C4, Note::QUARTER}, {Note::E4, Note::QUARTER}, 
    {Note::F4, Note::QUARTER}, {Note::A4, Note::QUARTER},
    {Note::G4, Note::HALF}, {Note::B4, Note::HALF}, {Note::F4, Note::QUARTER}, {Note::A4, Note::QUARTER}, 
    {Note::C4, Note::QUARTER}, {Note::E4, Note::QUARTER},
    {Note::A3, Note::HALF}, {Note::C4, Note::HALF}, {Note::F3, Note::QUARTER}, {Note::A3, Note::QUARTER}, 
    {Note::C4, Note::QUARTER}, {Note::E4, Note::QUARTER},
    
    // Development - building intensity with layered chords
    {Note::F4, Note::QUARTER}, {Note::A4, Note::QUARTER}, {Note::G4, Note::QUARTER}, {Note::B4, Note::QUARTER}, 
    {Note::A4, Note::HALF}, {Note::C5, Note::HALF},
    {Note::G4, Note::QUARTER}, {Note::B4, Note::QUARTER}, {Note::F4, Note::QUARTER}, {Note::A4, Note::QUARTER}, 
    {Note::C4, Note::HALF}, {Note::E4, Note::HALF},
    {Note::F4, Note::QUARTER}, {Note::A4, Note::QUARTER}, {Note::A4, Note::QUARTER}, {Note::C5, Note::QUARTER}, 
    {Note::C5, Note::HALF}, {Note::E5, Note::HALF},
    
    // Emotional climax - powerful chord combinations
    {Note::A4, Note::HALF}, {Note::C5, Note::HALF}, {Note::G4, Note::QUARTER}, {Note::B4, Note::QUARTER}, 
    {Note::F4, Note::QUARTER}, {Note::A4, Note::QUARTER},
    {Note::C5, Note::WHOLE}, {Note::E5, Note::WHOLE}, {Note::A4, Note::HALF}, {Note::C5, Note::HALF},
    {Note::F4, Note::QUARTER}, {Note::A4, Note::QUARTER}, {Note::G4, Note::QUARTER}, {Note::B4, Note::QUARTER}, 
    {Note::A4, Note::HALF}, {Note::C5, Note::HALF},
    
    // Resolution - return to harmonic home
    {Note::G4, Note::QUARTER}, {Note::B4, Note::QUARTER}, {Note::F4, Note::QUARTER}, {Note::A4, Note::QUARTER}, 
    {Note::C4, Note::HALF}, {Note::E4, Note::HALF},
    {Note::A3, Note::HALF}, {Note::C4, Note::HALF}, {Note::F3, Note::QUARTER}, {Note::A3, Note::QUARTER}, 
    {Note::C4, Note::QUARTER}, {Note::E4, Note::QUARTER},
    {Note::C4, Note::WHOLE}, {Note::E4, Note::WHOLE}, {Note::G4, Note::WHOLE}, {Note::REST, Note::HALF}
};

// ============================================================================
// CONSTRUCTOR & DESTRUCTOR - Simplified
// ============================================================================

Note::Note(I2SSpeaker* speaker, Utils::Logger* logger)
    : _speaker(speaker), _logger(logger), _amplitude(DEFAULT_AMPLITUDE), _soundType(GUITAR) {
    
    if (_logger) {
        _logger->debug("Note: Audio system initialized with %s sound", getSoundTypeName());
    }
}

Note::~Note() {
    // Simple cleanup
}

// ============================================================================
// CORE AUDIO GENERATION - Simplified and efficient
// ============================================================================

bool Note::playFrequency(uint16_t frequency, uint32_t durationMs) {
    if (!_speaker) {
        if (_logger) _logger->error("Note: Speaker not initialized");
        return false;
    }

    if (_logger) _logger->debug("Playing frequency %d Hz for %d ms", frequency, durationMs);

    // Calculate buffer parameters
    size_t channelCount = (_speaker->getChannelMode() == I2S_SLOT_MODE_STEREO) ? 2 : 1;
    size_t samplesNeeded = (SAMPLE_RATE * durationMs) / 1000;
    size_t totalSamples = samplesNeeded * channelCount;

    // Allocate buffer
    int16_t* buffer = (int16_t*)malloc(totalSamples * sizeof(int16_t));
    if (!buffer) {
        if (_logger) _logger->error("Note: Failed to allocate audio buffer");
        return false;
    }

    // Generate wave based on sound type
    generateWave(frequency, durationMs, buffer, totalSamples, channelCount);

    // Start speaker if needed
    if (!_speaker->isActive()) {
        _speaker->start();
    }

    // Play audio
    int samplesWritten = _speaker->writeSamples(buffer, totalSamples, 1000);
    bool success = (samplesWritten > 0);

    if (_logger) _logger->debug("Audio playback %s (%d samples)", success ? "successful" : "failed", samplesWritten);

    free(buffer);
    return success;
}

// ============================================================================
// MELODY PLAYBACK - Simplified
// ============================================================================

bool Note::playMelody(Melody melody, int repeatCount) {
    if (!_speaker) {
        if (_logger) _logger->error("Note: Speaker not initialized");
        return false;
    }

    size_t noteCount;
    const MusicNote* notes = getMelodyNotes(melody, &noteCount);

    if (!notes || noteCount == 0) {
        if (_logger) _logger->error("Note: Invalid melody");
        return false;
    }

    if (_logger) _logger->debug("Playing melody with %d notes", noteCount);

    // Simple melody playback
    for (size_t i = 0; i < noteCount; i++) {
        if (!playFrequency(notes[i].frequency, notes[i].duration)) {
            if (_logger) _logger->error("Note: Failed to play note %zu", i);
        }
        
        // Small gap between notes
        if (i < noteCount - 1) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }

    return true;
}

bool Note::playCustomMelody(const MusicNote* notes, size_t noteCount, int repeatCount) {
    if (!_speaker || !notes) {
        if (_logger) _logger->error("Note: Speaker or notes not available");
        return false;
    }

    if (_logger) _logger->debug("Playing custom melody with %d notes", noteCount);

    // Simple melody playback
    for (size_t i = 0; i < noteCount; i++) {
        if (!playFrequency(notes[i].frequency, notes[i].duration)) {
            if (_logger) _logger->error("Note: Failed to play note %zu", i);
        }
        
        // Small gap between notes
        if (i < noteCount - 1) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }

    return true;
}

// ============================================================================
// CHORD PLAYBACK - Simplified
// ============================================================================

bool Note::playChord(const Frequency* frequencies, size_t noteCount, uint32_t durationMs) {
    if (!_speaker || !frequencies || noteCount == 0) {
        if (_logger) _logger->error("Note: Invalid chord parameters");
        return false;
    }

    if (_logger) _logger->debug("Playing chord with %d notes for %d ms", noteCount, durationMs);

    // Calculate buffer parameters
    size_t channelCount = (_speaker->getChannelMode() == I2S_SLOT_MODE_STEREO) ? 2 : 1;
    size_t samplesNeeded = (SAMPLE_RATE * durationMs) / 1000;
    size_t totalSamples = samplesNeeded * channelCount;

    // Allocate buffer
    int16_t* buffer = (int16_t*)malloc(totalSamples * sizeof(int16_t));
    if (!buffer) {
        if (_logger) _logger->error("Note: Failed to allocate chord buffer");
        return false;
    }

    // Initialize buffer with zeros for mixing
    memset(buffer, 0, totalSamples * sizeof(int16_t));

    // Mix all frequencies into the buffer
    for (size_t freq_idx = 0; freq_idx < noteCount; freq_idx++) {
        if (frequencies[freq_idx] == REST) continue;
        
        // Generate wave for this frequency and mix it
        int16_t* tempBuffer = (int16_t*)malloc(totalSamples * sizeof(int16_t));
        if (tempBuffer) {
            generateWave(frequencies[freq_idx], durationMs, tempBuffer, totalSamples, channelCount);
            
            // Mix into main buffer with volume scaling
            for (size_t i = 0; i < totalSamples; i++) {
                int32_t mixed = buffer[i] + (tempBuffer[i] / (int)noteCount);
                // Clamp to 16-bit range
                if (mixed > 32767) mixed = 32767;
                if (mixed < -32767) mixed = -32767;
                buffer[i] = (int16_t)mixed;
            }
            
            free(tempBuffer);
        }
    }

    // Start speaker if needed
    if (!_speaker->isActive()) {
        _speaker->start();
    }

    // Play mixed buffer
    int samplesWritten = _speaker->writeSamples(buffer, totalSamples, 1000);
    bool success = (samplesWritten > 0);

    if (_logger) _logger->debug("Chord playback %s", success ? "successful" : "failed");

    free(buffer);
    return success;
}

bool Note::playChord(Frequency rootNote, ChordType type, uint32_t durationMs) {
    if (rootNote == REST) {
        return playFrequency(0, durationMs); // Play silence
    }
    
    // Generate chord frequencies (simplified - just major chords for now)
    Frequency chordNotes[4];
    size_t noteCount = 0;
    
    chordNotes[noteCount++] = rootNote; // Root
    
    switch (type) {
        case MAJOR:
            chordNotes[noteCount++] = (Frequency)(rootNote * 1.26); // Major 3rd
            chordNotes[noteCount++] = (Frequency)(rootNote * 1.5);  // Perfect 5th
            break;
        case MINOR:
            chordNotes[noteCount++] = (Frequency)(rootNote * 1.19); // Minor 3rd
            chordNotes[noteCount++] = (Frequency)(rootNote * 1.5);  // Perfect 5th
            break;
        default:
            // Just play root note for other chord types
            break;
    }
    
    return playChord(chordNotes, noteCount, durationMs);
}

// ============================================================================
// UNIFIED WAVE GENERATION - Much simpler approach
// ============================================================================

void Note::generateWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || totalSamples == 0) return;
    
    if (frequency == 0) {
        // Generate silence
        memset(buffer, 0, totalSamples * sizeof(int16_t));
        return;
    }
    
    size_t samplesPerChannel = totalSamples / channelCount;
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    double phase = 0.0;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        int16_t sample = 0;
        
        switch (_soundType) {
            case PIANO:
            case GUITAR:
            default:
                // Simple sine wave for most instruments
                sample = (int16_t)(_amplitude * sin(phase));
                break;
                
            case SQUARE_WAVE:
                // Square wave
                sample = (phase < M_PI) ? _amplitude : -_amplitude;
                break;
                
            case SAWTOOTH:
                // Sawtooth wave
                sample = (int16_t)(_amplitude * (2.0 * phase / (2.0 * M_PI) - 1.0));
                break;
        }
        
        // Apply to all channels
        for (size_t ch = 0; ch < channelCount; ch++) {
            buffer[i * channelCount + ch] = sample;
        }
        
        phase += phaseIncrement;
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }
    
    // Apply simple fade to prevent clicks
    size_t fadeLength = (samplesPerChannel < 20) ? samplesPerChannel / 4 : (size_t)(SAMPLE_RATE * 0.005);
    if (fadeLength > samplesPerChannel / 2) fadeLength = samplesPerChannel / 2;
    applyFade(buffer, totalSamples, fadeLength * channelCount, fadeLength * channelCount);
}

void Note::applyFade(int16_t* buffer, size_t sampleCount, size_t fadeInSamples, size_t fadeOutSamples) {
    if (!buffer || sampleCount == 0) return;
    
    // Fade in
    for (size_t i = 0; i < fadeInSamples && i < sampleCount; i++) {
        float factor = (float)i / fadeInSamples;
        buffer[i] = (int16_t)(buffer[i] * factor);
    }
    
    // Fade out
    size_t fadeOutStart = (sampleCount > fadeOutSamples) ? sampleCount - fadeOutSamples : 0;
    for (size_t i = fadeOutStart; i < sampleCount; i++) {
        float factor = (float)(sampleCount - i) / fadeOutSamples;
        buffer[i] = (int16_t)(buffer[i] * factor);
    }
}

// ============================================================================
// UTILITY METHODS - Simplified
// ============================================================================

const Note::MusicNote* Note::getMelodyNotes(Melody melody, size_t* noteCount) {
    switch (melody) {
        case DOREMI_SCALE:
            *noteCount = sizeof(DOREMI_SCALE_NOTES) / sizeof(DOREMI_SCALE_NOTES[0]);
            return DOREMI_SCALE_NOTES;

        case HAPPY_BIRTHDAY:
            *noteCount = sizeof(HAPPY_BIRTHDAY_NOTES) / sizeof(HAPPY_BIRTHDAY_NOTES[0]);
            return HAPPY_BIRTHDAY_NOTES;

        case RANDOM:
            *noteCount = sizeof(RANDOM_NOTES) / sizeof(RANDOM_NOTES[0]);
            return RANDOM_NOTES;

        default:
            *noteCount = 0;
            return nullptr;
    }
}

// Volume control
void Note::setVolume(uint8_t volumePercent) {
    _amplitude = (DEFAULT_AMPLITUDE * volumePercent) / 100;
    if (_logger) _logger->debug("Note: Volume set to %d%% (amplitude: %d)", volumePercent, _amplitude);
}

void Note::setVolumeRaw(uint16_t amplitude) {
    _amplitude = (amplitude > 32767) ? 32767 : amplitude;
    if (_logger) _logger->debug("Note: Raw amplitude set to %d", _amplitude);
}

uint8_t Note::getVolume() const {
    return (_amplitude * 100) / DEFAULT_AMPLITUDE;
}

uint16_t Note::getVolumeRaw() const {
    return _amplitude;
}

// Sound type control
void Note::setSoundType(SoundType soundType) {
    _soundType = soundType;
    if (_logger) _logger->debug("Note: Sound type set to %s", getSoundTypeName());
}

Note::SoundType Note::getSoundType() const {
    return _soundType;
}

const char* Note::getSoundTypeName() const {
    switch (_soundType) {
        case PIANO: return "Piano";
        case GUITAR: return "Guitar";
        case ORGAN: return "Organ";
        case FLUTE: return "Flute";
        case BELL: return "Bell";
        case SQUARE_WAVE: return "Square Wave";
        case SAWTOOTH: return "Sawtooth";
        case TRIANGLE: return "Triangle";
        default: return "Unknown";
    }
}

// System status
bool Note::isReady() const {
    return (_speaker != nullptr);
}

void Note::stop() {
    // Simple stop implementation
    if (_logger) _logger->debug("Note: Stopping audio system");
}

void Note::interrupt() {
    // Simple interrupt implementation
    if (_logger) _logger->debug("Note: Audio interrupted");
}

// ============================================================================
// SIMPLIFIED API - Remove complex polyphonic features for event-driven usage
// ============================================================================

bool Note::playScale(Frequency startNote, bool ascending) {
    // Simple scale implementation
    const Frequency scale[] = {C4, D4, E4, F4, G4, A4, B4, C5};
    size_t scaleSize = sizeof(scale) / sizeof(scale[0]);
    
    for (size_t i = 0; i < scaleSize; i++) {
        size_t index = ascending ? i : (scaleSize - 1 - i);
        if (!playFrequency(scale[index], QUARTER)) {
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    return true;
}

bool Note::generateRandomMelody(size_t noteCount, MusicNote* outputBuffer) {
    if (!outputBuffer) return false;
    
    const Frequency notes[] = {C4, D4, E4, F4, G4, A4, B4, C5};
    const Duration durations[] = {QUARTER, EIGHTH, HALF};
    
    for (size_t i = 0; i < noteCount; i++) {
        outputBuffer[i].frequency = notes[rand() % (sizeof(notes)/sizeof(notes[0]))];
        outputBuffer[i].duration = durations[rand() % (sizeof(durations)/sizeof(durations[0]))];
    }
    
    return true;
}

bool Note::generateRandomMelody(size_t noteCount, MusicNote* outputBuffer, Frequency startNote, Frequency* endNote) {
    // Simple wrapper for the above method
    return generateRandomMelody(noteCount, outputBuffer);
}

bool Note::playRandomMelody(size_t noteCount, int repeatCount) {
    MusicNote* melody = (MusicNote*)malloc(noteCount * sizeof(MusicNote));
    if (!melody) return false;
    
    bool success = generateRandomMelody(noteCount, melody);
    if (success) {
        success = playCustomMelody(melody, noteCount, repeatCount);
    }
    
    free(melody);
    return success;
}
