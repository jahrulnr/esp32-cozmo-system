#include "Automation.h"
#include "setup/setup.h"
#include <tasks/register.h>

#define BEHAVIOR_PROMPT "STRICT INSTRUCTIONS: You are a robot behavior generator. You MUST follow these rules exactly:\n"\
        "1. ONLY generate robot behaviors - NO explanations, comments, or other text\n" \
        "2. Generate EXACTLY 10 behaviors, one per line\n" \
        "3. MANDATORY format: [ACTION=time][ACTION2=time] *Robot vocalization*\n" \
        "4. ONLY use these EXACT action names (case-sensitive):\n" \
        "   Movement: MOVE_FORWARD, MOVE_BACKWARD, TURN_LEFT, TURN_RIGHT, STOP\n" \
        "   Looking: LOOK_LEFT, LOOK_RIGHT, LOOK_TOP, LOOK_BOTTOM, LOOK_FRONT, LOOK_AROUND\n" \
        "   Head: HEAD_UP, HEAD_DOWN, HEAD_CENTER, HEAD_POSITION\n" \
        "   Hand: HAND_UP, HAND_DOWN, HAND_CENTER, HAND_POSITION\n" \
        "   Motors: MOTOR_LEFT, MOTOR_RIGHT\n" \
        "   Faces: FACE_HAPPY, FACE_SAD, FACE_ANGRY, FACE_SURPRISED, FACE_WORRIED, " \
        "FACE_SKEPTIC, FACE_FOCUSED, FACE_UNIMPRESSED, FACE_FRUSTRATED, " \
        "FACE_SQUINT, FACE_AWE, FACE_GLEE, FACE_FURIOUS, FACE_SUSPICIOUS, FACE_SCARED, FACE_SLEEPY, FACE_NORMAL\n\n" \
        "5. Time format: ONLY use 'ms' or 's' (e.g., 500ms, 2s)\n" \
        "6. Position values: HEAD_POSITION/HAND_POSITION angles 0-180 only\n" \
        "7. Motor values: MOTOR_LEFT/MOTOR_RIGHT speeds 0-100 only\n" \
        "8. Vocalization: MUST be inside *asterisks* and be SHORT robot-like phrases\n" \
        "9. NO numbering (1., 2., etc.), NO bullet points, NO headers\n" \
        "10. NO explanatory text before or after behaviors\n" \
        "11. Each line MUST start with '[' and contain at least one action\n" \
        "12. Make behaviors different from existing examples below\n\n" \
        "FORBIDDEN: Do NOT include any text that doesn't match the exact format above.\n" \
        "REQUIRED OUTPUT: Start immediately with behaviors, no introduction.\n\n" \
        "Examples of current behaviors:\n"

