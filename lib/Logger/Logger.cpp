#include "Logger.h"
#include <LittleFS.h>

namespace Utils {

Logger::Logger() : _serialEnabled(true), _fileEnabled(false), _fileName("/logs.txt"), 
                 _logLevel(LogLevel::INFO), TAG("Logger") {
}

Logger::~Logger() {}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::init(bool serialEnabled, bool fileEnabled) {
    _serialEnabled = serialEnabled;
    _fileEnabled = fileEnabled;
    _fileName = "/logs.txt";
    
    if (_fileEnabled && !LittleFS.begin(false)) {
        if (_serialEnabled) {
            ESP_LOGE(TAG, "Failed to mount LittleFS");
        }
        _fileEnabled = false;
        return false;
    }

    if (!Serial) {
        _logLevel = LogLevel::ERROR;
    }
    
    return true;
}

void Logger::setLogLevel(LogLevel level) {
    _logLevel = level;
}

LogLevel Logger::getLogLevel() const {
    return _logLevel;
}

bool Logger::isLogLevelEnabled(LogLevel level) const {
    return level >= _logLevel;
}

String Logger::formatString(const char* format, va_list args) {
    char buffer[256]; // Buffer to hold formatted string
    
    // Format the string
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    // Return as a String object
    return String(buffer);
}

void Logger::debug(const String& format, ...) {
    debug(format.c_str());
}

void Logger::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    String formattedMessage = formatString(format, args);
    va_end(args);
    
    log(LogLevel::DEBUG, formattedMessage);
}

void Logger::info(const String& format, ...) {
    info(format.c_str());
}

void Logger::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    String formattedMessage = formatString(format, args);
    va_end(args);
    
    log(LogLevel::INFO, formattedMessage);
}

void Logger::warning(const String& format, ...) {
    warning(format.c_str());
}

void Logger::warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    String formattedMessage = formatString(format, args);
    va_end(args);
    
    log(LogLevel::WARNING, formattedMessage);
}

void Logger::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    String formattedMessage = formatString(format, args);
    va_end(args);
    
    log(LogLevel::ERROR, formattedMessage);
}

void Logger::error(const String& format, ...) {
    error(format.c_str());
}

void Logger::log(LogLevel level, const String& message) {
    if (level < _logLevel) {
        return;
    }
    
    // Always log to Serial and file synchronously for immediate feedback
    const char* msg = message.c_str();
    if (_serialEnabled) {
        switch(level){
            case LogLevel::INFO:
                log_i("%s", msg);
                break;
            case LogLevel::WARNING:
                log_w("%s", msg);
                break;
            case LogLevel::ERROR:
                log_e("%s", msg);
                break;
            default:
                log_d("%s", msg);
                break;
        };
    }
}

void Logger::log(LogLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    String formattedMessage = formatString(format, args);
    va_end(args);
    
    log(level, formattedMessage);
}

String Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

String Logger::logLevelToLowerString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "debug";
        case LogLevel::INFO:
            return "info";
        case LogLevel::WARNING:
            return "warning";
        case LogLevel::ERROR:
            return "error";
        default:
            return "unknown";
    }
}

} // namespace Utils
