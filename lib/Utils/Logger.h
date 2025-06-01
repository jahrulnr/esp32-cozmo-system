#pragma once

#include <Arduino.h>

namespace Utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
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
     * @return true if initialization was successful, false otherwise
     */
    bool init(bool serialEnabled = true, bool fileEnabled = false, const String& fileName = "/logs.txt");
    
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
    
    // Convert log level to string
    String logLevelToString(LogLevel level);
};

} // namespace Utils
