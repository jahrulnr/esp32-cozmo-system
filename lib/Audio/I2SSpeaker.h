#pragma once

#include <Arduino.h>
#include <driver/i2s_std.h>
#include <driver/i2s_pdm.h>
#include <driver/i2s_tdm.h>
#include <driver/i2s.h>

// Forward declaration
namespace Utils {
    class FileManager;
}

namespace Audio {

/**
 * I2SSpeaker class for MAX98357 I2S audio amplifier
 * 
 * Provides high-quality audio output using I2S protocol with the MAX98357A
 * digital audio amplifier. Supports playing audio data, tones, and sound effects.
 */
class I2SSpeaker {
public:
    /**
     * Constructor
     * 
     * @param bclkPin Bit Clock pin
     * @param wclkPin Word Clock (Left/Right Clock) pin
     * @param dataPin Data pin
     * @param i2sPort I2S port number (I2S_NUM_0 or I2S_NUM_1)
     */
    I2SSpeaker(int bclkPin, int wclkPin, int dataPin, i2s_port_t i2sPort = I2S_NUM_0);

    /**
     * Destructor
     */
    ~I2SSpeaker();

    /**
     * Initialize the I2S speaker
     * 
     * @param sampleRate Sample rate in Hz (8000, 16000, 22050, 44100, 48000)
     * @param bitsPerSample Bits per sample (16 or 32)
     * @return true if initialization was successful, false otherwise
     */
    bool init(uint32_t sampleRate = 16000, int bitsPerSample = 16);

    /**
     * Play a tone at specified frequency for a duration
     * 
     * @param frequency Frequency in Hz (20-20000)
     * @param duration Duration in milliseconds
     * @param volume Volume level (0-100)
     */
    void playTone(int frequency, int duration, int volume = 50);

    /**
     * Play raw audio data
     * 
     * @param data Pointer to audio data
     * @param dataSize Size of audio data in bytes
     * @param volume Volume level (0-100)
     */
    void playAudioData(const uint8_t* data, size_t dataSize, int volume = 50);

    /**
     * Play audio file from SPIFFS
     * 
     * @param filePath Path to audio file in SPIFFS
     * @param volume Volume level (0-100)
     * @return true if successful, false otherwise
     */
    bool playAudioFile(const String& filePath, int volume = 50);

    /**
     * Play MP3 file from SPIFFS
     * 
     * @param filePath Path to MP3 file in SPIFFS
     * @param volume Volume level (0-100)
     * @return true if successful, false otherwise
     */
    bool playMP3File(const String& filePath, int volume = 50);

    /**
     * Play MP3 file using streaming (memory efficient for large files)
     * 
     * @param filePath Path to the MP3 file
     * @param volume Volume level (0-100)
     * @param fileManager FileManager instance for file operations
     * @return true if successful, false otherwise
     */
    bool playMP3FileStreaming(const String& filePath, int volume, Utils::FileManager& fileManager);
    
    /**
     * Play MP3 file using frame-by-frame streaming (optimized for memory usage)
     * 
     * @param filePath Path to the MP3 file in SPIFFS
     * @param volume Volume level (0-100)
     * @return true if successful, false otherwise
     */
    bool playMP3FileStreamingOptimized(const String& filePath, int volume = 50);

    /**
     * Play WAV file (supports PCM 16-bit)
     * 
     * @param filePath Path to the WAV file
     * @param volume Volume level (0-100)
     * @return true if successful, false otherwise
     */
    bool playWAVFile(const String& filePath, int volume = 50);

    /**
     * Play WAV file using streaming (memory efficient for large files)
     * 
     * @param filePath Path to the WAV file
     * @param volume Volume level (0-100)
     * @param fileManager FileManager instance for file operations
     * @return true if successful, false otherwise
     */
    bool playWAVFileStreaming(const String& filePath, int volume, Utils::FileManager& fileManager);

    /**
     * Play a simple beep
     * 
     * @param volume Volume level (0-100)
     */
    void beep(int volume = 50);

    /**
     * Play a double beep
     * 
     * @param volume Volume level (0-100)
     */
    void doubleBeep(int volume = 50);

    /**
     * Play a confirmation sound (rising tones)
     * 
     * @param volume Volume level (0-100)
     */
    void playConfirmation(int volume = 50);

    /**
     * Play an error sound (descending tones)
     * 
     * @param volume Volume level (0-100)
     */
    void playError(int volume = 50);

    /**
     * Play a startup sound
     * 
     * @param volume Volume level (0-100)
     */
    void playStartup(int volume = 50);

    /**
     * Play a notification sound
     * 
     * @param volume Volume level (0-100)
     */
    void playNotification(int volume = 50);

    /**
     * Stop any currently playing sound
     */
    void stop();

    /**
     * Set the default volume
     * 
     * @param volume Volume level (0-100)
     */
    void setVolume(int volume);

    /**
     * Get the current volume
     * 
     * @return Current volume level (0-100)
     */
    int getVolume() const;

    /**
     * Check if the speaker is currently playing
     * 
     * @return true if playing, false otherwise
     */
    bool isPlaying();

    /**
     * Check if the speaker is properly initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;

    /**
     * Generate a sine wave tone
     * 
     * @param frequency Frequency in Hz
     * @param duration Duration in milliseconds
     * @param amplitude Amplitude (0.0 to 1.0)
     * @param sampleBuffer Buffer to store generated samples
     * @param bufferSize Size of the buffer
     * @return Number of samples generated
     */
    size_t generateSineWave(int frequency, int duration, float amplitude, int16_t* sampleBuffer, size_t bufferSize);

    /**
     * Set the sample rate
     * 
     * @param sampleRate New sample rate in Hz
     * @return true if successful, false otherwise
     */
    bool setSampleRate(uint32_t sampleRate);

private:
    int _bclkPin;
    int _wclkPin;
    int _dataPin;
    i2s_port_t _i2sPort;
    bool _initialized;
    int _defaultVolume;
    uint32_t _sampleRate;
    int _bitsPerSample;
    bool _playing;

    /**
     * Configure I2S interface
     * 
     * @return true if successful, false otherwise
     */
    bool configureI2S();

    /**
     * Write audio samples to I2S
     * 
     * @param samples Pointer to sample data
     * @param sampleCount Number of samples
     * @param volume Volume level (0-100)
     */
    void writeSamples(const int16_t* samples, size_t sampleCount, int volume);

    /**
     * Apply volume scaling to samples
     * 
     * @param samples Pointer to sample data
     * @param sampleCount Number of samples
     * @param volume Volume level (0-100)
     */
    void applyVolume(int16_t* samples, size_t sampleCount, int volume);

    int _memType = MALLOC_CAP_DEFAULT;
    int getMemoryType() const {
        return _memType;
    }
};

} // namespace Audio
