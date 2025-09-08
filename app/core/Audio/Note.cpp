#include "Note.h"
#include <algorithm>

// Musical note frequencies (in Hz) mapped to array indices
static const Note::MusicNote DOREMI_SCALE_NOTES[] = {
    {Note::C4, Note::QUARTER},
    {Note::D4, Note::QUARTER},
    {Note::E4, Note::QUARTER},
    {Note::F4, Note::QUARTER},
    {Note::G4, Note::QUARTER},
    {Note::A4, Note::QUARTER},
    {Note::B4, Note::QUARTER},
    {Note::C5, Note::HALF}
};

static const Note::MusicNote SPACE_THEME_NOTES[] = {
    {Note::C4, Note::QUARTER},
    {Note::E4, Note::QUARTER},
    {Note::G4, Note::QUARTER},
    {Note::C5, Note::QUARTER},
    {Note::E5, Note::QUARTER},
    {Note::G4, Note::QUARTER},
    {Note::C4, Note::HALF}
};

static const Note::MusicNote STAR_WARS_THEME_NOTES[] = {
    {Note::G3, Note::QUARTER},
    {Note::G3, Note::QUARTER},
    {Note::G3, Note::QUARTER},
    {Note::D3, Note::EIGHTH},
    {Note::A3, Note::EIGHTH},
    {Note::G3, Note::QUARTER},
    {Note::D3, Note::EIGHTH},
    {Note::A3, Note::EIGHTH},
    {Note::G3, Note::HALF}
};

static const Note::MusicNote UFO_LANDING_NOTES[] = {
    {Note::E5, Note::SIXTEENTH},
    {Note::D5, Note::SIXTEENTH},
    {Note::C5, Note::SIXTEENTH},
    {Note::B4, Note::SIXTEENTH},
    {Note::A4, Note::EIGHTH},
    {Note::REST, Note::EIGHTH},
    {Note::F4, Note::QUARTER},
    {Note::REST, Note::QUARTER},
    {Note::C4, Note::HALF}
};

static const Note::MusicNote COSMIC_JOURNEY_NOTES[] = {
    {Note::C3, Note::QUARTER},
    {Note::F3, Note::QUARTER},
    {Note::A3, Note::QUARTER},
    {Note::C4, Note::QUARTER},
    {Note::F4, Note::QUARTER},
    {Note::A4, Note::QUARTER},
    {Note::C5, Note::HALF}
};

static const Note::MusicNote ALIEN_CONTACT_NOTES[] = {
    {Note::F5, Note::SIXTEENTH},
    {Note::E5, Note::SIXTEENTH},
    {Note::D5, Note::EIGHTH},
    {Note::REST, Note::EIGHTH},
    {Note::B4, Note::SIXTEENTH},
    {Note::A4, Note::SIXTEENTH},
    {Note::G4, Note::EIGHTH},
    {Note::REST, Note::QUARTER}
};

static const Note::MusicNote WARP_SPEED_NOTES[] = {
    {Note::C3, Note::SIXTEENTH},
    {Note::E3, Note::SIXTEENTH},
    {Note::G3, Note::SIXTEENTH},
    {Note::C4, Note::SIXTEENTH},
    {Note::E4, Note::SIXTEENTH},
    {Note::G4, Note::SIXTEENTH},
    {Note::C5, Note::SIXTEENTH},
    {Note::E5, Note::EIGHTH}
};

Note::Note(I2SSpeaker* speaker, Utils::Logger* logger) 
    : _speaker(speaker), _logger(logger) {
    if (_logger) {
        _logger->info("Note musical system initialized");
    }
}

Note::~Note() {
    // No cleanup needed - simplified approach
}

bool Note::playFrequency(uint16_t frequency, uint32_t durationMs) {
    return playFrequencyInternal(frequency, durationMs, true);
}

