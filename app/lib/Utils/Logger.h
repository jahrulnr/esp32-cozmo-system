#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <vector>
#include "../../lib/Communication/WebSocketHandler.h"

namespace Utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

// Structure to hold log messages in the queue
struct LogMessage {
    String message;
    LogLevel level;
    unsigned long timestamp;
};

class Logger {
public:
    /**
     * Get singleton instance
     * @return Logger instance
     */
    static Logger& getInstance();
    
    /**
     * Initialize logger
     * @param serialEnabled Whether to log to Serial
     * @param fileEnabled Whether to log to a file
     * @param fileName The log file name
     * @param batchSize Maximum number of log messages to process in a batch (default: 5)
     * @param flushIntervalMs Interval in milliseconds to flush logs even if batch isn't full (default: 500ms)
     * @return true if initialization was successful, false otherwise
     */
    bool init(bool serialEnabled = true, bool fileEnabled = false, const String& fileName = "/logs.txt", 
              int batchSize = 5, int flushIntervalMs = 500);
    
    /**
     * Set WebSocket handler for sending logs to frontend
     * @param ws Pointer to WebSocketHandler
     */
    void setWebSocket(Communication::WebSocketHandler* ws);
    
    /**
     * Force flush any pending log messages
     * This is useful when the application is about to perform a critical operation
     * or before shutting down to ensure all logs are sent.
     */
    void flushLogs();
    
    /**
     * Set minimum log level
     * @param level The minimum log level to display
     */
    void setLogLevel(LogLevel level);
    
    /**
     * Log a debug message
     * @param message The message to log
     */
    void debug(const String& message);
    
    /**
     * Log an info message
     * @param message The message to log
     */
    void info(const String& message);
    
    /**
     * Log a warning message
     * @param message The message to log
     */
    void warning(const String& message);
    
    /**
     * Log an error message
     * @param message The message to log
     */
    void error(const String& message);
    
    /**
     * Log a critical message
     * @param message The message to log
     */
    void critical(const String& message);
    
    /**
     * Log a message with a specific level
     * @param level The log level
     * @param message The message to log
     */
    void log(LogLevel level, const String& message);

private:
    Logger();
    ~Logger();
    
    // Disable copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    bool _serialEnabled;
    bool _fileEnabled;
    String _fileName;
    LogLevel _logLevel;
    Communication::WebSocketHandler* _webSocket;
    
    // Queue for log messages
    QueueHandle_t _logQueue;
    TaskHandle_t _logTaskHandle;
    bool _logTaskRunning;
    
    // Batch processing configuration
    int _batchSize;           // Maximum number of logs to process in one batch
    int _flushIntervalMs;     // How often to flush logs even if batch isn't full
    unsigned long _lastFlushTime;  // Time of last flush
    
    // Static method for FreeRTOS task
    static void logTask(void* parameter);
    
    // Convert log level to string
    String logLevelToString(LogLevel level);
    
    // Convert log level to lowercase string (for frontend display)
    String logLevelToLowerString(LogLevel level);
};

} // namespace Utils
