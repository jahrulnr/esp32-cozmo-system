#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(Utils::FileManager* fileManager, 
                           Utils::Logger* logger, 
                           Notification* notification,
                           mic_fill_cb micCallback) 
    : _fileManager(fileManager), _logger(logger), _notification(notification) {
    
    // Ensure recordings directory exists
    if (_fileManager && !_fileManager->exists(AUDIO_RECORDING_PATH)) {
        _fileManager->createDir(AUDIO_RECORDING_PATH);
    }

    if (micCallback) 
        _micCallback = micCallback;
    
    if (_logger) {
        _logger->info("AudioRecorder initialized");
    }
}

AudioRecorder::~AudioRecorder() {
    // Cleanup if needed
}

bool AudioRecorder::startRecording(uint32_t durationMs) {

    if (isRecordingActive()) {
        if (_logger) _logger->error("Recording already in progress");
        return false;
    }
    
    if (durationMs > 0) {
        _recordingDurationMs = durationMs;
    }
    
    // Pause system tasks first
    pauseSystemTasks();
    
    // Create recording task using SendTask with very high priority and large stack
    _currentTaskId = Command::Send([this]() {
        this->recordingTask();
    }, configMAX_PRIORITIES - 2, "Audio Recording Task", 16384); // 16KB stack for audio processing
    
    if (_currentTaskId.isEmpty()) {
        if (_logger) _logger->error("Failed to create recording task");
        resumeSystemTasks();
        return false;
    }
    
    if (_logger) _logger->info("Recording started with task ID: " + _currentTaskId);
    
    // Notify display about recording status
    if (_notification) {
        _notification->send(NOTIFICATION_DISPLAY, (void*)RECORDING_STARTED);
    }
    
    return true;
}

bool AudioRecorder::isRecordingActive() {
    if (_currentTaskId.isEmpty()) {
        return false;
    }
    
    Command::TaskStatus status = Command::GetTaskStatus(_currentTaskId);
    return (status == Command::TaskStatus::WAITING || status == Command::TaskStatus::INPROGRESS);
}

void AudioRecorder::stopRecording() {
    if (!_currentTaskId.isEmpty()) {
        Command::RemoveTask(_currentTaskId);
        _currentTaskId = "";
        resumeSystemTasks();
    }
}

Command::TaskStatus AudioRecorder::getRecordingStatus() {
    if (_currentTaskId.isEmpty()) {
        return Command::TaskStatus::DONE;
    }
    return Command::GetTaskStatus(_currentTaskId);
}

void AudioRecorder::pauseSystemTasks() {
    // Pause ESP-SR
    _notification->send(NOTIFICATION_SR, (void*)EVENT_SR::PAUSE);
    
    // Pause Automation
    _notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::PAUSE);
    
    // Pause PicoTTS
    _notification->send(NOTIFICATION_TTS, (void*)EVENT_TTS::PAUSE);
    
    // Give tasks time to pause
    vTaskDelay(pdMS_TO_TICKS(500));
}

void AudioRecorder::resumeSystemTasks() {
    // Resume ESP-SR
    _notification->send(NOTIFICATION_SR, (void*)EVENT_SR::RESUME);
    
    // Resume Automation
    _notification->send(NOTIFICATION_AUTOMATION, (void*)EVENT_AUTOMATION::RESUME);
    
    // Resume PicoTTS
    _notification->send(NOTIFICATION_TTS, (void*)EVENT_TTS::RESUME);
}

Utils::Sstring AudioRecorder::generateFileName() {
    return Utils::Sstring(AUDIO_RECORDING_PATH) + "/recording_" + Utils::Sstring(millis()) + ".wav";
}

void AudioRecorder::recordingTask() {
    if (_logger) _logger->info("Recording task started");
    if (!_micCallback) {
        if (_logger) _logger->error("microphone not available");
        resumeSystemTasks();
        return;
    }
    
    // Wait for I2S to stabilize
    vTaskDelay(pdMS_TO_TICKS(100));
    
    recordWav();
    
    resumeSystemTasks();
    
    // Notify completion
    if (_notification) {
        _notification->send(NOTIFICATION_AUDIO, (void*)EVENT_AUDIO::RECORDING_COMPLETE);
        _notification->send(NOTIFICATION_DISPLAY, (void*)RECORDING_STOPPED);
    }
    
    // Clear task ID
    _currentTaskId = "";
}

