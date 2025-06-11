#include "Automation.h"
#include "app.h"

namespace Automation {

// Constructor
Automation::Automation(Utils::FileManager* fileManager, 
                       Utils::CommandMapper* commandMapper,
                       Utils::Logger* logger,
                       Communication::WebSocketHandler* webSocket)
    : _fileManager(fileManager)
    , _commandMapper(commandMapper)
    , _logger(logger)
    , _webSocket(webSocket)
    , _taskHandle(NULL)
    , _enabled(AUTOMATION_ENABLED)
    , _lastManualControlTime(0)
    , _behaviorIndex(0)
    , _timer(0)
    , _randomBehaviorOrder(false) // Add this line
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
    xTaskCreatePinnedToCore(
        taskFunction,    // Function that implements the task
        "automation",    // Task name
        8192,           // Stack size in words
        this,           // Parameter passed to the task
        1,              // Priority
        &_taskHandle,   // Task handle
        0
    );

    if (_fileManager && !_fileManager->exists("/config/templates_update.txt")) {
        xTaskCreate(
            [](void * param){
                if (WiFi.isConnected()) {
                    vTaskDelay(pdMS_TO_TICKS(11000));
                    Automation* automation = static_cast<Automation*>(param);
                    automation->fetchAndAddNewBehaviors();                
                }
                vTaskDelete(NULL);
            },    // Function that implements the task
            "automationUpdate",    // Task name
            8192,           // Stack size in words
            this,           // Parameter passed to the task
            1,              // Priority
            NULL
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
    
    // Send status to all connected clients
    if (_webSocket) {
        Utils::SpiJsonDocument statusDoc;
        statusDoc["enabled"] = _enabled;
        _webSocket->sendJsonMessage(-1, "automation_status", statusDoc);
    }
    
    if (_logger) {
        _logger->info(String("Automation ") + (enabled ? "enabled" : "disabled"));
    }
}

// Add getter/setter for random order
bool Automation::isRandomBehaviorOrder() const {
    return _randomBehaviorOrder;
}

void Automation::setRandomBehaviorOrder(bool randomOrder) {
    _randomBehaviorOrder = randomOrder;
    if (_logger) {
        _logger->info(String("Automation behavior order set to ") + (randomOrder ? "random" : "sequential"));
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
    
    // Run automation forever
    while (true) {
        if (inprogress){
            vTaskDelay(pdMS_TO_TICKS(AUTOMATION_CHECK_INTERVAL / 2));
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
                vTaskDelay(pdMS_TO_TICKS(300));
            }
        }

        // Check if automation is enabled and no manual control for a while
        if (automation->_enabled && 
            (millis() - automation->_lastManualControlTime > AUTOMATION_INACTIVITY_TIMEOUT)) {
            
            if (xSemaphoreTake(automation->_behaviorsMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                if (!automation->_templateBehaviors.empty()) {
                    // --- CHANGED: choose behavior index based on random/sequential ---
                    size_t behaviorIdx = 0;
                    if (automation->_randomBehaviorOrder) {
                        behaviorIdx = random(0, automation->_templateBehaviors.size());
                    } else {
                        behaviorIdx = automation->_behaviorIndex;
                        automation->_behaviorIndex = (automation->_behaviorIndex + 1) % 
                                                    automation->_templateBehaviors.size();
                    }
                    Utils::Sstring behavior = automation->_templateBehaviors[behaviorIdx];
                    // --- END CHANGE ---

                    xSemaphoreGive(automation->_behaviorsMutex);
                    automation->executeBehavior(behavior);
                    automation->_lastManualControlTime = millis();
                    int randomDelay = random(5000, 10000);
                    vTaskDelay(pdMS_TO_TICKS(randomDelay));
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
        // Check at regular intervals
        vTaskDelay(pdMS_TO_TICKS(AUTOMATION_CHECK_INTERVAL));
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
            Utils::Sstring behavior = templateDefault.substring(startPos, nextPos).trim();
            if (behavior.length() > 0) {
                _templateBehaviors.push_back(behavior);
            }
            startPos = nextPos + 1;
        }
        
        // Add the remaining text as the last behavior if it exists
        Utils::Sstring lastBehavior = templateDefault.substring(startPos).trim();
        if (lastBehavior.length() > 0) {
            _templateBehaviors.push_back(lastBehavior);
        }
        
        // Give the mutex back
        xSemaphoreGive(_behaviorsMutex);
        
        if (_logger) {
            _logger->info("Loaded " + String(_templateBehaviors.size()) + " template behaviors");
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
        String voiceMessage = "";
        
        if (startVoice >= 0 && endVoice > startVoice) {
            voiceMessage = behavior.toString().substring(startVoice + 1, endVoice);
        }
        
        // Display the message on the screen if available
        // The screen class already handles internal mutex locking in its mutexX methods
        if (::screen && !voiceMessage.isEmpty()) {
            // Save the current face to restore after showing the message
            Face* face = ::screen->getFace();
            
            // Display the vocalization message
            ::screen->mutexClear();
            ::screen->drawCenteredText(30, voiceMessage);
            ::screen->mutexUpdate();
            
            // Extract and execute commands
            int commandCount = _commandMapper->executeCommandString(behavior.toString());
            
            // Give the message some time to be visible, if it's not a long running action
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            // Just execute the commands without showing the message
            int commandCount = _commandMapper->executeCommandString(behavior.toString());
        }
        
        if (_logger) {
            _logger->debug("Executed automation behavior commands");
        }
    }
}

// Add a new behavior to the templates
bool Automation::addNewBehavior(const String& behavior) {
    if (behavior.isEmpty()) {
        return false;
    }
    
    // Take the mutex to safely modify the behaviors list
    if (xSemaphoreTake(_behaviorsMutex, portMAX_DELAY) == pdTRUE) {
        // Add the new behavior to our list
        _templateBehaviors.push_back(Utils::Sstring(behavior));
        
        // Save to file
        bool saveResult = saveBehaviorsToFile();
        
        // Give the mutex back
        xSemaphoreGive(_behaviorsMutex);
        
        if (_logger) {
            if (saveResult) {
                _logger->info("New behavior added: " + behavior);
            } else {
                _logger->error("Failed to save new behavior to file");
            }
        }
        
        return saveResult;
    }
    
    return false;
}

// Fetch new behaviors from GPT and add them
bool Automation::fetchAndAddNewBehaviors(const String& prompt) {
    if (!::gptAdapter) {
        if (_logger) {
            _logger->error("GPT adapter not available for generating behaviors");
        }
        return false;
    }
    
    if (_logger) {
        _logger->info("Requesting new behaviors from GPT with prompt: " + prompt);
    }
    
    // Create a semaphore for synchronizing with the callback
    SemaphoreHandle_t doneSemaphore = xSemaphoreCreateBinary();
    bool success = false;
    
    // Build a list of existing behaviors to avoid duplicates
    String existingBehaviorsList = "";
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
                existingBehaviorsList += "Example " + String(exampleCount + 1) + ": " + 
                                         _templateBehaviors[indices[i]].toString() + "\n";
                exampleCount++;
            }
        }
        
        // Release the mutex
        xSemaphoreGive(_behaviorsMutex);
    }
    
    // Additional command to guide GPT in generating appropriate behaviors
    String additionalCommand = 
        "Generate 8 new robot behaviors in the exact format of existing templates. "
        "Each behavior should be on a new line and use only this format: "
        "[ACTION=time][ACTION2=time] *Robot vocalization*\n\n"
        "Valid actions are:\n"
        "Movement: MOVE_FORWARD, MOVE_BACKWARD, TURN_LEFT, TURN_RIGHT, STOP\n"
        "Looking: LOOK_LEFT, LOOK_RIGHT, LOOK_TOP, LOOK_BOTTOM, LOOK_FRONT, LOOK_AROUND\n"
        "Head control: HEAD_UP, HEAD_DOWN, HEAD_CENTER, HEAD_POSITION\n"
        "Hand control: HAND_UP, HAND_DOWN, HAND_CENTER, HAND_POSITION\n"
        "Advanced movement: MOTOR_LEFT, MOTOR_RIGHT, DANCE_SPIN\n"
        "Face expressions: FACE_HAPPY, FACE_SAD, FACE_ANGRY, FACE_SURPRISED, FACE_WORRIED, "
        "FACE_SKEPTIC, FACE_FOCUSED, FACE_UNIMPRESSED, FACE_FRUSTRATED, "
        "FACE_SQUINT, FACE_AWE, FACE_GLEE, FACE_FURIOUS, FACE_SUSPICIOUS, FACE_SCARED, FACE_SLEEPY, FACE_NORMAL\n\n"
        "Times should be specified in ms (500ms) or s (2s).\n"
        "Servo positions should be specified as angles 0-180: [HEAD_POSITION=90]\n"
        "Motor speeds should be specified as values 0-100: [MOTOR_LEFT=75]\n"
        "Make behaviors unique and different from existing ones.\n"
        "Each behavior should represent a cohesive action with matching facial expression and vocalization.\n"
        "Create a mix of simple and complex behaviors.\n"
        "Do not include any explanations, numbering, or extra text.\n\n"
        "Here are some examples of current behaviors:\n" + existingBehaviorsList;
        
    // Store a pointer to this for use in the lambda
    Automation* self = this;
        
    // Send the prompt to GPT using sendPromptWithCustomSystem for better memory management
    ::gptAdapter->sendPromptWithCustomSystem(prompt, additionalCommand, 
        [self, doneSemaphore, &success](const String& response) {
        // Process the response from GPT
        int addedCount = 0;
        
        if (self->_logger) {
            self->_logger->debug("GPT Response received");
        }
        
        // Split the response by newlines
        int startPos = 0;
        int endPos = 0;
        
        while ((endPos = response.indexOf('\n', startPos)) != -1) {
            String behavior = response.substring(startPos, endPos);
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
        String lastBehavior = response.substring(startPos);
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
                self->_logger->info("Added " + String(addedCount) + " new behaviors from GPT");
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
        Utils::Sstring behavior = defaultTemplates.substring(startPos, nextPos).trim();
        if (behavior.length() > 0) {
            defaultTemplatesList.push_back(behavior);
        }
        startPos = nextPos + 1;
    }
    
    Utils::Sstring lastDefault = defaultTemplates.substring(startPos).trim();
    if (lastDefault.length() > 0) {
        defaultTemplatesList.push_back(lastDefault);
    }
    
    // Separate user-added behaviors from default behaviors
    for (const auto& behavior : _templateBehaviors) {
        // Check if this behavior is in the default list
        bool isDefault = false;
        for (const auto& defaultBehavior : defaultTemplatesList) {
            if (behavior.toString() == defaultBehavior.toString()) {
                isDefault = true;
                break;
            }
        }
        
        if (isDefault) {
            defaultBehaviors.push_back(behavior);
        } else {
            userBehaviors.push_back(behavior);
        }
    }
    
    // Apply LRU (Least Recently Used) logic to prevent memory issues
    // Prefer keeping user-added behaviors, only trim if absolutely necessary
    size_t totalBehaviors = defaultBehaviors.size() + userBehaviors.size();
    if (totalBehaviors > AUTOMATION_MAX_BEHAVIORS) {
        // Calculate how many behaviors to remove
        size_t toRemove = totalBehaviors - AUTOMATION_MAX_BEHAVIORS;
        
        if (_logger) {
            _logger->warning("Too many behaviors (" + String(totalBehaviors) + 
                             "), removing " + String(toRemove) + " oldest behaviors");
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
    String content = "";
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
            _logger->error("Behavior content too large (" + String(content.length()) + 
                           " bytes), truncating to prevent memory issues");
        }
        content = content.substring(0, 10240);
    }
    
    // Save to file (completely replace the templates_update.txt to ensure persistence)
    bool success = _fileManager->writeFile("/config/templates_update.txt", content);
    
    // Log the result
    if (_logger) {
        if (success) {
            _logger->info("Successfully saved " + String(validBehaviors) + " user behaviors to templates_update.txt");
        } else {
            _logger->error("Failed to save behaviors to templates_update.txt");
        }
    }
    
    return success;
}

} // namespace Automation
