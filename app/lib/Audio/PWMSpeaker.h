#pragma once

#include <Arduino.h>
#include <driver/ledc.h>

namespace Audio {

/**
 * PWMSpeaker class for simple audio output using PWM
 * 
 * Provides basic audio capabilities including tones, beeps, and simple melodies
 * using PWM output on a digital pin connected to a speaker.
 */
class PWMSpeaker {
public:
    /**
     * Constructor
     * 
     * @param pin The digital pin connected to the speaker
     * @param channel PWM channel to use (0-15)
     */
    PWMSpeaker(int pin, int channel = 0);

    /**
     * Destructor
     */
    ~PWMSpeaker();

    /**
     * Initialize the PWM speaker
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool init();

    /**
     * Play a tone at specified frequency for a duration
     * 
     * @param frequency Frequency in Hz (20-20000)
     * @param duration Duration in milliseconds
     * @param volume Volume level (0-100)
     */
    void playTone(int frequency, int duration, int volume = 50);

    /**
     * Play a simple beep
     * 
     * @param volume Volume level (0-100)
     */
    void beep(int volume = 50);

    /**
     * Play audio data from memory
     * 
     * @param data Pointer to audio data
     * @param dataSize Size of audio data in bytes
     * @param sampleRate Sample rate of the audio
     * @param volume Volume level (0-100)
     */
    void playAudioData(const uint8_t* data, size_t dataSize, uint32_t sampleRate, int volume = 50);

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
     * Play a simple melody
     * 
     * @param frequencies Array of frequencies in Hz
     * @param durations Array of durations in milliseconds
     * @param length Number of notes in the melody
     * @param volume Volume level (0-100)
     */
    void playMelody(const int* frequencies, const int* durations, int length, int volume = 50);

private:
    int _pin;
    ledc_channel_t _channel;
    ledc_timer_t _timer;
    bool _initialized;
    int _defaultVolume;
    bool _playing;
    unsigned long _playEndTime;

    /**
     * Internal function to set PWM frequency and duty cycle
     * 
     * @param frequency Frequency in Hz
     * @param volume Volume level (0-100)
     */
    void setPWM(int frequency, int volume);

    /**
     * Internal function to stop PWM output
     */
    void stopPWM();
};

} // namespace Audio
