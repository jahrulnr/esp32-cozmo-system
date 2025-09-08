#include "Note.h"
#include <algorithm>

// Musical note frequencies (in Hz) mapped to array indices
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

// Happy Birthday melody - traditional version
static const Note::MusicNote HAPPY_BIRTHDAY_NOTES[] = {
    // "Happy Birth-day to you" (first line)
    {Note::G4, Note::EIGHTH},
    {Note::G4, Note::EIGHTH},
    {Note::A4, Note::QUARTER},
    {Note::G4, Note::QUARTER},
    {Note::C5, Note::QUARTER},
    {Note::B4, Note::HALF},
    
    // "Happy Birth-day to you" (second line)
    {Note::G4, Note::EIGHTH},
    {Note::G4, Note::EIGHTH},
    {Note::A4, Note::QUARTER},
    {Note::G4, Note::QUARTER},
    {Note::D5, Note::QUARTER},
    {Note::C5, Note::HALF},
    
    // "Happy Birth-day dear [name]" (third line - higher)
    {Note::G4, Note::EIGHTH},
    {Note::G4, Note::EIGHTH},
    {Note::G5, Note::QUARTER},
    {Note::E5, Note::QUARTER},
    {Note::C5, Note::QUARTER},
    {Note::B4, Note::QUARTER},
    {Note::A4, Note::HALF},
    
    // "Happy Birth-day to you" (final line)
    {Note::F5, Note::EIGHTH},
    {Note::F5, Note::EIGHTH},
    {Note::E5, Note::QUARTER},
    {Note::C5, Note::QUARTER},
    {Note::D5, Note::QUARTER},
    {Note::C5, Note::HALF}
};