namespace Automation {

// Constructor
Automation::Automation(Utils::FileManager* fileManager, 
                       Utils::CommandMapper* commandMapper,
                       Utils::Logger* logger)
    : _fileManager(fileManager)
    , _commandMapper(commandMapper)
    , _logger(logger)
    , _taskHandle(NULL)
    , _enabled(AUTOMATION_ENABLED)
    , _lastManualControlTime(0)
    , _behaviorIndex(0)
    , _timer(0)
    , _randomBehaviorOrder(false) // Add this line
    , _behaviorPrompt(BEHAVIOR_PROMPT)
{
    // Create a mutex for thread-safe access to behaviors
    _behaviorsMutex = xSemaphoreCreateMutex();

    
}

// Destructor
Automation::~Automation() {
    stop();
}

// Start automation task
void Automation::start() {
    if (_taskHandle != NULL) {
        return; // Task already running
    }
    
    // Load behaviors before starting task
    loadTemplateBehaviors();
    
    // Create the task
    xTaskCreate(
        taskFunction,    // Function that implements the task
        "automation",    // Task name
        4096 * 2,           // Stack size in words
        this,           // Parameter passed to the task
        0,              // Priority
        &_taskHandle   // Task handle
    );

    if (_fileManager && !_fileManager->exists("/config/templates_update.txt")) {
        xTaskCreatePinnedToCore(
            [](void * param){
                vTaskDelay(pdMS_TO_TICKS(20099));
                if (WiFi.isConnected()) {
                    Automation* automation = static_cast<Automation*>(param);
                    automation->fetchAndAddNewBehaviors();                
                }
                vTaskDelete(NULL);
            },    // Function that implements the task
            "automationUpdate",    // Task name
            8192,           // Stack size in words
            this,           // Parameter passed to the task
            0,              // Priority
            NULL,
            0
        );
    }
    
    if (_logger) {
        _logger->info("Automation task started");
    }
}

// Stop automation task
void Automation::stop() {
    if (_taskHandle != NULL) {
        vTaskDelete(_taskHandle);
        _taskHandle = NULL;
        
        if (_logger) {
            _logger->info("Automation task stopped");
        }
    }
}

// Update the time of last manual control
void Automation::updateManualControlTime() {
    _lastManualControlTime = millis();
}

// Get automation enabled status
bool Automation::isEnabled() const {
    return _enabled;
}

// Set automation enabled status
void Automation::setEnabled(bool enabled) {
    _enabled = enabled;
}

// Add getter/setter for random order
bool Automation::isRandomBehaviorOrder() const {
    return _randomBehaviorOrder;
}

void Automation::setRandomBehaviorOrder(bool randomOrder) {
    _randomBehaviorOrder = randomOrder;
    if (_logger) {
        _logger->info("Automation behavior order set to %s", randomOrder ? "random" : "sequential");
    }
}

// Static task function that receives the class instance
void Automation::taskFunction(void* parameter) {
    // Convert parameter to Automation instance
    Automation* automation = static_cast<Automation*>(parameter);
    long updateTimer = millis();
    long updateInterval = pdMS_TO_TICKS(60000 * 30);
    long servoTimer = updateTimer;
    long servoInterval = pdMS_TO_TICKS(10000);
    bool inprogress = false;
    bool paused = false;
    vTaskDelay(pdMS_TO_TICKS(10000));
    
    // Run automation forever
    TickType_t lastWakeTime = xTaskGetTickCount();
    while (true) {
        // Check at regular intervals
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(AUTOMATION_CHECK_INTERVAL));

        if (notification->has(NOTIFICATION_AUTOMATION)) {
            const char* notif = (const char*)notification->consume(NOTIFICATION_AUTOMATION);
            if (notif == EVENT_AUTOMATION_PAUSE){
                paused = true;
                automation->updateManualControlTime();
            }
            else if (notif == EVENT_AUTOMATION_RESUME){
                paused = false;
                automation->updateManualControlTime();
                vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(AUTOMATION_CHECK_INTERVAL * 5));
            }
        }

        if (inprogress || paused){
            if(paused) inprogress = false;
            vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(AUTOMATION_CHECK_INTERVAL / 2));
            continue;
        }

        inprogress = true;
        // get servo position
        if (servos) {
            long lastServoUpdate = servoTimer;
            bool restoreServo = false;
            if (servos->getHead() != DEFAULT_HEAD_ANGLE && millis() - lastServoUpdate > servoInterval) {
                servos->setHead(DEFAULT_HEAD_ANGLE);
                servoTimer = millis();
                restoreServo = true;
            }
            if (servos->getHand() != DEFAULT_HAND_ANGLE && millis() - lastServoUpdate > servoInterval) {
                servos->setHand(DEFAULT_HAND_ANGLE);
                servoTimer = millis();
                restoreServo = true;
            }

            if (restoreServo) {
                vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(300));
            }
        }

        // Check if automation is enabled and no manual control for a while
        if (automation->_enabled && 
            (millis() - automation->_lastManualControlTime > AUTOMATION_INACTIVITY_TIMEOUT)) {
            
            if (xSemaphoreTake(automation->_behaviorsMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                if (!automation->_templateBehaviors.empty()) {
                    size_t behaviorIdx = 0;
                    if (automation->_randomBehaviorOrder) {
                        behaviorIdx = random(0, automation->_templateBehaviors.size());
                    } else {
                        behaviorIdx = automation->_behaviorIndex;
                        automation->_behaviorIndex = (automation->_behaviorIndex + 1) % 
                                                    automation->_templateBehaviors.size();
                    }
                    Utils::Sstring behavior = automation->_templateBehaviors[behaviorIdx];

                    xSemaphoreGive(automation->_behaviorsMutex);
                    automation->executeBehavior(behavior);
                    automation->_lastManualControlTime = millis();
                    int randomDelay = random(5000, 10000);
                    
                    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(randomDelay));
                } else {
                    xSemaphoreGive(automation->_behaviorsMutex);
                }
            }
        }

        if (millis() - updateTimer > updateInterval) {
            xTaskCreate([](void * param){
            Automation* automation = static_cast<Automation*>(param);
                automation->fetchAndAddNewBehaviors();
                vTaskDelete(NULL);
            }, "UpdateTemplate", 20 * 1024, automation, 1, NULL);
            updateTimer = millis();
        }
        
        inprogress = false;
    }
}

