#include "I2SSpeaker.h"
#include "MP3Decoder.h"
#include "lib/Utils/FileManager.h"
#include <math.h>

namespace Audio {

I2SSpeaker::I2SSpeaker(int bclkPin, int wclkPin, int dataPin, i2s_port_t i2sPort) 
    : _bclkPin(bclkPin), _wclkPin(wclkPin), _dataPin(dataPin), _i2sPort(i2sPort),
      _initialized(false), _defaultVolume(50), _sampleRate(16000), _bitsPerSample(16), _playing(false) {
}

I2SSpeaker::~I2SSpeaker() {
    if (_initialized) {
        stop();
        i2s_driver_uninstall(_i2sPort);
    }
}

bool I2SSpeaker::init(uint32_t sampleRate, int bitsPerSample) {
    if (_initialized) {
        return true; // Already initialized
    }

    _sampleRate = sampleRate;
    _bitsPerSample = bitsPerSample;

    if (!configureI2S()) {
        return false;
    }

    _initialized = true;
    return true;
}

bool I2SSpeaker::configureI2S() {
    // I2S configuration
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = _sampleRate,
        .bits_per_sample = (i2s_bits_per_sample_t)_bitsPerSample,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    // Install I2S driver
    esp_err_t err = i2s_driver_install(_i2sPort, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        return false;
    }

    // I2S pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = _bclkPin,
        .ws_io_num = _wclkPin,
        .data_out_num = _dataPin,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    err = i2s_set_pin(_i2sPort, &pin_config);
    if (err != ESP_OK) {
        i2s_driver_uninstall(_i2sPort);
        return false;
    }

    return true;
}

void I2SSpeaker::playTone(int frequency, int duration, int volume) {
    if (!_initialized) {
        return;
    }

    // Constrain frequency to audible range
    frequency = constrain(frequency, 20, 20000);
    volume = constrain(volume, 0, 100);

    // Calculate number of samples needed
    size_t samplesNeeded = (_sampleRate * duration) / 1000;
    
    // Allocate buffer for samples
    int16_t* sampleBuffer = new int16_t[samplesNeeded * 2]; // Stereo
    if (!sampleBuffer) {
        return;
    }

    // Generate sine wave
    size_t samplesGenerated = generateSineWave(frequency, duration, 0.5f, sampleBuffer, samplesNeeded * 2);

    // Play the samples
    if (samplesGenerated > 0) {
        _playing = true;
        writeSamples(sampleBuffer, samplesGenerated, volume);
        _playing = false;
    }

    delete[] sampleBuffer;
}

size_t I2SSpeaker::generateSineWave(int frequency, int duration, float amplitude, int16_t* sampleBuffer, size_t bufferSize) {
    if (!sampleBuffer || bufferSize == 0) {
        return 0;
    }

    size_t samplesNeeded = (_sampleRate * duration) / 1000;
    size_t actualSamples = _min(samplesNeeded, bufferSize / 2); // Stereo

    float angularFreq = 2.0f * PI * frequency / _sampleRate;

    for (size_t i = 0; i < actualSamples; i++) {
        float sineValue = sin(angularFreq * i);
        int16_t sample = (int16_t)(sineValue * amplitude * 32767);
        
        // Stereo output (same sample for both channels)
        sampleBuffer[i * 2] = sample;     // Left channel
        sampleBuffer[i * 2 + 1] = sample; // Right channel
    }

    return actualSamples * 2; // Return total samples (stereo)
}

void I2SSpeaker::writeSamples(const int16_t* samples, size_t sampleCount, int volume) {
    if (!_initialized || !samples || sampleCount == 0) {
        return;
    }

    // Apply volume scaling
    int16_t* volumeAdjustedSamples = new int16_t[sampleCount];
    if (!volumeAdjustedSamples) {
        return;
    }

    memcpy(volumeAdjustedSamples, samples, sampleCount * sizeof(int16_t));
    applyVolume(volumeAdjustedSamples, sampleCount, volume);

    // Write to I2S
    size_t bytesWritten;
    i2s_write(_i2sPort, volumeAdjustedSamples, sampleCount * sizeof(int16_t), &bytesWritten, portMAX_DELAY);

    delete[] volumeAdjustedSamples;
}

void I2SSpeaker::applyVolume(int16_t* samples, size_t sampleCount, int volume) {
    if (!samples || sampleCount == 0) {
        return;
    }

    float volumeScale = volume / 100.0f;
    
    for (size_t i = 0; i < sampleCount; i++) {
        samples[i] = (int16_t)(samples[i] * volumeScale);
    }
}

void I2SSpeaker::playAudioData(const uint8_t* data, size_t dataSize, int volume) {
    if (!_initialized || !data || dataSize == 0) {
        return;
    }
    
    // Ensure data is 16-bit aligned and size is a multiple of 2
    if (dataSize % sizeof(int16_t) != 0) {
        return;
    }

    _playing = true;

    const int16_t* samples = (const int16_t*)data;
    size_t sampleCount = dataSize / sizeof(int16_t);
    
    writeSamples(samples, sampleCount, volume);
    
    _playing = false;
}

void I2SSpeaker::beep(int volume) {
    playTone(1000, 200, volume);
}

void I2SSpeaker::doubleBeep(int volume) {
    playTone(1000, 150, volume);
    delay(100);
    playTone(1000, 150, volume);
}

void I2SSpeaker::playConfirmation(int volume) {
    playTone(800, 150, volume);
    delay(50);
    playTone(1200, 200, volume);
}

void I2SSpeaker::playError(int volume) {
    playTone(400, 300, volume);
    delay(100);
    playTone(300, 300, volume);
}

void I2SSpeaker::playStartup(int volume) {
    // Play startup melody: C, E, G, C (octave higher)
    playTone(523, 200, volume);  // C5
    delay(50);
    playTone(659, 200, volume);  // E5
    delay(50);
    playTone(784, 200, volume);  // G5
    delay(50);
    playTone(1047, 400, volume); // C6
}

void I2SSpeaker::playNotification(int volume) {
    playTone(1000, 100, volume);
    delay(50);
    playTone(1500, 100, volume);
    delay(50);
    playTone(1000, 100, volume);
}

void I2SSpeaker::stop() {
    if (_initialized) {
        i2s_stop(_i2sPort);
        i2s_start(_i2sPort);
        _playing = false;
    }
}

void I2SSpeaker::setVolume(int volume) {
    _defaultVolume = constrain(volume, 0, 100);
}

int I2SSpeaker::getVolume() const {
    return _defaultVolume;
}

bool I2SSpeaker::isPlaying() {
    return _playing;
}

bool I2SSpeaker::isInitialized() const {
    return _initialized;
}

bool I2SSpeaker::setSampleRate(uint32_t sampleRate) {
    if (!_initialized) {
        return false;
    }

    _sampleRate = sampleRate;
    
    // Reconfigure I2S with new sample rate
    i2s_stop(_i2sPort);
    
    esp_err_t err = i2s_set_sample_rates(_i2sPort, sampleRate);
    if (err == ESP_OK) {
        i2s_start(_i2sPort);
        return true;
    }
    
    return false;
}

bool I2SSpeaker::playAudioFile(const String& filePath, int volume) {
    // We need access to FileManager - this will be handled by the speaker.cpp wrapper
    // For now, return false to indicate this should be called through the main speaker interface
    return false;
}

bool I2SSpeaker::playMP3File(const String& filePath, int volume) {
    if (!_initialized) {
        return false;
    }
    
    // Create MP3 decoder
    MP3Decoder decoder;
    if (!decoder.init()) {
        return false;
    }
    
    // Decode MP3 file to PCM data
    int16_t* pcmBuffer = nullptr;
    size_t pcmSize = 0;
    MP3Decoder::MP3Info info;
    
    if (!decoder.decodeFile(filePath, &pcmBuffer, &pcmSize, &info)) {
        return false;
    }
    
    // Check if sample rate matches our I2S configuration
    if (info.sampleRate != _sampleRate) {
        // setSampleRate(info.sampleRate);
    }
    
    // Play the decoded PCM data
    // If PCM data is larger than 100KB, play in chunks (loop)
    const size_t CHUNK_SIZE = 100 * 1024 / sizeof(int16_t); // 100KB worth of int16_t samples
    size_t totalSamples = pcmSize;
    size_t offset = 0;

    while (offset < totalSamples) {
        size_t samplesToPlay = std::min(CHUNK_SIZE, totalSamples - offset);
        playAudioData((uint8_t*)(pcmBuffer + offset), samplesToPlay * sizeof(int16_t), volume);
        offset += samplesToPlay;
    }
    
    // Clean up
    decoder.freePCMBuffer(pcmBuffer);
    
    return true;
}

bool I2SSpeaker::playMP3FileStreaming(const String& filePath, int volume, Utils::FileManager& fileManager) {
    if (!_initialized) {
        Serial.println("I2S not initialized");
        return false;
    }
    
    // Create MP3 decoder
    MP3Decoder decoder;
    if (!decoder.init()) {
        Serial.println("Failed to initialize MP3 decoder");
        return false;
    }
    
    // Open file for streaming
    File audioFile = fileManager.openFileForReading(filePath);
    if (!audioFile) {
        Serial.printf("Failed to open MP3 file: %s\n", filePath.c_str());
        return false;
    }
    
    size_t fileSize = audioFile.size();
    Serial.printf("Starting streaming MP3 playback: %s (%d bytes)\n", filePath.c_str(), fileSize);
    
    // Get MP3 info from file header
    MP3Decoder::MP3Info info;
    if (!decoder.getFileInfo(filePath, &info)) {
        Serial.println("Failed to get MP3 file info");
        fileManager.closeFile(audioFile);
        return false;
    }
    
    Serial.printf("MP3 Stream Info: %dHz, %dch, %dkbps\n", 
                  info.sampleRate, info.channels, info.bitRate);
    
    // Streaming buffer sizes
    const size_t STREAM_BUFFER_SIZE = 4096;  // 4KB chunks from file
    const size_t PCM_BUFFER_SIZE = 8192;     // 8KB PCM output buffer
    
    uint8_t* streamBuffer = (uint8_t*)heap_caps_malloc(STREAM_BUFFER_SIZE, MALLOC_CAP_DEFAULT);
    int16_t* pcmBuffer = (int16_t*)heap_caps_malloc(PCM_BUFFER_SIZE, MALLOC_CAP_DEFAULT);
    
    if (!streamBuffer || !pcmBuffer) {
        Serial.println("Failed to allocate streaming buffers");
        if (streamBuffer) heap_caps_free(streamBuffer);
        if (pcmBuffer) heap_caps_free(pcmBuffer);
        fileManager.closeFile(audioFile);
        return false;
    }
    
    _playing = true;
    size_t totalBytesRead = 0;
    size_t totalPCMSamples = 0;
    
    // Streaming playback loop
    while (audioFile.available() > 0 && _playing) {
        // Read chunk from file
        size_t bytesRead = fileManager.readStream(audioFile, streamBuffer, STREAM_BUFFER_SIZE);
        if (bytesRead == 0) {
            break;
        }
        
        totalBytesRead += bytesRead;
        
        // Decode MP3 chunk to PCM
        int16_t* decodedPCM = nullptr;
        size_t pcmSamples = 0;
        
        if (decoder.decodeData(streamBuffer, bytesRead, &decodedPCM, &pcmSamples)) {
            totalPCMSamples += pcmSamples;
            
            // Play decoded PCM data in smaller chunks to avoid blocking
            const size_t PLAY_CHUNK_SAMPLES = 1024;  // Play 1024 samples at a time
            size_t samplesPlayed = 0;
            
            while (samplesPlayed < pcmSamples && _playing) {
                size_t samplesToPlay = _min(PLAY_CHUNK_SAMPLES, pcmSamples - samplesPlayed);
                
                // Copy to play buffer and apply volume
                memcpy(pcmBuffer, decodedPCM + samplesPlayed, samplesToPlay * sizeof(int16_t));
                applyVolume(pcmBuffer, samplesToPlay, volume);
                
                // Write to I2S
                size_t bytesWritten;
                esp_err_t result = i2s_write(_i2sPort, pcmBuffer, samplesToPlay * sizeof(int16_t), 
                                           &bytesWritten, pdMS_TO_TICKS(100));
                
                if (result != ESP_OK) {
                    Serial.printf("I2S write error: %d\n", result);
                    break;
                }
                
                samplesPlayed += samplesToPlay;
                
                // Small delay to prevent watchdog timeout
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            
            // Free decoded PCM buffer
            decoder.freePCMBuffer(decodedPCM);
        }
        
        // Progress indication every 64KB
        if (totalBytesRead % (64 * 1024) == 0) {
            Serial.printf("Streaming progress: %d/%d bytes (%.1f%%)\n", 
                         totalBytesRead, fileSize, 
                         (float)totalBytesRead / fileSize * 100.0f);
        }
        
        // Yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    _playing = false;
    
    // Cleanup
    heap_caps_free(streamBuffer);
    heap_caps_free(pcmBuffer);
    fileManager.closeFile(audioFile);
    
    Serial.printf("Streaming playback completed: %d bytes read, %d PCM samples\n", 
                 totalBytesRead, totalPCMSamples);
    
    return true;
}

/**
 * Play MP3 file using the new optimized streaming decoder
 * This method uses the frame-by-frame decoding approach for more efficient memory usage
 */
bool I2SSpeaker::playMP3FileStreamingOptimized(const String& filePath, int volume) {
    if (!_initialized) {
        Serial.println("I2S not initialized");
        return false;
    }
    
    // Create MP3 decoder
    MP3Decoder decoder;
    if (!decoder.init()) {
        Serial.println("Failed to initialize MP3 decoder");
        return false;
    }
    
    // Allocate buffer for PCM data
    const size_t PCM_BUFFER_SIZE = 4096;
    int16_t* pcmBuffer = (int16_t*)heap_caps_malloc(PCM_BUFFER_SIZE * sizeof(int16_t), MALLOC_CAP_DEFAULT);
    if (!pcmBuffer) {
        Serial.println("Failed to allocate PCM buffer");
        return false;
    }
    
    _playing = true;
    size_t totalFrames = 0;
    
    Serial.printf("Starting optimized streaming playback: %s\n", filePath.c_str());
    
    // Start streaming with callback
    bool success = decoder.startStreaming(filePath, 
        [this, pcmBuffer, volume, &totalFrames](const int16_t* data, size_t len, MP3Decoder::MP3Info& info) {
            // First frame with info
            if (totalFrames == 0) {
                Serial.printf("MP3 Stream Info: %dHz, %d channels, %dkbps\n", 
                              info.sampleRate, info.channels, info.bitRate);
            }
            
            totalFrames++;
            
            // Process data in smaller chunks to avoid blocking
            const size_t PLAY_CHUNK_SIZE = 1024;
            size_t samplesPlayed = 0;
            
            while (samplesPlayed < len && _playing) {
                size_t samplesToPlay = _min(PLAY_CHUNK_SIZE, len - samplesPlayed);
                
                // Copy to play buffer and apply volume
                memcpy(pcmBuffer, data + samplesPlayed, samplesToPlay * sizeof(int16_t));
                applyVolume(pcmBuffer, samplesToPlay, volume);
                
                // Write to I2S
                size_t bytesWritten;
                esp_err_t result = i2s_write(_i2sPort, pcmBuffer, samplesToPlay * sizeof(int16_t), 
                                           &bytesWritten, pdMS_TO_TICKS(100));
                
                if (result != ESP_OK) {
                    Serial.printf("I2S write error: %d\n", result);
                    return false;
                }
                
                samplesPlayed += samplesToPlay;
                
                // Small delay to prevent watchdog timeout
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            
            // Progress indication every 100 frames
            if (totalFrames % 100 == 0) {
                Serial.printf("Streaming progress: %d frames processed\n", totalFrames);
            }
            
            return _playing; // Continue streaming if still playing
        });
    
    if (!success) {
        Serial.println("Failed to start streaming");
        heap_caps_free(pcmBuffer);
        return false;
    }
    
    // Process frames until streaming is complete
    while (decoder.isStreaming() && _playing) {
        if (!decoder.processStreamFrame()) {
            break;
        }
        
        // Yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    _playing = false;
    
    // Stop streaming and cleanup
    decoder.stopStreaming();
    heap_caps_free(pcmBuffer);
    
    Serial.printf("Optimized streaming playback completed: %d frames processed\n", totalFrames);
    
    return true;
}

// WAV file header structure
struct WAVHeader {
    char riff[4];           // "RIFF"
    uint32_t fileSize;      // File size - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmtSize;       // Format chunk size
    uint16_t audioFormat;   // Audio format (1 = PCM)
    uint16_t channels;      // Number of channels
    uint32_t sampleRate;    // Sample rate
    uint32_t byteRate;      // Byte rate
    uint16_t blockAlign;    // Block align
    uint16_t bitsPerSample; // Bits per sample
    char data[4];           // "data"
    uint32_t dataSize;      // Data size
};

bool I2SSpeaker::playWAVFile(const String& filePath, int volume) {
    if (!_initialized) {
        Serial.println("I2S not initialized");
        return false;
    }
    
    // Open WAV file
    File wavFile = SPIFFS.open(filePath, "r");
    if (!wavFile) {
        Serial.printf("Failed to open WAV file: %s\n", filePath.c_str());
        return false;
    }
    
    // Read WAV header
    WAVHeader header;
    if (wavFile.readBytes((char*)&header, sizeof(WAVHeader)) != sizeof(WAVHeader)) {
        Serial.println("Failed to read WAV header");
        wavFile.close();
        return false;
    }
    
    // Validate WAV file
    if (strncmp(header.riff, "RIFF", 4) != 0 || strncmp(header.wave, "WAVE", 4) != 0) {
        Serial.println("Invalid WAV file format");
        wavFile.close();
        return false;
    }
    
    // Check if it's PCM format
    if (header.audioFormat != 1) {
        Serial.printf("Unsupported audio format: %d (only PCM supported)\n", header.audioFormat);
        wavFile.close();
        return false;
    }
    
    // Check bit depth
    if (header.bitsPerSample != 16 && header.bitsPerSample != 8) {
        Serial.printf("Unsupported bit depth: %d (only 8/16-bit supported)\n", header.bitsPerSample);
        wavFile.close();
        return false;
    }
    
    Serial.printf("WAV Info: %dHz, %dch, %d-bit, %d bytes\n", 
                  header.sampleRate, header.channels, header.bitsPerSample, header.dataSize);
    
    // Skip to data chunk (in case there are additional chunks)
    wavFile.seek(sizeof(WAVHeader));
    
    // Allocate buffer for audio data
    const size_t CHUNK_SIZE = 4096;
    uint8_t* audioBuffer = (uint8_t*)heap_caps_malloc(CHUNK_SIZE, MALLOC_CAP_DEFAULT);
    if (!audioBuffer) {
        Serial.println("Failed to allocate audio buffer");
        wavFile.close();
        return false;
    }
    
    _playing = true;
    size_t totalBytesRead = 0;
    
    // Play audio data in chunks
    while (wavFile.available() > 0 && totalBytesRead < header.dataSize && _playing) {
        size_t bytesToRead = _min(CHUNK_SIZE, header.dataSize - totalBytesRead);
        size_t bytesRead = wavFile.readBytes((char*)audioBuffer, bytesToRead);
        
        if (bytesRead == 0) {
            break;
        }
        
        totalBytesRead += bytesRead;
        
        // Convert 8-bit to 16-bit if necessary
        if (header.bitsPerSample == 8) {
            // Convert 8-bit unsigned to 16-bit signed
            int16_t* convertedBuffer = (int16_t*)heap_caps_malloc(bytesRead * 2, MALLOC_CAP_DEFAULT);
            if (convertedBuffer) {
                for (size_t i = 0; i < bytesRead; i++) {
                    convertedBuffer[i] = ((int16_t)audioBuffer[i] - 128) * 256;
                }
                playAudioData((uint8_t*)convertedBuffer, bytesRead * 2, volume);
                heap_caps_free(convertedBuffer);
            }
        } else {
            // 16-bit data, play directly
            playAudioData(audioBuffer, bytesRead, volume);
        }
        
        // Progress indication
        if (totalBytesRead % (32 * 1024) == 0) {
            Serial.printf("WAV progress: %d/%d bytes (%.1f%%)\n", 
                         totalBytesRead, header.dataSize, 
                         (float)totalBytesRead / header.dataSize * 100.0f);
        }
        
        // Small delay to prevent watchdog timeout
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    _playing = false;
    
    // Cleanup
    heap_caps_free(audioBuffer);
    wavFile.close();
    
    Serial.printf("WAV playback completed: %d bytes played\n", totalBytesRead);
    return true;
}

bool I2SSpeaker::playWAVFileStreaming(const String& filePath, int volume, Utils::FileManager& fileManager) {
    if (!_initialized) {
        Serial.println("I2S not initialized");
        return false;
    }
    
    // Open WAV file for streaming
    File wavFile = fileManager.openFileForReading(filePath);
    if (!wavFile) {
        Serial.printf("Failed to open WAV file: %s\n", filePath.c_str());
        return false;
    }
    
    // Read WAV header
    WAVHeader header;
    if (fileManager.readStream(wavFile, (uint8_t*)&header, sizeof(WAVHeader)) != sizeof(WAVHeader)) {
        Serial.println("Failed to read WAV header");
        fileManager.closeFile(wavFile);
        return false;
    }
    
    // Validate WAV file
    if (strncmp(header.riff, "RIFF", 4) != 0 || strncmp(header.wave, "WAVE", 4) != 0) {
        Serial.println("Invalid WAV file format");
        fileManager.closeFile(wavFile);
        return false;
    }
    
    // Check if it's PCM format
    if (header.audioFormat != 1) {
        Serial.printf("Unsupported audio format: %d (only PCM supported)\n", header.audioFormat);
        fileManager.closeFile(wavFile);
        return false;
    }
    
    // Check bit depth
    if (header.bitsPerSample != 16 && header.bitsPerSample != 8) {
        Serial.printf("Unsupported bit depth: %d (only 8/16-bit supported)\n", header.bitsPerSample);
        fileManager.closeFile(wavFile);
        return false;
    }
    
    Serial.printf("WAV Stream Info: %dHz, %dch, %d-bit, %d bytes\n", 
                  header.sampleRate, header.channels, header.bitsPerSample, header.dataSize);
    
    // Streaming buffer
    const size_t STREAM_CHUNK_SIZE = 4096;
    uint8_t* streamBuffer = (uint8_t*)heap_caps_malloc(STREAM_CHUNK_SIZE, MALLOC_CAP_DEFAULT);
    int16_t* convertBuffer = nullptr;
    
    if (!streamBuffer) {
        Serial.println("Failed to allocate stream buffer");
        fileManager.closeFile(wavFile);
        return false;
    }
    
    // Allocate conversion buffer for 8-bit audio
    if (header.bitsPerSample == 8) {
        convertBuffer = (int16_t*)heap_caps_malloc(STREAM_CHUNK_SIZE * 2, MALLOC_CAP_DEFAULT);
        if (!convertBuffer) {
            Serial.println("Failed to allocate conversion buffer");
            heap_caps_free(streamBuffer);
            fileManager.closeFile(wavFile);
            return false;
        }
    }
    
    _playing = true;
    size_t totalBytesRead = 0;
    
    // Streaming playback loop
    while (wavFile.available() > 0 && totalBytesRead < header.dataSize && _playing) {
        size_t bytesToRead = _min(STREAM_CHUNK_SIZE, header.dataSize - totalBytesRead);
        size_t bytesRead = fileManager.readStream(wavFile, streamBuffer, bytesToRead);
        
        if (bytesRead == 0) {
            break;
        }
        
        totalBytesRead += bytesRead;
        
        // Handle different bit depths
        if (header.bitsPerSample == 8) {
            // Convert 8-bit unsigned to 16-bit signed
            for (size_t i = 0; i < bytesRead; i++) {
                convertBuffer[i] = ((int16_t)streamBuffer[i] - 128) * 256;
            }
            
            // Apply volume and play
            applyVolume(convertBuffer, bytesRead, volume);
            
            // Write to I2S
            size_t bytesWritten;
            esp_err_t result = i2s_write(_i2sPort, convertBuffer, bytesRead * 2, 
                                       &bytesWritten, pdMS_TO_TICKS(100));
            
            if (result != ESP_OK) {
                Serial.printf("I2S write error: %d\n", result);
                break;
            }
        } else {
            // 16-bit data
            int16_t* samples = (int16_t*)streamBuffer;
            size_t sampleCount = bytesRead / 2;
            
            // Apply volume
            applyVolume(samples, sampleCount, volume);
            
            // Write to I2S
            size_t bytesWritten;
            esp_err_t result = i2s_write(_i2sPort, samples, bytesRead, 
                                       &bytesWritten, pdMS_TO_TICKS(100));
            
            if (result != ESP_OK) {
                Serial.printf("I2S write error: %d\n", result);
                break;
            }
        }
        
        // Progress indication every 64KB
        if (totalBytesRead % (64 * 1024) == 0) {
            Serial.printf("WAV streaming progress: %d/%d bytes (%.1f%%)\n", 
                         totalBytesRead, header.dataSize, 
                         (float)totalBytesRead / header.dataSize * 100.0f);
        }
        
        // Yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    _playing = false;
    
    // Cleanup
    heap_caps_free(streamBuffer);
    if (convertBuffer) {
        heap_caps_free(convertBuffer);
    }
    fileManager.closeFile(wavFile);
    
    Serial.printf("WAV streaming playback completed: %d bytes played\n", totalBytesRead);
    return true;
}

// Example usage of audio playback options:
void exampleAudioUsage() {
    Utils::FileManager fileManager;
    Audio::I2SSpeaker speaker(26, 27, 25); // BCLK, WCLK, DATA pins
    
    // Initialize
    fileManager.init();
    speaker.init(16000, 16); // 16kHz, 16-bit
    
    // 1. Play WAV file (fastest, no decoding needed)
    speaker.playWAVFile("/audio/beep.wav", 75);
    
    // 2. Stream large WAV file (memory efficient)
    speaker.playWAVFileStreaming("/audio/large_sound.wav", 60, fileManager);
    
    // 3. Play MP3 file (requires decoding, smaller file size)
    speaker.playMP3File("/audio/music.mp3", 50);
    
    // 4. Stream large MP3 file (memory efficient)
    speaker.playMP3FileStreaming("/audio/large_music.mp3", 50, fileManager);
    
    // 5. Manual file streaming
    File audioFile = fileManager.openFileForReading("/audio/data.pcm");
    if (audioFile) {
        const size_t CHUNK_SIZE = 4096;
        uint8_t buffer[CHUNK_SIZE];
        
        while (audioFile.available()) {
            size_t bytesRead = fileManager.readStream(audioFile, buffer, CHUNK_SIZE);
            if (bytesRead > 0) {
                // Process raw PCM data
                speaker.playAudioData(buffer, bytesRead, 50);
            }
        }
        fileManager.closeFile(audioFile);
    }
    
    // 6. Read specific byte range from file
    uint8_t rangeBuffer[1024];
    size_t bytesRead = fileManager.readStream("/audio/sound.wav", 44, 1068, rangeBuffer); // Skip WAV header
    speaker.playAudioData(rangeBuffer, bytesRead, 50);
}

} // namespace Audio
