#pragma once

#include <Arduino.h>
#include <SPIFFS.h>

// Include the ESP32 Helix MP3 decoder library
extern "C" {
    #include "mp3dec.h"
}

namespace Audio {

/**
 * MP3Decoder class for decoding MP3 files
 * 
 * Uses the ESP32 Helix MP3 decoder library to decode MP3 files
 * and convert them to PCM data for playback through speakers.
 */
class MP3Decoder {
public:
    struct MP3Info {
        int sampleRate;
        int channels;
        int bitRate;
        int duration;      // in seconds
        bool valid;
    };

    MP3Decoder();
    ~MP3Decoder();

    /**
     * Initialize the MP3 decoder
     * @return true if initialization was successful
     */
    bool init();

    /**
     * Decode MP3 file from SPIFFS
     * @param filePath Path to MP3 file in SPIFFS
     * @param pcmBuffer Output buffer for PCM data (will be allocated)
     * @param pcmSize Output size of PCM data
     * @param info Output MP3 file information
     * @return true if decoding was successful
     */
    bool decodeFile(const String& filePath, int16_t** pcmBuffer, size_t* pcmSize, MP3Info* info = nullptr);

    /**
     * Decode MP3 data from memory
     * @param mp3Data Input MP3 data
     * @param mp3Size Size of MP3 data
     * @param pcmBuffer Output buffer for PCM data (will be allocated)
     * @param pcmSize Output size of PCM data
     * @param info Output MP3 file information
     * @return true if decoding was successful
     */
    bool decodeData(const uint8_t* mp3Data, size_t mp3Size, int16_t** pcmBuffer, size_t* pcmSize, MP3Info* info = nullptr);

    /**
     * Get MP3 file information without full decoding
     * @param filePath Path to MP3 file
     * @param info Output MP3 information
     * @return true if information was retrieved successfully
     */
    bool getFileInfo(const String& filePath, MP3Info* info);

    /**
     * Free PCM buffer allocated by decode functions
     * @param pcmBuffer Buffer to free
     */
    void freePCMBuffer(int16_t* pcmBuffer);

    /**
     * Check if decoder is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return _initialized; }

private:
    HMP3Decoder _decoder;
    bool _initialized;
    
    static const size_t INPUT_BUFFER_SIZE = 2048;
    static const size_t OUTPUT_BUFFER_SIZE = 4608; // Max PCM samples per frame
    
    uint8_t* _inputBuffer;
    int16_t* _outputBuffer;
    
    bool decodeInternal(const uint8_t* mp3Data, size_t mp3Size, int16_t** pcmBuffer, size_t* pcmSize, MP3Info* info);
};

} // namespace Audio