// Load template behaviors from file
void Automation::loadTemplateBehaviors() {
    // Take the mutex to safely modify the behaviors list
    if (xSemaphoreTake(_behaviorsMutex, portMAX_DELAY) == pdTRUE) {
        _templateBehaviors.clear();
        
        // Load template behaviors from file
        Utils::Sstring templateDefault = "";
        if (_fileManager && _fileManager->exists("/config/templates.txt")) {
            templateDefault = _fileManager->readFile("/config/templates.txt");
        }

        if (_fileManager && _fileManager->exists("/config/templates_update.txt")) {
            templateDefault += _fileManager->readFile("/config/templates_update.txt");
        }
        
        // Split template behaviors into an array
        int startPos = 0;
        int nextPos = 0;
        while ((nextPos = templateDefault.indexOf('\n', startPos)) != -1) {
            Utils::Sstring behavior = templateDefault.toString().substring(startPos, nextPos);
            behavior.trim();
            if (behavior.length() > 0) {
                _templateBehaviors.push_back(behavior);
            }
            startPos = nextPos + 1;
        }
        
        // Add the remaining text as the last behavior if it exists
        Utils::Sstring lastBehavior = templateDefault.substring(startPos);
        if (lastBehavior.length() > 0) {
            _templateBehaviors.push_back(lastBehavior);
        }
        
        // Give the mutex back
        xSemaphoreGive(_behaviorsMutex);
        
        if (_logger) {
            _logger->info("Loaded %d template behaviors", _templateBehaviors.size());
        }
    }
}

// Execute a specific behavior
void Automation::executeBehavior(const Utils::Sstring& behavior) {
    if (_commandMapper) {
        if (_logger) {
            _logger->debug("Executing automation behavior: " + behavior.toString());
        }
        
        // Check if there's a vocalization message (text enclosed in asterisks)
        int startVoice = behavior.toString().indexOf('*');
        int endVoice = behavior.toString().lastIndexOf('*');
        Utils::Sstring voiceMessage = "";
        
        if (startVoice >= 0 && endVoice > startVoice) {
            voiceMessage = behavior.toString().substring(startVoice + 1, endVoice);
            sayText(voiceMessage.c_str());
            delay(2000);
        }
        
        // Display the message on the screen if available
        // The screen class already handles internal mutex locking in its mutexX methods
        if (::screen && !voiceMessage.isEmpty()) {
            _commandMapper->executeCommandString(behavior.toString());
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            // Just execute the commands without showing the message
            _commandMapper->executeCommandString(behavior.toString());
        }
        
        if (_logger) {
            _logger->debug("Executed automation behavior commands");
        }
    }
}

// Add a new behavior to the templates
bool Automation::addNewBehavior(const Utils::Sstring& behavior) {
    if (behavior.isEmpty()) {
        return false;
    }
    
    // Take the mutex to safely modify the behaviors list
    if (xSemaphoreTake(_behaviorsMutex, portMAX_DELAY) == pdTRUE) {
        // Add the new behavior to our list
        _templateBehaviors.push_back(behavior);
        
        // Save to file
        bool saveResult = saveBehaviorsToFile();
        
        // Give the mutex back
        xSemaphoreGive(_behaviorsMutex);
        
        if (_logger) {
            if (saveResult) {
                _logger->info("New behavior added: %s", behavior.c_str());
            } else {
                _logger->error("Failed to save new behavior to file");
            }
        }
        
        return saveResult;
    }
    
    return false;
}

