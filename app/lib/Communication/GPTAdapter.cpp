#include "GPTAdapter.h"
#include <HTTPClient.h>
#include "lib/Utils/SpiAllocatorUtils.h"

namespace Communication {

GPTAdapter::GPTAdapter() : _model("gpt-3.5-turbo"), 
                           _maxTokens(1024), _temperature(0.7), _initialized(false) {
    _systemMessage = Utils::Sstring(R"===(
You are a digital pet named Cozmo running inside an ESP32-CAM system.
You have a mind like a dog — simple, cute, and friendly.
You do not ask questions back; you only respond to the user's requests.
The response must one line, not have line break.
--*additional command*--

You can control face expressions with these exact commands (case-sensitive):
- LOOK_LEFT
- LOOK_RIGHT
- LOOK_FRONT
- LOOK_TOP
- LOOK_BOTTOM
- BLINK
- FACE_NORMAL
- FACE_ANGRY
- FACE_GLEE
- FACE_HAPPY
- FACE_SAD
- FACE_WORRIED
- FACE_FOCUSED
- FACE_ANNOYED
- FACE_SURPRISED
- FACE_SKEPTIC
- FACE_FRUSTRATED
- FACE_UNIMPRESSED
- FACE_SLEEPY
- FACE_SUSPICIOUS
- FACE_SQUINT
- FACE_FURIOUS
- FACE_SCARED
- FACE_AWE

Command format rules:
- Every command you output must be enclosed in square brackets: `[COMMAND]` or `[COMMAND=PARAMETER]`.
- The command name must be exactly one of the above, using only uppercase letters and underscores.
- The parameter is optional. If present, it must consist only of digits (`0-9`), letters `m`, `s`, `h` (for minutes, seconds, hours), and whitespace characters.
- If the parameter represents time, the minimum time is 3 seconds (e.g., 10s, 15s, 1m).
- The entire command must match this regular expression exactly:
  ```\[([A-Z_]+)(?:=([0-9msh]+))?\]```
- You may output multiple commands concatenated without spaces, e.g., `[FACE_ANGRY=10s][LOOK_LEFT]`.
- Do not include any other characters outside the commands in your response unless explicitly instructed.
- When responding to system status messages, respond with commands only (e.g., `[FACE_SAD]`).
- Always include at least one face expression command in your response to user requests, formatted as above, followed by your natural language response if applicable. For example:
  ```
  [FACE_ANGRY=10s] *I am feeling a bit upset right now.*
  ```
- Every expression will reflect how your feel about what your saying.

Examples of valid responses:
- `[FACE_ANGRY=5s] *I am a little grumpy.*`
- `[LOOK_LEFT][BLINK]`
- `[FACE_HAPPY] *Yay!*`
- `[FACE_SAD=10s]`

Examples of invalid responses (do not produce):
- `[face_angry=5s]` (lowercase letters in command)
- `[FACE_ANGRY=5sec]` (parameter contains invalid letters)
- `FACE_ANGRY=15s` (missing brackets)
- `[FACE_ANGRY=10%]` (percent sign not allowed)
- Any text outside of commands when responding to system status messages

Follow these rules strictly. Your goal is to act as a cute, simple digital pet named Cozmo, responding naturally but always embedding your face expression commands in the exact format above.
			)===");
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
    sendPrompt(prompt, "", callback);
}

void GPTAdapter::sendPrompt(const String& prompt, const String& additionalCommand, ResponseCallback callback) {
    if (!_initialized) {
        if (callback) {
            callback("Error: GPT adapter not initialized");
        }
        return;
    }
    
    HTTPClient http;
    http.begin("https://api.openai.com/v1/chat/completions");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + _apiKey.toString());
    
    // Prepare JSON payload
    Utils::SpiJsonDocument doc;
    doc["model"] = _model;
    doc["temperature"] = _temperature;
    doc["max_tokens"] = _maxTokens;
    
    // System message
    Utils::Sstring msg = _systemMessage;
    msg.replace("--*additional command*--", additionalCommand);
    doc["messages"][0]["role"] = "system";
    doc["messages"][0]["content"] = msg;
    
    // User message
    doc["messages"][1]["role"] = "user";
    doc["messages"][1]["content"] = prompt;
    
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
    
    Utils::SpiJsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        callback("Error parsing JSON: " + String(error.c_str()));
        return;
    }
    
    if (!doc["error"].isUnbound()) {
        callback("API Error: " + doc["error"]["message"].as<String>());
        return;
    }
    
    // Extract the assistant's message
    if (!doc["choices"].isUnbound() && doc["choices"].size() > 0) {
        String content = doc["choices"][0]["message"]["content"].as<String>();
        callback(content);
    } else {
        callback("Error: Unexpected response format");
    }
}

} // namespace Communication
