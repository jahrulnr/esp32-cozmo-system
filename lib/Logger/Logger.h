#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <vector>
#include <cstdarg>

namespace Utils {

#ifdef ARDUHAL_LOG_FORMAT
#undef ARDUHAL_LOG_FORMAT
#define ARDUHAL_LOG_FORMAT(letter, format)                                                                                                              \
  ARDUHAL_LOG_COLOR_##letter "[" #letter "][%s:%u] %s(): " format ARDUHAL_LOG_RESET_COLOR "\r\n", \
    pathToFileName(__FILE__), __LINE__, __FUNCTION__
#endif

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
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
    bool init(bool serialEnabled = true, bool fileEnabled = false);

    /**
     * Set minimum log level
     * @param level The minimum log level to display
     */
    void setLogLevel(LogLevel level);

    /**
     * Get current log level
     * @return The current minimum log level
     */
    LogLevel getLogLevel() const;

    /**
     * Check if a specific log level is enabled
     * @param level The log level to check
     * @return true if the specified log level is enabled, false otherwise
     */
    bool isLogLevelEnabled(LogLevel level) const;

    /**
     * Log a formatted debug message (printf-style)
     * @param format The format string with placeholders like %d, %s, etc.
     * @param ... Variable arguments to fill the format
     */
    void debug(const char* format, ...);
    void debug(const String& format, ...);

    /**
     * Log a formatted info message (printf-style)
     * @param format The format string with placeholders like %d, %s, etc.
     * @param ... Variable arguments to fill the format
     */
    void info(const char* format, ...);
    void info(const String& format, ...);

    /**
     * Log a formatted warning message (printf-style)
     * @param format The format string with placeholders like %d, %s, etc.
     * @param ... Variable arguments to fill the format
     */
    void warning(const char* format, ...);
    void warning(const String& format, ...);

    /**
     * Log a formatted error message (printf-style)
     * @param format The format string with placeholders like %d, %s, etc.
     * @param ... Variable arguments to fill the format
     */
    void error(const char* format, ...);
    void error(const String& format, ...);

    /**
     * Log a message with a specific level
     * @param level The log level
     * @param message The message to log
     */
    void log(LogLevel level, const String& message);

    /**
     * Log a formatted message with a specific level (printf-style)
     * @param level The log level
     * @param format The format string with placeholders like %d, %s, etc.
     * @param ... Variable arguments to fill the format
     */
    void log(LogLevel level, const char* format, ...);

private:
    const char* TAG;
    Logger();
    ~Logger();

    // Disable copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    bool _serialEnabled;
    bool _fileEnabled;
    String _fileName;
    LogLevel _logLevel;

    // Convert log level to string
    String logLevelToString(LogLevel level);

    // Convert log level to lowercase string (for frontend display)
    String logLevelToLowerString(LogLevel level);

    /**
     * Helper method to format a string with variable arguments
     * @param format The format string with placeholders
     * @param args Variable argument list
     * @return Formatted string
     */
    String formatString(const char* format, va_list args);
};

} // namespace Utils