// Fetch new behaviors from GPT and add them
bool Automation::fetchAndAddNewBehaviors(const Utils::Sstring& prompt) {
    if (!::gptAdapter) {
        if (_logger) {
            _logger->error("GPT adapter not available for generating behaviors");
        }
        return false;
    }
    
    if (_logger) {
        _logger->info("Requesting new behaviors from GPT with prompt: %s", prompt.c_str());
    }
    
    // Create a semaphore for synchronizing with the callback
    SemaphoreHandle_t doneSemaphore = xSemaphoreCreateBinary();
    bool success = false;
    
    // Build a list of existing behaviors to avoid duplicates
    Utils::Sstring existingBehaviorsList = "";
    int exampleCount = 0;
    
    // Take the mutex to safely access the behaviors list
    if (xSemaphoreTake(_behaviorsMutex, portMAX_DELAY) == pdTRUE) {
        // Get up to 5 random examples from existing behaviors
        if (!_templateBehaviors.empty()) {
            // Create a list of indices
            std::vector<int> indices(_templateBehaviors.size());
            for (size_t i = 0; i < indices.size(); ++i) {
                indices[i] = static_cast<int>(i);
            }
            
            // Shuffle to get random behaviors
            for (size_t i = 0; i < indices.size(); ++i) {
                size_t j = random(0, indices.size());
                std::swap(indices[i], indices[j]);
            }
            
            // Get up to 5 examples
            for (size_t i = 0; i < std::min(static_cast<size_t>(5), _templateBehaviors.size()); ++i) {
                existingBehaviorsList += "Example ";
                existingBehaviorsList += Utils::Sstring(exampleCount + 1) + ": " + 
                                         _templateBehaviors[indices[i]].toString() + "\n";
                exampleCount++;
            }
        }
        
        // Release the mutex
        xSemaphoreGive(_behaviorsMutex);
    }
    
    // Additional command to guide GPT in generating appropriate behaviors
    Utils::Sstring additionalCommand = _behaviorPrompt;
    additionalCommand += existingBehaviorsList;
        
    // Store a pointer to this for use in the lambda
    Automation* self = this;
        
    // Send the prompt to GPT using sendPromptWithCustomSystem for better memory management
    ::gptAdapter->sendPromptWithCustomSystem(prompt, additionalCommand, 
        [self, doneSemaphore, &success](const Utils::Sstring& response) {
        // Process the response from GPT
        int addedCount = 0;
        
        if (self->_logger) {
            self->_logger->debug("GPT Response received");
        }
        
        // Split the response by newlines
        int startPos = 0;
        int endPos = 0;
        
        while ((endPos = response.indexOf('\n', startPos)) != -1) {
            Utils::Sstring behavior = response.substring(startPos, endPos);
            behavior.trim();
            if (!behavior.isEmpty() && behavior.indexOf('[') >= 0) {
                // This looks like a valid behavior template
                if (self->addNewBehavior(behavior)) {
                    addedCount++;
                }
            }
            startPos = endPos + 1;
        }
        
        // Check for one last behavior after the last newline
        Utils::Sstring lastBehavior = response.substring(startPos);
        lastBehavior.trim();
        if (!lastBehavior.isEmpty() && lastBehavior.indexOf('[') >= 0) {
            if (self->addNewBehavior(lastBehavior)) {
                addedCount++;
            }
        }
        
        // Set success flag if we added at least one behavior
        success = (addedCount > 0);
        
        if (self->_logger) {
            if (success) {
                self->_logger->info("Added %d new behaviors from GPT", addedCount);
            } else {
                self->_logger->warning("No valid behaviors found in GPT response");
            }
        }
        
        // Signal that we're done processing
        xSemaphoreGive(doneSemaphore);
    });
    
    // Wait for the callback to complete (with a timeout)
    if (xSemaphoreTake(doneSemaphore, pdMS_TO_TICKS(30000)) != pdTRUE) {
        // Timed out waiting for GPT response
        if (_logger) {
            _logger->error("Timed out waiting for GPT to generate behaviors");
        }
        vSemaphoreDelete(doneSemaphore);
        return false;
    }
    
    // Clean up
    vSemaphoreDelete(doneSemaphore);
    return success;
}