uint8_t* AudioRecorder::recordWavToMemory(uint32_t durationMs, size_t* out_size) {
    if (!_micCallback) {
        if (_logger) _logger->error("microphone not available");
        resumeSystemTasks();
        return nullptr;
    }
    
    // Get recording parameters from microphone
    uint32_t sample_rate = 16000;
    i2s_data_bit_width_t mic_bits = I2S_DATA_BIT_WIDTH_16BIT;
    i2s_slot_mode_t mic_channels = I2S_SLOT_MODE_STEREO;

    bool need_32_to_16_transform = true;
    bool need_stereo_to_mono_transform = true;
    
    // Output format (what we want in the WAV file)
    uint16_t output_sample_width = 16; // Always 16-bit output
    uint16_t output_channels = 1; // Always mono output for easier processing
    
    // Input format (what microphone provides)
    uint16_t input_sample_width = (mic_bits == I2S_DATA_BIT_WIDTH_32BIT) ? 32 : 16;
    uint16_t input_channels = (mic_channels == I2S_SLOT_MODE_STEREO) ? 2 : 1;
    
    // Calculate recording size (output format)
    size_t samples_per_ms = sample_rate / 1000;
    size_t total_output_samples = samples_per_ms * durationMs;
    size_t output_rec_size = total_output_samples * (output_sample_width / 8) * output_channels;
    
    // Calculate input buffer size (may be larger due to transforms)
    size_t input_samples_per_read = AUDIO_BUFFER_SIZE;
    size_t input_bytes_per_sample = (input_sample_width / 8) * input_channels;
    size_t input_buffer_size = input_samples_per_read * input_bytes_per_sample;
    
    // Create WAV header using ESP_I2S.cpp pattern (PCM_WAV_HEADER_DEFAULT equivalent)
    WAVHeader wav_header;
    memset(&wav_header, 0, sizeof(wav_header));
    
    // RIFF chunk
    memcpy(wav_header.riff, "RIFF", 4);
    wav_header.fileSize = output_rec_size + sizeof(WAVHeader) - 8;
    memcpy(wav_header.wave, "WAVE", 4);
    
    // Format chunk  
    memcpy(wav_header.fmt, "fmt ", 4);
    wav_header.fmtSize = 16;
    wav_header.audioFormat = 1; // PCM
    wav_header.channels = output_channels;
    wav_header.sampleRate = sample_rate;
    wav_header.bitsPerSample = output_sample_width;
    wav_header.blockAlign = output_channels * (output_sample_width / 8);
    wav_header.byteRate = sample_rate * wav_header.blockAlign;
    
    // Data chunk
    memcpy(wav_header.data, "data", 4);
    wav_header.dataSize = output_rec_size;
    
    if (_logger) {
        _logger->info("ESP_I2S Recording: " + String(sample_rate) + "Hz, " + 
                     String(input_sample_width) + "→" + String(output_sample_width) + "bit, " +
                     String(input_channels) + "→" + String(output_channels) + "ch");
    }
    
    // Allocate WAV buffer (ESP_I2S.cpp pattern)
    const size_t WAVE_HEADER_SIZE = sizeof(WAVHeader);
    uint8_t* wav_buf = (uint8_t*)malloc(output_rec_size + WAVE_HEADER_SIZE);
    if (wav_buf == NULL) {
        if (_logger) _logger->error("Failed to allocate WAV buffer");
        *out_size = 0;
        return nullptr;
    }
    
    // Allocate temporary input buffer for transforms (ESP_I2S.cpp pattern)
    uint8_t* input_buf = nullptr;
    int16_t* temp_buf = nullptr;
    
    if (need_32_to_16_transform || need_stereo_to_mono_transform) {
        input_buf = (uint8_t*)malloc(input_buffer_size);
        temp_buf = (int16_t*)malloc(input_samples_per_read * sizeof(int16_t) * input_channels);
        
        if (!input_buf || !temp_buf) {
            if (_logger) _logger->error("Failed to allocate transform buffers");
            free(wav_buf);
            if (input_buf) free(input_buf);
            if (temp_buf) free(temp_buf);
            *out_size = 0;
            return nullptr;
        }
    }
    
    // Copy header to buffer
    memcpy(wav_buf, &wav_header, WAVE_HEADER_SIZE);
    
    // Read audio data with transforms (ESP_I2S.cpp pattern)
    uint8_t* audio_data = wav_buf + WAVE_HEADER_SIZE;
    size_t total_output_written = 0;
    uint32_t start_time = millis();
    
    // Reading loop with transforms
    while (total_output_written < output_rec_size && (millis() - start_time) < (durationMs + 1000)) {
        size_t bytes_read = 0;
        uint8_t* read_buffer = (input_buf) ? input_buf : (audio_data + total_output_written);
        size_t read_size = (input_buf) ? input_buffer_size : (output_rec_size - total_output_written);
        
        // Read raw audio data
        esp_err_t err = _micCallback(nullptr, read_buffer, read_size, &bytes_read, 100);
        
        if (err == ESP_OK && bytes_read > 0) {
            uint8_t* final_data = read_buffer;
            size_t final_bytes = bytes_read;
            
            // Apply transforms if needed (ESP_I2S.cpp pattern)
            if (need_32_to_16_transform || need_stereo_to_mono_transform) {
                size_t input_samples = bytes_read / input_bytes_per_sample;
                
                // 32-bit to 16-bit transform
                if (need_32_to_16_transform) {
                    transform32To16((uint32_t*)read_buffer, temp_buf, input_samples);
                    final_data = (uint8_t*)temp_buf;
                    final_bytes = input_samples * sizeof(int16_t);
                } else {
                    // Copy to temp buffer for stereo transform
                    memcpy(temp_buf, read_buffer, bytes_read);
                    final_data = (uint8_t*)temp_buf;
                }
                
                // Stereo to mono transform
                if (need_stereo_to_mono_transform) {
                    size_t stereo_samples = final_bytes / sizeof(int16_t);
                    transformStereoToMono((int16_t*)final_data, (int16_t*)final_data, stereo_samples);
                    final_bytes = stereo_samples / 2 * sizeof(int16_t);
                }
            }
            
            // Copy to output buffer
            size_t remaining_space = output_rec_size - total_output_written;
            size_t copy_bytes = (final_bytes > remaining_space) ? remaining_space : final_bytes;
            
            memcpy(audio_data + total_output_written, final_data, copy_bytes);
            total_output_written += copy_bytes;
            
        } else if (err != ESP_ERR_TIMEOUT) {
            if (_logger) _logger->error("Audio read error: " + String(esp_err_to_name(err)));
            break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1)); // Small delay
    }
    
    // Clean up transform buffers
    if (input_buf) free(input_buf);
    if (temp_buf) free(temp_buf);
    
    if (total_output_written < output_rec_size && _logger) {
        _logger->warning("Incomplete: " + String(total_output_written) + "/" + String(output_rec_size));
        
        // Update header for actual recorded size
        WAVHeader* header_ptr = (WAVHeader*)wav_buf;
        header_ptr->dataSize = total_output_written;
        header_ptr->fileSize = total_output_written + WAVE_HEADER_SIZE - 8;
    }
    
    *out_size = total_output_written + WAVE_HEADER_SIZE;
    
    if (_logger) {
        _logger->info("Recording completed: " + String(total_output_written) + " bytes (" + 
                     String((millis() - start_time) / 1000.0, 1) + "s)");
    }
    
    return wav_buf;
}

