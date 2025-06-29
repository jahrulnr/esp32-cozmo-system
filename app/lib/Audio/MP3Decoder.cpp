#include "MP3Decoder.h"
#include "lib/Utils/Logger.h"
#include <esp_heap_caps.h>

namespace Audio {

MP3Decoder::MP3Decoder() 
    : _decoder(nullptr), _initialized(false), _inputBuffer(nullptr), _outputBuffer(nullptr) {
}

MP3Decoder::~MP3Decoder() {
    if (_decoder) {
        MP3FreeDecoder(_decoder);
    }
    
    if (_inputBuffer) {
        heap_caps_free(_inputBuffer);
    }
    
    if (_outputBuffer) {
        heap_caps_free(_outputBuffer);
    }
}

bool MP3Decoder::init() {
    if (_initialized) {
        return true;
    }
    
    // Create MP3 decoder instance
    _decoder = MP3InitDecoder();
    if (!_decoder) {
        return false;
    }
    
    // Allocate input and output buffers
    _inputBuffer = (uint8_t*)heap_caps_malloc(INPUT_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
    _outputBuffer = (int16_t*)heap_caps_malloc(OUTPUT_BUFFER_SIZE * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
    
    if (!_inputBuffer || !_outputBuffer) {
        if (_inputBuffer) heap_caps_free(_inputBuffer);
        if (_outputBuffer) heap_caps_free(_outputBuffer);
        if (_decoder) MP3FreeDecoder(_decoder);
        _inputBuffer = nullptr;
        _outputBuffer = nullptr;
        _decoder = nullptr;
        return false;
    }
    
    _initialized = true;
    return true;
}

bool MP3Decoder::decodeFile(const String& filePath, int16_t** pcmBuffer, size_t* pcmSize, MP3Info* info) {
    if (!_initialized) {
        return false;
    }
    
    // Open file from SPIFFS
    File file = SPIFFS.open(filePath, "r");
    if (!file) {
        return false;
    }
    
    size_t fileSize = file.size();
    if (fileSize == 0) {
        file.close();
        return false;
    }
    
    // Allocate buffer for MP3 data
    uint8_t* mp3Data = (uint8_t*)heap_caps_malloc(fileSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
    if (!mp3Data) {
        file.close();
        return false;
    }
    
    // Read entire file into memory
    size_t bytesRead = file.readBytes((char*)mp3Data, fileSize);
    file.close();
    
    if (bytesRead != fileSize) {
        heap_caps_free(mp3Data);
        return false;
    }
    
    // Decode the MP3 data
    bool result = decodeInternal(mp3Data, fileSize, pcmBuffer, pcmSize, info);
    
    heap_caps_free(mp3Data);
    return result;
}

bool MP3Decoder::decodeData(const uint8_t* mp3Data, size_t mp3Size, int16_t** pcmBuffer, size_t* pcmSize, MP3Info* info) {
    if (!_initialized || !mp3Data || mp3Size == 0) {
        return false;
    }
    
    return decodeInternal(mp3Data, mp3Size, pcmBuffer, pcmSize, info);
}

bool MP3Decoder::decodeInternal(const uint8_t* mp3Data, size_t mp3Size, int16_t** pcmBuffer, size_t* pcmSize, MP3Info* info) {
    if (!pcmBuffer || !pcmSize) {
        return false;
    }
    
    *pcmBuffer = nullptr;
    *pcmSize = 0;
    
    // Estimate PCM buffer size (MP3 compression is typically 10:1)
    size_t estimatedPCMSize = mp3Size * 10;
    int16_t* pcmData = (int16_t*)heap_caps_malloc(estimatedPCMSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
    if (!pcmData) {
        return false;
    }
    
    const uint8_t* readPtr = mp3Data;
    size_t bytesLeft = mp3Size;
    size_t pcmOffset = 0;
    size_t totalPCMSamples = 0;
    
    MP3FrameInfo frameInfo;
    bool firstFrame = true;
    
    while (bytesLeft > 0) {
        // Find sync word
        int offset = MP3FindSyncWord((unsigned char*)readPtr, bytesLeft);
        if (offset < 0) {
            break; // No more sync words found
        }
        
        readPtr += offset;
        bytesLeft -= offset;
        
        // Get frame info
        int result = MP3GetNextFrameInfo(_decoder, &frameInfo, (unsigned char*)readPtr);
        if (result != 0) {
            break; // Error getting frame info
        }
        
        // Store info from first frame
        if (firstFrame && info) {
            info->sampleRate = frameInfo.samprate;
            info->channels = frameInfo.nChans;
            info->bitRate = frameInfo.bitrate;
            info->valid = true;
            firstFrame = false;
        }
        
        // Check if we need more space for PCM data
        size_t samplesNeeded = frameInfo.outputSamps;
        if ((totalPCMSamples + samplesNeeded) * sizeof(int16_t) > estimatedPCMSize) {
            estimatedPCMSize *= 2;
            int16_t* newBuffer = (int16_t*)heap_caps_realloc(pcmData, estimatedPCMSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
            if (!newBuffer) {
                heap_caps_free(pcmData);
                return false;
            }
            pcmData = newBuffer;
        }
        
        // Decode frame
        try {
            result = MP3Decode(_decoder, (unsigned char**)&readPtr, (int*)&bytesLeft, 
                            pcmData + totalPCMSamples, 0);
        } catch(...) {
            result = 0;
        }
        
        if (result != 0) {
            if (result == ERR_MP3_INDATA_UNDERFLOW) {
                break; // End of data
            } else {
                // Other error, but continue trying to decode
                readPtr++;
                if (bytesLeft > 0)
                    bytesLeft--;
                continue;
            }
        }
        
        totalPCMSamples += frameInfo.outputSamps;
    }
    
    if (totalPCMSamples == 0) {
        heap_caps_free(pcmData);
        return false;
    }
    
    // Resize buffer to actual size
    int16_t* finalBuffer = (int16_t*)heap_caps_realloc(pcmData, totalPCMSamples * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
    if (finalBuffer) {
        pcmData = finalBuffer;
    }
    
    *pcmBuffer = pcmData;
    *pcmSize = totalPCMSamples;
    
    return true;
}

bool MP3Decoder::getFileInfo(const String& filePath, MP3Info* info) {
    if (!_initialized || !info) {
        return false;
    }
    
    // Open file
    File file = SPIFFS.open(filePath, "r");
    if (!file) {
        return false;
    }
    
    size_t fileSize = file.size();
    if (fileSize == 0) {
        file.close();
        return false;
    }
    
    // Read first few KB to get frame info
    size_t readSize = min(fileSize, (size_t)4096);
    uint8_t* buffer = (uint8_t*)heap_caps_malloc(readSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
    if (!buffer) {
        file.close();
        return false;
    }
    
    size_t bytesRead = file.readBytes((char*)buffer, readSize);
    file.close();
    
    if (bytesRead == 0) {
        heap_caps_free(buffer);
        return false;
    }
    
    // Find first frame and get info
    const uint8_t* readPtr = buffer;
    size_t bytesLeft = bytesRead;
    
    int offset = MP3FindSyncWord((unsigned char*)readPtr, bytesLeft);
    if (offset < 0) {
        heap_caps_free(buffer);
        return false;
    }
    
    readPtr += offset;
    bytesLeft -= offset;
    
    MP3FrameInfo frameInfo;
    int result = MP3GetNextFrameInfo(_decoder, &frameInfo, (unsigned char*)readPtr);
    
    heap_caps_free(buffer);
    
    if (result != 0) {
        return false;
    }
    
    info->sampleRate = frameInfo.samprate;
    info->channels = frameInfo.nChans;
    info->bitRate = frameInfo.bitrate;
    info->valid = true;

    // Print MP3 info for debugging
    Serial.printf("MP3 Info: SampleRate=%d Hz, Channels=%d, BitRate=%d kbps\n", info->sampleRate, info->channels, info->bitRate);

    // Estimate duration (rough calculation)
    if (frameInfo.bitrate > 0) {
        info->duration = (fileSize * 8) / frameInfo.bitrate;
    } else {
        info->duration = 0;
    }
    
    return true;
}

void MP3Decoder::freePCMBuffer(int16_t* pcmBuffer) {
    if (pcmBuffer) {
        heap_caps_free(pcmBuffer);
    }
}

} // namespace Audio