bool Note::playFrequencyInternal(uint16_t frequency, uint32_t durationMs, bool checkPlaying) {
    if (!_speaker) {
        _logger->error("Speaker not available");
        return false;
    }
    
    // Simplified - remove race condition with _isPlaying flag
    _logger->info("Playing frequency %d Hz for %d ms", frequency, durationMs);
    
    // Calculate buffer size for the duration (handle stereo/mono channels)
    size_t channelCount = (_speaker->getChannelMode() == I2S_SLOT_MODE_STEREO) ? 2 : 1;
    size_t samplesNeeded = (SAMPLE_RATE * durationMs) / 1000;
    size_t totalSamples = samplesNeeded * channelCount;
    size_t bufferSize = totalSamples * sizeof(int16_t);
    
    _logger->info("Buffer size: %d samples (%d channels), %d bytes", samplesNeeded, channelCount, bufferSize);
    
    int16_t* buffer = (int16_t*)malloc(bufferSize);
    if (!buffer) {
        _logger->error("Failed to allocate audio buffer");
        return false;
    }
    
    if (frequency == 0) {
        _logger->info("Generating silence");
        generateSilence(durationMs, buffer, totalSamples);
    } else {
        _logger->info("Generating sine wave");
        generateSineWave(frequency, durationMs, buffer, totalSamples, channelCount);
    }
    
    // Apply fade to prevent clicks (like AudioSamples does)
    size_t fadeLength = std::min(samplesNeeded / 20, (size_t)(SAMPLE_RATE * 0.005)); // 5ms fade
    applyFade(buffer, totalSamples, fadeLength * channelCount, fadeLength * channelCount);
    
    // Start speaker if not active
    if (!_speaker->isActive()) {
        _speaker->start();
    }
    
    // Play the buffer using writeSamples with timeout (blocking call like AudioSamples)
    _logger->info("Calling speaker->writeSamples() with %d samples", totalSamples);
    int samplesWritten = _speaker->writeSamples(buffer, totalSamples, 1000); // 1000ms timeout
    bool result = (samplesWritten > 0);
    _logger->info("Speaker call completed, %d samples written, success: %s", samplesWritten, result ? "true" : "false");
    
    free(buffer);
    
    return result;
}