void AudioRecorder::recordWav() {
    uint32_t duration = _recordingDurationMs;
    size_t wav_size = 0;
    
    // Use memory-based recording (ESP_I2S.cpp pattern)
    uint8_t* wav_buffer = recordWavToMemory(duration, &wav_size);
    
    if (!wav_buffer || wav_size == 0) {
        if (_logger) _logger->error("Recording failed");
        if (wav_buffer) free(wav_buffer);
        return;
    }
    
    // Generate filename and write to file
    Utils::Sstring fileName = generateFileName();
    
    File wavFile = _fileManager->openFileForWriting(String(fileName.c_str()));
    if (!wavFile) {
        if (_logger) _logger->error("File open failed");
        free(wav_buffer);
        return;
    }
    
    // Write entire WAV buffer to file
    size_t written = _fileManager->writeBinary(wavFile, wav_buffer, wav_size);
    wavFile.close();
    
    if (written == wav_size && _logger) {
        _logger->info("Saved: " + String(fileName.c_str()) + " (" + String(wav_size) + " bytes)");
    }
    
    // Clean up memory
    free(wav_buffer);
}

void AudioRecorder::transform32To16(uint32_t* src, int16_t* dst, size_t sampleCount) {
    // ESP_I2S.cpp pattern: convert 32-bit samples to 16-bit by taking upper 16 bits
    for (size_t i = 0; i < sampleCount; i++) {
        dst[i] = src[i] >> 16;
    }
}

void AudioRecorder::transformStereoToMono(int16_t* src, int16_t* dst, size_t stereoSampleCount) {
    // ESP_I2S.cpp pattern: convert stereo to mono by taking left channel
    for (size_t i = 0; i < stereoSampleCount; i += 2) {
        dst[i / 2] = src[i]; // Take left channel sample
    }
}