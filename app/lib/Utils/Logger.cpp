#include "Logger.h"
#include <SPIFFS.h>
#include "lib/Utils/SpiAllocator.h"

namespace Utils {

Logger::Logger() : _serialEnabled(true), _fileEnabled(false), _fileName("/logs.txt"), 
                 _logLevel(LogLevel::INFO), _webSocket(nullptr), _logTaskRunning(false),
                 _batchSize(50), _flushIntervalMs(500), _lastFlushTime(0) {
    // Create queue for log messages (larger size to handle high log volumes)
    _logQueue = xQueueCreate(50, sizeof(LogMessage*));
    
    // Create task for processing log messages
    xTaskCreate(
        logTask,
        "LoggerTask",
        4 * 1024,  // Stack size
        this,  // Pass this instance as parameter
        1,     // Priority
        &_logTaskHandle
    );
}

Logger::~Logger() {
    // Clean up resources
    if (_logTaskRunning) {
        _logTaskRunning = false;
        vTaskDelay(pdMS_TO_TICKS(100));
        vTaskDelete(_logTaskHandle);
    }
    
    if (_logQueue != nullptr) {
        vQueueDelete(_logQueue);
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::init(bool serialEnabled, bool fileEnabled, const String& fileName,
              int batchSize, int flushIntervalMs) {
    _serialEnabled = serialEnabled;
    _fileEnabled = fileEnabled;
    _fileName = fileName;
    _batchSize = batchSize > 0 ? batchSize : 5;  // Ensure positive value
    _flushIntervalMs = flushIntervalMs > 0 ? flushIntervalMs : 500;  // Ensure positive value
    
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

void Logger::critical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    String formattedMessage = formatString(format, args);
    va_end(args);
    
    log(LogLevel::CRITICAL, formattedMessage);
}

void Logger::critical(const String& format, ...) {
    critical(format.c_str());
}

void Logger::log(LogLevel level, const String& message) {
    if (level < _logLevel) {
        return;
    }
    
    unsigned long currentTime = millis();
    String logMessage = String(currentTime) + " [" + logLevelToString(level) + "] " + message;
    
    // Always log to Serial and file synchronously for immediate feedback
    if (_serialEnabled) {
        Serial.println(logMessage);
    }
    
    // if (_fileEnabled) {
    //     File file = SPIFFS.open(_fileName, "a");
    //     if (file) {
    //         file.println(logMessage);
    //         file.close();
    //     }
    // }
    
    // Queue the message for async WebSocket sending
    if (_webSocket && _webSocket->hasClients()) {
        // Create log message object for the queue with timestamp
        unsigned long currentTime = millis();
        LogMessage* msg = new LogMessage{message, level, currentTime};
        
        // Send to queue, don't wait if queue is full (non-blocking)
        if (xQueueSend(_logQueue, &msg, 0) != pdTRUE) {
            // If queue is full, delete the message to prevent memory leak
            delete msg;
            
            // Optional: Log this to serial only to avoid recursion
            if (_serialEnabled) {
                Serial.println("WARNING: Log queue full, message dropped");
            }
        }
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
        case LogLevel::CRITICAL:
            return "CRITICAL";
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
        case LogLevel::CRITICAL:
            return "critical";
        default:
            return "unknown";
    }
}

void Logger::setWebSocket(Communication::WebSocketHandler* ws) {
    _webSocket = ws;
    
    if (_webSocket) {
        if (_serialEnabled) {
            Serial.println("Logger WebSocket handler set successfully");
        }
    } else {
        if (_serialEnabled) {
            Serial.println("WARNING: Null WebSocket handler provided to Logger");
        }
    }
}

void Logger::flushLogs() {
    // Set last flush time to a past time to trigger immediate flush
    _lastFlushTime = millis() - _flushIntervalMs - 1;
    
    // Allow a short delay for the log task to process the flush
    vTaskDelay(pdMS_TO_TICKS(20));
}

void Logger::logTask(void* parameter) {
    Logger* logger = static_cast<Logger*>(parameter);
    logger->_logTaskRunning = true;
    
    // Debug info to console only
    if (logger->_serialEnabled) {
        Serial.println("Logger task started");
    }
    
    // Local container for batching log messages
    std::vector<LogMessage*> logBatch;
    logBatch.reserve(logger->_batchSize); // Preallocate to avoid reallocations
    
    // When was the last time we processed messages
    logger->_lastFlushTime = millis();
    
    // Process messages from the queue
    while (logger->_logTaskRunning) {
        LogMessage* msg = nullptr;
        unsigned long currentTime = millis();
        bool timeToFlush = (currentTime - logger->_lastFlushTime) >= logger->_flushIntervalMs;
        
        // Try to get a message with very short timeout to avoid blocking
        if (xQueueReceive(logger->_logQueue, &msg, timeToFlush ? pdMS_TO_TICKS(100) : pdMS_TO_TICKS(1000)) == pdTRUE) {
            if (msg != nullptr) {
                // Add to batch
                logBatch.push_back(msg);
            }
        }
        
        // Process batch if it's full or time to flush
        if (logBatch.size() >= logger->_batchSize || (timeToFlush && !logBatch.empty())) {
            // Only send if we have a WebSocket and clients
            if (logger->_webSocket && logger->_webSocket->hasClients()) {
                // Create a batch message JSON structure
                Utils::SpiJsonDocument batchData;
                
                for (LogMessage* logMsg : logBatch) {
                    JsonObject log = batchData["logs"].add<JsonObject>();
                    log["message"] = logMsg->message;
                    log["level"] = logger->logLevelToLowerString(logMsg->level);
                    log["timestamp"] = logMsg->timestamp;
                }
                batchData.shrinkToFit();
                
                // Send as a batch_log_message type for better frontend handling
                // logger->_webSocket->sendJsonMessage(-1, "batch_log_messages", batchData);
            }
            
            // Cleanup all messages
            for (LogMessage* logMsg : logBatch) {
                delete logMsg;
            }
            
            logBatch.clear();
            logger->_lastFlushTime = currentTime;
            
            // Give other tasks time to run after processing a batch
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        
        // If no messages and not time to flush, give other tasks some CPU time
        if (logBatch.empty() && !timeToFlush) {
            vTaskDelay(pdMS_TO_TICKS(25));
        }
    }
    
    // Clean up any remaining messages in the batch before exiting
    for (LogMessage* logMsg : logBatch) {
        delete logMsg;
    }
    logBatch.clear();
    
    // Debug info to console only
    if (logger->_serialEnabled) {
        Serial.println("Logger task shutting down");
    }
    
    // Task is ending
    logger->_logTaskRunning = false;
    vTaskDelete(NULL);
}

} // namespace Utils