Note::Note(I2SSpeaker* speaker, Utils::Logger* logger) 
    : _speaker(speaker), _logger(logger), _interrupt(false), _amplitude(DEFAULT_AMPLITUDE), _soundType(GUITAR) {
    if (_logger) {
        _logger->debug("Note musical system initialized with default volume %d, sound: PIANO", _amplitude);
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
    _logger->debug("Playing frequency %d Hz for %d ms", frequency, durationMs);
    
    // Calculate buffer size for the duration (handle stereo/mono channels)
    size_t channelCount = (_speaker->getChannelMode() == I2S_SLOT_MODE_STEREO) ? 2 : 1;
    size_t samplesNeeded = (SAMPLE_RATE * durationMs) / 1000;
    size_t totalSamples = samplesNeeded * channelCount;
    size_t bufferSize = totalSamples * sizeof(int16_t);
    
    _logger->debug("Buffer size: %d samples (%d channels), %d bytes", samplesNeeded, channelCount, bufferSize);
    
    int16_t* buffer = (int16_t*)malloc(bufferSize);
    if (!buffer) {
        _logger->error("Failed to allocate audio buffer");
        return false;
    }
    
    if (frequency == 0) {
        _logger->debug("Generating silence");
        generateSilence(durationMs, buffer, totalSamples);
    } else {
        _logger->debug("Generating %s wave", getSoundTypeName());
        generateInstrumentWave(frequency, durationMs, buffer, totalSamples, channelCount, _soundType);
    }
    
    // Apply fade to prevent clicks (like AudioSamples does)
    size_t fadeLength = std::min(samplesNeeded / 20, (size_t)(SAMPLE_RATE * 0.005)); // 5ms fade
    applyFade(buffer, totalSamples, fadeLength * channelCount, fadeLength * channelCount);
    
    // Start speaker if not active
    if (!_speaker->isActive()) {
        _speaker->start();
    }
    
    // Play the buffer using writeSamples with timeout (blocking call like AudioSamples)
    _logger->debug("Calling speaker->writeSamples() with %d samples", totalSamples);
    int samplesWritten = _speaker->writeSamples(buffer, totalSamples, 1000); // 1000ms timeout
    bool result = (samplesWritten > 0);
    _logger->debug("Speaker call completed, %d samples written, success: %s", samplesWritten, result ? "true" : "false");
    
    free(buffer);
    
    return result;
}

bool Note::playMelody(Melody melody, int repeatCount) {
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
    
    _logger->debug("Playing melody with %d notes, repeat: %d", noteCount, repeatCount);
    
    // Handle repeat logic: 0 = once, -1 = forever, >0 = specific count
    int actualRepeats = (repeatCount == 0) ? 1 : repeatCount;
    bool forever = (repeatCount == -1);
    int currentRepeat = 0;
    
    do {
        // Play each note sequentially (blocking)
        for (size_t i = 0; i < noteCount; i++) {
            // Check for interrupt before playing each note
            if (_interrupt) {
                _interrupt = false;
                return true;
            }
            
            _logger->debug("Playing note %d: freq=%d, dur=%d", i, notes[i].frequency, notes[i].duration);
            
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

        if (_interrupt) {
            _interrupt = false;
            return true;
        }
        
        currentRepeat++;
        
        // Add a longer pause between repetitions
        if ((forever || currentRepeat < actualRepeats) && noteCount > 1) {
            vTaskDelay(pdMS_TO_TICKS(500)); // 500ms pause between melody repetitions
        }
        
    } while (forever || currentRepeat < actualRepeats);
    
    _logger->debug("Melody playback completed successfully");
    return true;
}

bool Note::playCustomMelody(const MusicNote* notes, size_t noteCount, int repeatCount) {
    if (!_speaker || !notes) {
        _logger->error("Speaker or notes not available");
        return false;
    }
    
    _logger->debug("Playing custom melody with %d notes, repeat: %d", noteCount, repeatCount);
    
    // Handle repeat logic: 0 = once, -1 = forever, >0 = specific count
    int actualRepeats = (repeatCount == 0) ? 1 : repeatCount;
    bool forever = (repeatCount == -1);
    int currentRepeat = 0;
    
    do {
        for (size_t i = 0; i < noteCount; i++) {
            // Check for interrupt before playing each note
            if (_interrupt) {
                _interrupt = false;
                return true;
            }
            
            bool result = playFrequencyInternal(notes[i].frequency, notes[i].duration, false);
            if (!result) {
                _logger->error("Failed to play custom note %d", i);
                return false;
            }
            
            if (i < noteCount - 1) {
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }

        if (_interrupt) {
            _interrupt = false;
            return true;
        }
        
        currentRepeat++;
        
        // Add a longer pause between repetitions
        if ((forever || currentRepeat < actualRepeats) && noteCount > 1) {
            vTaskDelay(pdMS_TO_TICKS(500)); // 500ms pause between melody repetitions
        }
        
    } while (forever || currentRepeat < actualRepeats);
    
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

bool Note::generateRandomMelody(size_t noteCount, MusicNote* outputBuffer) {
    // Default version - starts on C or G, ends on C
    return generateRandomMelody(noteCount, outputBuffer, (Frequency)0, nullptr);
}

bool Note::generateRandomMelody(size_t noteCount, MusicNote* outputBuffer, Frequency startNote, Frequency* endNote) {
    if (!outputBuffer || noteCount == 0) {
        if (_logger) _logger->error("Invalid parameters for melody generation");
        return false;
    }
    
    // C major scale notes in order (C, D, E, F, G, A, B)
    Frequency cMajorScale[] = {C4, D4, E4, F4, G4, A4, B4};
    const size_t scaleSize = 7;
    
    // Helper function to find scale index for a given frequency
    auto findScaleIndex = [&](Frequency freq) -> int {
        for (int i = 0; i < (int)scaleSize; i++) {
            if (cMajorScale[i] == freq) return i;
        }
        return -1; // Not found
    };
    
    // Initialize random seed with current time (millis)
    uint32_t seed = millis();
    if (seed == 0) seed = 1; // Ensure non-zero seed
    
    // Simple linear congruential generator for randomness
    auto random = [&seed](int max) -> int {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        return seed % max;
    };
    
    // Helper function to get weighted random duration
    auto getRandomDuration = [&]() -> Duration {
        int totalWeight = 100;
        int randomValue = random(totalWeight);
        
        if (randomValue < 60) return QUARTER;      // 60%
        else if (randomValue < 90) return EIGHTH;  // 30%
        else return HALF;                          // 10%
    };
    
    // Helper function to get random step movement
    auto getRandomStep = [&]() -> int {
        int randomValue = random(100);
        int step;
        
        if (randomValue < 60) {
            step = 1; // ±1 step (60%)
        } else if (randomValue < 85) {
            step = 2; // ±2 steps (25%)
        } else {
            step = 3; // ±3 steps (15%)
        }
        
        // Randomly choose positive or negative direction
        return (random(2) == 0) ? step : -step;
    };
    
    // Determine starting note
    int currentScaleIndex;
    bool forceEndOnC = false;
    
    if (startNote == (Frequency)0) {
        // Default behavior - choose C or G randomly, force end on C
        currentScaleIndex = (random(2) == 0) ? 0 : 4; // C=0, G=4
        forceEndOnC = true;
        _logger->debug("Generating random melody with %d notes, starting on %s (auto-chosen)", 
            noteCount, (currentScaleIndex == 0) ? "C" : "G");
    } else {
        // Use provided starting note
        currentScaleIndex = findScaleIndex(startNote);
        if (currentScaleIndex == -1) {
            // Invalid starting note, fall back to C
            currentScaleIndex = 0;
            _logger->warning("Invalid starting note, falling back to C");
        }
        // Don't force end on C when continuing a melody
        forceEndOnC = false;
        const char* noteNames[] = {"C", "D", "E", "F", "G", "A", "B"};
        _logger->debug("Generating random melody with %d notes, starting on %s (specified)", 
            noteCount, noteNames[currentScaleIndex]);
    }
    
    int previousNote = -1; // Track previous note to avoid too many repetitions
    int repeatCount = 0;   // Count consecutive same notes
    
    for (size_t i = 0; i < noteCount; i++) {
        // For the last note, optionally force it to be C (only if forceEndOnC is true)
        if (i == noteCount - 1 && forceEndOnC) {
            currentScaleIndex = 0; // C is at index 0
        } else {
            // Apply movement rules for non-final notes (or when not forcing end on C)
            if (i > 0) {
                int step = getRandomStep();
                int newIndex = currentScaleIndex + step;
                
                // Keep within scale bounds (0-6)
                if (newIndex < 0) newIndex = 0;
                if (newIndex >= (int)scaleSize) newIndex = scaleSize - 1;
                
                // Avoid repeating the same note more than 2 times
                if (newIndex == previousNote) {
                    repeatCount++;
                    if (repeatCount >= 2) {
                        // Force a different note
                        if (newIndex > 0 && newIndex < (int)scaleSize - 1) {
                            newIndex += (random(2) == 0) ? 1 : -1;
                        } else if (newIndex == 0) {
                            newIndex = 1;
                        } else {
                            newIndex = scaleSize - 2;
                        }
                        repeatCount = 0;
                    }
                } else {
                    repeatCount = 0;
                }
                
                currentScaleIndex = newIndex;
            }
        }
        
        // Choose duration (use HALF more frequently at phrase endings)
        Duration duration;
        if (i == noteCount - 1 || (i > 0 && (i + 1) % 4 == 0)) {
            // Last note or phrase ending - higher chance for HALF
            int randomValue = random(100);
            if (randomValue < 40) duration = QUARTER;      // 40%
            else if (randomValue < 60) duration = EIGHTH;  // 20%
            else duration = HALF;                          // 40%
        } else {
            duration = getRandomDuration();
        }
        
        // Set the note in the output buffer
        outputBuffer[i].frequency = cMajorScale[currentScaleIndex];
        outputBuffer[i].duration = duration;
        
        previousNote = currentScaleIndex;
        
        if (_logger) {
            const char* noteNames[] = {"C", "D", "E", "F", "G", "A", "B"};
            const char* durName = (duration == QUARTER) ? "QUARTER" : 
                                 (duration == EIGHTH) ? "EIGHTH" : "HALF";
            _logger->debug("Note %d: %s4 (%dHz) - %s (%dms)", 
                          i, noteNames[currentScaleIndex], 
                          cMajorScale[currentScaleIndex], durName, duration);
        }
    }
    
    // Return the ending note if requested
    if (endNote) {
        *endNote = cMajorScale[currentScaleIndex];
    }
    
    if (_logger) {
        const char* noteNames[] = {"C", "D", "E", "F", "G", "A", "B"};
        _logger->info("Random melody generated successfully with %d notes, ending on %s", 
                     noteCount, noteNames[currentScaleIndex]);
    }
    
    return true;
}

bool Note::playRandomMelody(size_t noteCount, int repeatCount) {
    if (!_speaker || noteCount == 0) {
        if (_logger) _logger->error("Speaker not available or invalid note count");
        return false;
    }
    
    // Allocate buffer for the melody
    MusicNote* melodyBuffer = (MusicNote*)malloc(noteCount * sizeof(MusicNote));
    if (!melodyBuffer) {
        if (_logger) _logger->error("Failed to allocate memory for random melody");
        return false;
    }
    
    // Generate the random melody
    bool success = generateRandomMelody(noteCount, melodyBuffer);
    if (!success) {
        free(melodyBuffer);
        return false;
    }
    
    if (_logger) {
        _logger->info("Playing random melody with %d notes, repeat: %d", noteCount, repeatCount);
    }
    
    // Play the generated melody
    success = playCustomMelody(melodyBuffer, noteCount, repeatCount);
    
    // Clean up
    free(melodyBuffer);
    
    return success;
}

void Note::generateInstrumentWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount, SoundType soundType) {
    switch (soundType) {
        case PIANO:
            generateSineWave(frequency, durationMs, buffer, totalSamples, channelCount);
            break;
        case GUITAR:
            generateGuitarWave(frequency, durationMs, buffer, totalSamples, channelCount);
            break;
        case ORGAN:
            generateOrganWave(frequency, durationMs, buffer, totalSamples, channelCount);
            break;
        case FLUTE:
            generateFluteWave(frequency, durationMs, buffer, totalSamples, channelCount);
            break;
        case BELL:
            generateBellWave(frequency, durationMs, buffer, totalSamples, channelCount);
            break;
        case SQUARE_WAVE:
            generateSquareWave(frequency, durationMs, buffer, totalSamples);
            break;
        case SAWTOOTH:
            generateSawtoothWave(frequency, durationMs, buffer, totalSamples, channelCount);
            break;
        case TRIANGLE:
            generateTriangleWave(frequency, durationMs, buffer, totalSamples, channelCount);
            break;
        default:
            generateSineWave(frequency, durationMs, buffer, totalSamples, channelCount);
            break;
    }
}

void Note::generateGuitarWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || frequency == 0) return;
    
    size_t samplesPerChannel = totalSamples / channelCount;
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    double phase = 0.0;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        // Guitar: Plucked string with exponential decay
        double time = (double)i / SAMPLE_RATE;
        double decay = exp(-time * 3.0); // Exponential decay
        
        // Add slight harmonics for string character
        double fundamental = sin(phase);
        double harmonic2 = 0.3 * sin(phase * 2.0);
        double harmonic3 = 0.1 * sin(phase * 3.0);
        
        int16_t sample = (int16_t)(_amplitude * decay * (fundamental + harmonic2 + harmonic3));
        phase += phaseIncrement;
        
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        
        for (size_t ch = 0; ch < channelCount; ch++) {
            buffer[i * channelCount + ch] = sample;
        }
    }
}

