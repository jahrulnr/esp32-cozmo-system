#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include "Sstring.h"

namespace Services {

class GPTService {
public:
    // Callback type for GPT responses
    using ResponseCallback = std::function<void(const Utils::Sstring& response)>;

    GPTService();
    ~GPTService();

    /**
     * Initialize GPT adapter
     * @param apiKey The API key for GPT service
     * @return true if initialization was successful, false otherwise
     */
    bool init(const Utils::Sstring& apiKey);

    /**
     * Check if the adapter is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return _initialized; }

    /**
     * Send a prompt to the GPT service
     * @param prompt The prompt to send
     * @param callback Callback function for the response
     */
    void sendPrompt(const Utils::Sstring& prompt, ResponseCallback callback);
    void sendPrompt(const Utils::Sstring& prompt, const Utils::Sstring& additionalCommand, ResponseCallback callback);
    void sendPromptWithCustomSystem(const Utils::Sstring& prompt, const Utils::Sstring& systemCommand, ResponseCallback callback);

    /**
     * Set the model to use
     * @param model The model name (e.g., "gpt-3.5-turbo", "gpt-4")
     */
    void setModel(const Utils::Sstring& model);

    /**
     * Set the system message
     * @param message The system message to use
     */
    void setSystemMessage(const Utils::Sstring& message);

    /**
     * Set the maximum tokens for the response
     * @param maxTokens The maximum number of tokens
     */
    void setMaxTokens(int maxTokens);

    /**
     * Set the temperature for response generation
     * @param temperature Temperature value (0.0 - 1.0)
     */
    void setTemperature(float temperature);

private:
    Utils::Sstring _apiKey;
    Utils::Sstring _model;
    Utils::Sstring _systemMessage;
    int _maxTokens;
    float _temperature;
    bool _initialized;

    // Process the HTTP response
    void processResponse(const Utils::Sstring& response, ResponseCallback callback);
};

} // namespace Communication