// Save the current behaviors to file
bool Automation::saveBehaviorsToFile() {
    if (!_fileManager) {
        return false;
    }
    
    // First, separate default behaviors (from templates.txt) from user-added behaviors
    std::vector<Utils::Sstring> defaultBehaviors;
    std::vector<Utils::Sstring> userBehaviors;
    
    // Load default behaviors to identify them
    Utils::Sstring defaultTemplates = "";
    if (_fileManager && _fileManager->exists("/config/templates.txt")) {
        defaultTemplates = _fileManager->readFile("/config/templates.txt");
    }
    
    // Parse default behaviors
    std::vector<Utils::Sstring> defaultTemplatesList;
    int startPos = 0;
    int nextPos = 0;
    while ((nextPos = defaultTemplates.indexOf('\n', startPos)) != -1) {
        Utils::Sstring behavior = defaultTemplates.toString().substring(startPos, nextPos);
        behavior.trim();
        if (behavior.length() > 0) {
            defaultTemplatesList.push_back(behavior);
        }
        startPos = nextPos + 1;
    }
    
    Utils::Sstring lastDefault = defaultTemplates.toString().substring(startPos);
    lastDefault.trim();
    if (lastDefault.length() > 0) {
        defaultTemplatesList.push_back(lastDefault);
    }
    
    // Separate user-added behaviors from default behaviors
    for (const auto& behavior : _templateBehaviors) {
        // Check if this behavior is in the default list
        bool isDefault = false;
        for (const auto& defaultBehavior : defaultTemplatesList) {
            if (behavior.equals(defaultBehavior)) {
                isDefault = true;
                break;
            }
        }
        
        if (isDefault) {
            defaultBehaviors.push_back(behavior);
        } else {
            userBehaviors.push_back(behavior.toString());
        }
    }
    
    // Apply LRU (Least Recently Used) logic to prevent memory issues
    // Prefer keeping user-added behaviors, only trim if absolutely necessary
    size_t totalBehaviors = defaultBehaviors.size() + userBehaviors.size();
    if (totalBehaviors > AUTOMATION_MAX_BEHAVIORS) {
        // Calculate how many behaviors to remove
        size_t toRemove = totalBehaviors - AUTOMATION_MAX_BEHAVIORS;
        
        if (_logger) {
            _logger->warning("Too many behaviors (%d), removing %d oldest behaviors",
                totalBehaviors, toRemove);
        }
        
        // First try to remove from default behaviors (they can be reloaded from file)
        if (toRemove <= defaultBehaviors.size()) {
            defaultBehaviors.erase(defaultBehaviors.begin(), defaultBehaviors.begin() + toRemove);
            toRemove = 0;
        } else {
            toRemove -= defaultBehaviors.size();
            defaultBehaviors.clear(); // Remove all default behaviors
            
            // If we still need to remove more, start removing user behaviors
            if (toRemove > 0 && !userBehaviors.empty()) {
                userBehaviors.erase(userBehaviors.begin(), userBehaviors.begin() + std::min(toRemove, userBehaviors.size()));
            }
        }
        
        // Rebuild the template behaviors list
        _templateBehaviors.clear();
        _templateBehaviors.insert(_templateBehaviors.end(), defaultBehaviors.begin(), defaultBehaviors.end());
        _templateBehaviors.insert(_templateBehaviors.end(), userBehaviors.begin(), userBehaviors.end());
        
        // Adjust behaviorIndex if needed to prevent out-of-bounds access
        if (_behaviorIndex >= _templateBehaviors.size() && !_templateBehaviors.empty()) {
            _behaviorIndex = 0;
        }
    }
    
    // Build the content to save, validating behavior size to avoid memory issues
    Utils::Sstring content = "";
    size_t validBehaviors = 0;
    
    for (const auto& behavior : userBehaviors) {
        // Skip behaviors that are too large
        if (behavior.length() > AUTOMATION_MAX_BEHAVIOR_LENGTH) {
            if (_logger) {
                _logger->warning("Skipping oversized behavior: " + behavior.toString().substring(0, 30) + "...");
            }
            continue;
        }
        
        content += behavior.toString() + "\n";
        validBehaviors++;
    }
    
    // Check if content is too large (rough estimate, allowing ~10KB max)
    if (content.length() > 10240) {
        if (_logger) {
            _logger->error("Behavior content too large (%d bytes), truncating to prevent memory issues",
                content.length());
        }
        content = content.substring(0, 10240);
    }
    
    // Save to file (completely replace the templates_update.txt to ensure persistence)
    bool success = _fileManager->writeFile("/config/templates_update.txt", content.c_str());
    
    // Log the result
    if (_logger) {
        if (success) {
            _logger->info("Successfully saved %d user behaviors to templates_update.txt", validBehaviors);
        } else {
            _logger->error("Failed to save behaviors to templates_update.txt");
        }
    }
    
    return success;
}

} // namespace Automation