void Note::generateOrganWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || frequency == 0) return;
    
    size_t samplesPerChannel = totalSamples / channelCount;
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    double phase = 0.0;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        // Organ: Rich harmonics, no decay
        double fundamental = sin(phase);
        double harmonic2 = 0.5 * sin(phase * 2.0);
        double harmonic3 = 0.25 * sin(phase * 3.0);
        double harmonic4 = 0.125 * sin(phase * 4.0);
        
        int16_t sample = (int16_t)(_amplitude * 0.6 * (fundamental + harmonic2 + harmonic3 + harmonic4));
        phase += phaseIncrement;
        
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        
        for (size_t ch = 0; ch < channelCount; ch++) {
            buffer[i * channelCount + ch] = sample;
        }
    }
}

void Note::generateFluteWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || frequency == 0) return;
    
    size_t samplesPerChannel = totalSamples / channelCount;
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    double phase = 0.0;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        // Flute: Pure tone with slight vibrato
        double time = (double)i / SAMPLE_RATE;
        double vibrato = 1.0 + 0.02 * sin(2.0 * M_PI * 5.0 * time); // 5Hz vibrato
        
        int16_t sample = (int16_t)(_amplitude * 0.8 * vibrato * sin(phase));
        phase += phaseIncrement;
        
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        
        for (size_t ch = 0; ch < channelCount; ch++) {
            buffer[i * channelCount + ch] = sample;
        }
    }
}

