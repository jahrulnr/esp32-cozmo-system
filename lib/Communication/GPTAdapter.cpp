#include "GPTAdapter.h"
#include <HTTPClient.h>

namespace Communication {

GPTAdapter::GPTAdapter() : _model("gpt-3.5-turbo"), _systemMessage("You are a helpful assistant."),
                           _maxTokens(1024), _temperature(0.7), _initialized(false) {
}

GPTAdapter::~GPTAdapter() {
    // Clean up resources if needed
}

bool GPTAdapter::init(const String& apiKey) {
    _apiKey = apiKey;
    _initialized = true;
    return true;
}

void GPTAdapter::sendPrompt(const String& prompt, ResponseCallback callback) {
    if (!_initialized) {
        if (callback) {
            callback("Error: GPT adapter not initialized");
        }
        return;
    }
    
    HTTPClient http;
    http.begin("https://api.openai.com/v1/chat/completions");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + _apiKey);
    
    // Prepare JSON payload
    JsonDocument doc;
    doc["model"] = _model;
    doc["temperature"] = _temperature;
    doc["max_tokens"] = _maxTokens;
    
    JsonArray messages = doc.createNestedArray("messages");
    
    // System message
    JsonObject systemMsg = messages.createNestedObject();
    systemMsg["role"] = "system";
    systemMsg["content"] = _systemMessage;
    
    // User message
    JsonObject userMsg = messages.createNestedObject();
    userMsg["role"] = "user";
    userMsg["content"] = prompt;
    
    String payload;
    serializeJson(doc, payload);
    
    int httpCode = http.POST(payload);
    
    if (httpCode > 0) {
        String response = http.getString();
        processResponse(response, callback);
    } else {
        if (callback) {
            callback("Error: " + http.errorToString(httpCode));
        }
    }
    
    http.end();
}

void GPTAdapter::setModel(const String& model) {
    _model = model;
}

void GPTAdapter::setSystemMessage(const String& message) {
    _systemMessage = message;
}

void GPTAdapter::setMaxTokens(int maxTokens) {
    _maxTokens = maxTokens;
}

void GPTAdapter::setTemperature(float temperature) {
    _temperature = constrain(temperature, 0.0f, 1.0f);
}

void GPTAdapter::processResponse(const String& response, ResponseCallback callback) {
    if (!callback) {
        return;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        callback("Error parsing JSON: " + String(error.c_str()));
        return;
    }
    
    if (doc.containsKey("error")) {
        callback("API Error: " + doc["error"]["message"].as<String>());
        return;
    }
    
    // Extract the assistant's message
    if (doc.containsKey("choices") && doc["choices"].size() > 0) {
        String content = doc["choices"][0]["message"]["content"].as<String>();
        callback(content);
    } else {
        callback("Error: Unexpected response format");
    }
}

} // namespace Communication
