#pragma once

#include <Arduino.h>

namespace Sensors {

/**
 * MicrophoneSensor class for MAX9814 microphone
 * 
 * Provides interface for reading audio levels and detecting sound events
 * from the MAX9814 electret microphone amplifier with AGC.
 */
class MicrophoneSensor {
public:
    /**
     * Constructor
     * 
     * @param analogPin The analog pin connected to the MAX9814 output
     * @param gainPin Optional gain control pin (leave -1 if not used)
     * @param attackReleasePin Optional attack/release control pin (leave -1 if not used)
     */
    MicrophoneSensor(int analogPin, int gainPin = -1, int attackReleasePin = -1);

    /**
     * Destructor
     */
    ~MicrophoneSensor();

    /**
     * Initialize the microphone sensor
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool init();

    /**
     * Read the current audio level
     * 
     * @return Audio level value (0-4095 for 12-bit ADC)
     */
    int readLevel();

    /**
     * Read the peak audio level over a specified duration
     * 
     * @param durationMs Duration to sample in milliseconds
     * @return Peak audio level during the sampling period
     */
    int readPeakLevel(int durationMs = 100);

    /**
     * Read the average audio level over a specified duration
     * 
     * @param durationMs Duration to sample in milliseconds
     * @return Average audio level during the sampling period
     */
    int readAverageLevel(int durationMs = 100);

    /**
     * Check if sound is detected above a threshold
     * 
     * @param threshold The threshold level to compare against
     * @return true if sound level is above threshold, false otherwise
     */
    bool isSoundDetected(int threshold = 2000);

    /**
     * Set the gain level (if gain pin is connected)
     * 
     * @param gainLevel Gain level (LOW = 40dB, HIGH = 50dB, FLOATING = 60dB)
     */
    void setGain(int gainLevel);

    /**
     * Set the attack/release time (if attack/release pin is connected)
     * 
     * @param attackRelease Attack/release setting (LOW = fast, HIGH = slow)
     */
    void setAttackRelease(bool attackRelease);

    /**
     * Get the baseline noise level (for calibration)
     * 
     * @param samplingTime Time to sample baseline in milliseconds
     * @return Baseline noise level
     */
    int calibrateBaseline(int samplingTime = 1000);

    /**
     * Check if the sensor is properly initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;

private:
    int _analogPin;
    int _gainPin;
    int _attackReleasePin;
    bool _initialized;
    int _baselineLevel;
    
    /**
     * Internal function to read multiple samples
     * 
     * @param samples Number of samples to read
     * @param delayMs Delay between samples in milliseconds
     * @return Array of sample values (caller must delete)
     */
    int* readSamples(int samples, int delayMs = 1);
};

} // namespace Sensors