void Note::generateBellWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || frequency == 0) return;
    
    size_t samplesPerChannel = totalSamples / channelCount;
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    double phase = 0.0;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        // Bell: Metallic harmonics with slow decay
        double time = (double)i / SAMPLE_RATE;
        double decay = exp(-time * 1.5); // Slow decay
        
        // Metallic harmonics (not exact integer multiples)
        double fundamental = sin(phase);
        double harmonic2 = 0.4 * sin(phase * 2.76);
        double harmonic3 = 0.2 * sin(phase * 5.40);
        double harmonic4 = 0.1 * sin(phase * 8.93);
        
        int16_t sample = (int16_t)(_amplitude * decay * 0.7 * (fundamental + harmonic2 + harmonic3 + harmonic4));
        phase += phaseIncrement;
        
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        
        for (size_t ch = 0; ch < channelCount; ch++) {
            buffer[i * channelCount + ch] = sample;
        }
    }
}

void Note::generateSawtoothWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || frequency == 0) return;
    
    size_t samplesPerChannel = totalSamples / channelCount;
    uint32_t samplesPerCycle = SAMPLE_RATE / frequency;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        // Sawtooth: Linear ramp from -1 to 1
        uint32_t cyclePos = i % samplesPerCycle;
        double sampleValue = 2.0 * ((double)cyclePos / samplesPerCycle) - 1.0;
        
        int16_t sample = (int16_t)(_amplitude * sampleValue);
        
        for (size_t ch = 0; ch < channelCount; ch++) {
            buffer[i * channelCount + ch] = sample;
        }
    }
}