bool Note::playMelody(Melody melody) {
    if (!_speaker) {
        _logger->error("Speaker not available");
        return false;
    }
    
    // Simple blocking approach - no complex task management
    size_t noteCount;
    const MusicNote* notes = getMelodyNotes(melody, &noteCount);
    
    if (!notes || noteCount == 0) {
        _logger->error("Invalid melody - no notes found");
        return false;
    }
    
    _logger->info("Playing melody with %d notes", noteCount);
    
    // Play each note sequentially (blocking)
    for (size_t i = 0; i < noteCount; i++) {
        _logger->info("Playing note %d: freq=%d, dur=%d", i, notes[i].frequency, notes[i].duration);
        
        bool result = playFrequencyInternal(notes[i].frequency, notes[i].duration, false);
        if (!result) {
            _logger->error("Failed to play note %d", i);
            return false;
        }
        
        // Small gap between notes for clarity
        if (i < noteCount - 1) {  // Don't delay after last note
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    
    _logger->info("Melody playback completed successfully");
    return true;
}

bool Note::playCustomMelody(const MusicNote* notes, size_t noteCount) {
    if (!_speaker || !notes) {
        _logger->error("Speaker or notes not available");
        return false;
    }
    
    _logger->info("Playing custom melody with %d notes", noteCount);
    
    for (size_t i = 0; i < noteCount; i++) {
        bool result = playFrequencyInternal(notes[i].frequency, notes[i].duration, false);
        if (!result) {
            _logger->error("Failed to play custom note %d", i);
            return false;
        }
        
        if (i < noteCount - 1) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    
    return true;
}

bool Note::playScale(Frequency startNote, bool ascending) {
    if (!_speaker) return false;
    
    Frequency scale[] = {C4, D4, E4, F4, G4, A4, B4, C5};
    
    // Find starting position in scale
    int startIdx = 0;
    for (int i = 0; i < 8; i++) {
        if (scale[i] == startNote) {
            startIdx = i;
            break;
        }
    }
    
    if (ascending) {
        for (int i = startIdx; i < 8; i++) {
            playFrequencyInternal(scale[i], QUARTER, false);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    } else {
        for (int i = startIdx; i >= 0; i--) {
            playFrequencyInternal(scale[i], QUARTER, false);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    
    return true;
}

void Note::generateSineWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || frequency == 0) return;
    
    size_t samplesPerChannel = totalSamples / channelCount;
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    double phase = 0.0;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        int16_t sample = (int16_t)(AMPLITUDE * sin(phase));
        phase += phaseIncrement;
        
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        
        // Fill all channels with the same sample
        for (size_t ch = 0; ch < channelCount; ch++) {
            buffer[i * channelCount + ch] = sample;
        }
    }
}

void Note::generateSquareWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t bufferSize) {
    if (!buffer || frequency == 0) return;
    
    uint32_t samplesPerCycle = SAMPLE_RATE / frequency;
    uint32_t halfCycle = samplesPerCycle / 2;
    
    for (size_t i = 0; i < bufferSize; i++) {
        uint32_t cyclePos = i % samplesPerCycle;
        buffer[i] = (cyclePos < halfCycle) ? AMPLITUDE : -AMPLITUDE;
    }
}

void Note::generateSilence(uint32_t durationMs, int16_t* buffer, size_t bufferSize) {
    if (!buffer) return;
    
    memset(buffer, 0, bufferSize * sizeof(int16_t));
}

void Note::applyFade(int16_t* buffer, size_t sampleCount, size_t fadeInSamples, size_t fadeOutSamples) {
    if (!buffer || sampleCount == 0) return;
    
    // Apply fade in
    for (size_t i = 0; i < std::min(fadeInSamples, sampleCount); i++) {
        float fadeMultiplier = (float)i / fadeInSamples;
        buffer[i] = (int16_t)(buffer[i] * fadeMultiplier);
    }
    
    // Apply fade out
    size_t fadeOutStart = (sampleCount > fadeOutSamples) ? (sampleCount - fadeOutSamples) : 0;
    for (size_t i = fadeOutStart; i < sampleCount; i++) {
        float fadeMultiplier = (float)(sampleCount - i) / fadeOutSamples;
        buffer[i] = (int16_t)(buffer[i] * fadeMultiplier);
    }
}

const Note::MusicNote* Note::getMelodyNotes(Melody melody, size_t* noteCount) {
    switch (melody) {
        case DOREMI_SCALE:
            *noteCount = sizeof(DOREMI_SCALE_NOTES) / sizeof(MusicNote);
            return DOREMI_SCALE_NOTES;
            
        case SPACE_THEME:
            *noteCount = sizeof(SPACE_THEME_NOTES) / sizeof(MusicNote);
            return SPACE_THEME_NOTES;
            
        case STAR_WARS_THEME:
            *noteCount = sizeof(STAR_WARS_THEME_NOTES) / sizeof(MusicNote);
            return STAR_WARS_THEME_NOTES;
            
        case UFO_LANDING:
            *noteCount = sizeof(UFO_LANDING_NOTES) / sizeof(MusicNote);
            return UFO_LANDING_NOTES;
            
        case COSMIC_JOURNEY:
            *noteCount = sizeof(COSMIC_JOURNEY_NOTES) / sizeof(MusicNote);
            return COSMIC_JOURNEY_NOTES;
            
        case ALIEN_CONTACT:
            *noteCount = sizeof(ALIEN_CONTACT_NOTES) / sizeof(MusicNote);
            return ALIEN_CONTACT_NOTES;
            
        case WARP_SPEED:
            *noteCount = sizeof(WARP_SPEED_NOTES) / sizeof(MusicNote);
            return WARP_SPEED_NOTES;
            
        default:
            *noteCount = 0;
            return nullptr;
    }
}

bool Note::isReady() const {
    return (_speaker != nullptr && _speaker->isInitialized());
}

// Simplified methods - no complex task management
void Note::setVolume(uint8_t volume) {
    // Volume control can be implemented later
}

void Note::setWaveform(bool useSquareWave) {
    // Waveform switching can be implemented later
}
