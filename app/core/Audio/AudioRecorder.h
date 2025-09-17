#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FS.h"
#include "Config.h"
#include "Constants.h"
#include "Sstring.h"
#include "Logger.h"
#include "SendTask.h"
#include "Notification.h"
#include "FileManager.h"
#include "AnalogMicrophone.h"
#include "I2SMicrophone.h"
#include "SendTask.h"
#include <wav_header.h>
#include <cstring>

class AudioRecorder {
private:
    typedef esp_err_t (*mic_fill_cb)(void* arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms);

    Utils::FileManager* _fileManager;
    Utils::Logger* _logger;
    Notification* _notification;
    mic_fill_cb _micCallback;

    uint32_t _recordingDurationMs = AUDIO_RECORDING_DURATION_MS;
    String _currentTaskId;  // Track current recording task

    // WAV header structure
    struct WAVHeader {
        char riff[4] = {'R', 'I', 'F', 'F'};
        uint32_t fileSize;
        char wave[4] = {'W', 'A', 'V', 'E'};
        char fmt[4] = {'f', 'm', 't', ' '};
        uint32_t fmtSize = 16;
        uint16_t audioFormat = 1; // PCM
        uint16_t channels = AUDIO_CHANNELS;
        uint32_t sampleRate = AUDIO_SAMPLE_RATE;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample = AUDIO_BITS_PER_SAMPLE;
        char data[4] = {'d', 'a', 't', 'a'};
        uint32_t dataSize;
    };

    Utils::Sstring generateFileName();
    void pauseSystemTasks();
    void resumeSystemTasks();

    // ESP_I2S library inspired recording method
    uint8_t* recordWavToMemory(uint32_t durationMs, size_t* out_size);
    void recordWav();

    // ESP_I2S inspired data transform functions
    void transform32To16(uint32_t* src, int16_t* dst, size_t sampleCount);
    void transformStereoToMono(int16_t* src, int16_t* dst, size_t stereoSampleCount);

public:
    // Constructor with dependency injection
    AudioRecorder(Utils::FileManager* fileManager,
                  Utils::Logger* logger,
                  Notification* notification,
                  mic_fill_cb micCallback);
    ~AudioRecorder();

    bool startRecording(uint32_t durationMs = 0); // 0 = use default duration
    void setRecordingDuration(uint32_t durationMs) { _recordingDurationMs = durationMs; }
    uint32_t getRecordingDuration() const { return _recordingDurationMs; }

    // Recording status methods
    bool isRecordingActive();
    void stopRecording();
    Command::TaskStatus getRecordingStatus();

    // Recording task function (will be used by SendTask)
    void recordingTask();
};

#endif