void Note::generateTriangleWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || frequency == 0) return;
    
    size_t samplesPerChannel = totalSamples / channelCount;
    uint32_t samplesPerCycle = SAMPLE_RATE / frequency;
    uint32_t halfCycle = samplesPerCycle / 2;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        // Triangle: Linear ramp up then down
        uint32_t cyclePos = i % samplesPerCycle;
        double sampleValue;
        
        if (cyclePos < halfCycle) {
            sampleValue = 2.0 * ((double)cyclePos / halfCycle) - 1.0;
        } else {
            sampleValue = 1.0 - 2.0 * ((double)(cyclePos - halfCycle) / halfCycle);
        }
        
        int16_t sample = (int16_t)(_amplitude * sampleValue);
        
        for (size_t ch = 0; ch < channelCount; ch++) {
            buffer[i * channelCount + ch] = sample;
        }
    }
}

void Note::generateSineWave(uint16_t frequency, uint32_t durationMs, int16_t* buffer, size_t totalSamples, size_t channelCount) {
    if (!buffer || frequency == 0) return;
    
    size_t samplesPerChannel = totalSamples / channelCount;
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    double phase = 0.0;
    
    for (size_t i = 0; i < samplesPerChannel; i++) {
        int16_t sample = (int16_t)(_amplitude * sin(phase));  // Use dynamic amplitude
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
        buffer[i] = (cyclePos < halfCycle) ? _amplitude : -_amplitude;  // Use dynamic amplitude
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
            
        case HAPPY_BIRTHDAY:
            *noteCount = sizeof(HAPPY_BIRTHDAY_NOTES) / sizeof(MusicNote);
            return HAPPY_BIRTHDAY_NOTES;
            
        default:
            *noteCount = 0;
            return nullptr;
    }
}

bool Note::isReady() const {
    return (_speaker != nullptr && _speaker->isInitialized());
}

void Note::stop() {
    _interrupt = true;
}

void Note::interrupt() {
    _interrupt = true;
}

void Note::setVolume(uint8_t volumePercent) {
    // Clamp volume to 0-100%
    if (volumePercent > 100) volumePercent = 100;
    
    // Convert percentage to amplitude (0-32767)
    _amplitude = (uint16_t)((volumePercent * 32767UL) / 100);
    
    if (_logger) {
        _logger->debug("Volume set to %d%% (amplitude: %d)", volumePercent, _amplitude);
    }
}

void Note::setVolumeRaw(uint16_t amplitude) {
    // Clamp amplitude to valid range
    if (amplitude > 32767) amplitude = 32767;
    
    _amplitude = amplitude;
    
    if (_logger) {
        uint8_t percent = (uint8_t)((_amplitude * 100UL) / 32767);
        _logger->debug("Volume set to raw amplitude %d (%d%%)", _amplitude, percent);
    }
}

uint8_t Note::getVolume() const {
    return (uint8_t)((_amplitude * 100UL) / 32767);
}

uint16_t Note::getVolumeRaw() const {
    return _amplitude;
}

void Note::setSoundType(SoundType soundType) {
    _soundType = soundType;
    
    if (_logger) {
        _logger->debug("Sound type set to: %s", getSoundTypeName());
    }
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