#include "Logger.h"
#include <SPIFFS.h>

namespace Utils {

Logger::Logger() : _serialEnabled(true), _fileEnabled(false), _fileName("/logs.txt"), _logLevel(LogLevel::INFO) {
}

Logger::~Logger() {
    // Clean up resources if needed
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::init(bool serialEnabled, bool fileEnabled, const String& fileName) {
    _serialEnabled = serialEnabled;
    _fileEnabled = fileEnabled;
    _fileName = fileName;
    
    if (_serialEnabled) {
        Serial.begin(115200);
    }
    
    if (_fileEnabled && !SPIFFS.begin()) {
        if (_serialEnabled) {
            Serial.println("Failed to mount SPIFFS");
        }
        _fileEnabled = false;
        return false;
    }
    
    return true;
}

void Logger::setLogLevel(LogLevel level) {
    _logLevel = level;
}

void Logger::debug(const String& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const String& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const String& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const String& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const String& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::log(LogLevel level, const String& message) {
    if (level < _logLevel) {
        return;
    }
    
    String logMessage = String(millis()) + " [" + logLevelToString(level) + "] " + message;
    
    if (_serialEnabled) {
        Serial.println(logMessage);
    }
    
    if (_fileEnabled) {
        File file = SPIFFS.open(_fileName, "a");
        if (file) {
            file.println(logMessage);
            file.close();
        }
    }
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
        case LogLevel::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

} // namespace Utils
