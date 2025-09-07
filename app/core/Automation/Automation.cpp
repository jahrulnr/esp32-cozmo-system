#include "Automation.h"
#include "setup/setup.h"
#include <tasks/register.h>
#include <SendTask.h>

#define BEHAVIOR_PROMPT "⚠️ CRITICAL: You are a robot behavior generator. ANY deviation from these rules will cause system failure!\n\n"\
        "🔒 STRICT VALIDATION RULES:\n"\
        "1. OUTPUT EXACTLY 50 lines of robot behaviors - NOTHING ELSE\n"\
        "2. Each line format: [ACTION=time][ACTION2=time] *Complete vocalization*\n"\
        "3. COPY THESE EXACT COMMANDS (no variations allowed):\n"\
        "   ✅ VALID: MOVE_FORWARD, MOVE_BACKWARD, TURN_LEFT, TURN_RIGHT, STOP\n"\
        "   ✅ VALID: LOOK_LEFT, LOOK_RIGHT, LOOK_TOP, LOOK_BOTTOM, LOOK_FRONT, LOOK_AROUND\n"\
        "   ✅ VALID: HEAD_UP, HEAD_DOWN, HEAD_CENTER, HEAD_POSITION\n"\
        "   ✅ VALID: HAND_UP, HAND_DOWN, HAND_CENTER, HAND_POSITION\n"\
        "   ✅ VALID: MOTOR_LEFT, MOTOR_RIGHT\n"\
        "   ✅ VALID: FACE_HAPPY, FACE_SAD, FACE_ANGRY, FACE_SURPRISED, FACE_WORRIED, "\
        "FACE_SURPRISED, FACE_FOCUSED, FACE_UNIMPRESSED, FACE_FRUSTRATED, "\
        "FACE_SQUINT, FACE_AWE, FACE_GLEE, FACE_FURIOUS, FACE_SUSPICIOUS, FACE_SCARED, FACE_SLEEPY, FACE_NORMAL\n\n"\
        "❌ INVALID EXAMPLES (DO NOT USE):\n"\
        "   HANDS_DOWN (wrong! use HAND_DOWN), HEADS_UP (wrong! use HEAD_UP)\n"\
        "   [HEAD_POSITION=90=500ms] (wrong! use [HEAD_POSITION=90][FACE_HAPPY=500ms])\n"\
        "   *Incomplete message (wrong! must close with *)\n\n"\
        "✅ VALID SYNTAX EXAMPLES FROM TEMPLATES:\n"\
        "   [LOOK_LEFT=1s][FACE_SURPRISED=2s] *Hmm, what's that?*\n"\
        "   [MOVE_FORWARD=2s][FACE_HAPPY=1s] *Let's go explore!*\n"\
        "   [TURN_LEFT=1s][TURN_RIGHT=1s][FACE_GLEE=2s] *Spinning around!*\n"\
        "   [HEAD_UP=1s][LOOK_TOP=2s][FACE_SURPRISED=1s] *Wow, look up there!*\n"\
        "   [HAND_UP=2s][FACE_HAPPY=1s][BLINK=1s] *Hello there!*\n"\
        "   [MOVE_BACKWARD=1s][FACE_WORRIED=2s] *Oops, better back up!*\n"\
        "   [LOOK_AROUND=3s][FACE_FOCUSED=2s] *Scanning the area*\n"\
        "   [HEAD_DOWN=2s][FACE_SLEEPY=3s] *Time for a little nap*\n"\
        "   [TURN_LEFT=3s][FACE_HAPPY=2s] *Dancing to the left!*\n"\
        "   [MOVE_FORWARD=1s][TURN_LEFT=1s][MOVE_BACKWARD=1s][TURN_RIGHT=1s][FACE_HAPPY=2s] *Square dance time!*\n\n"\
        "🎯 REQUIREMENTS:\n"\
        "• Time: ONLY 'ms' or 's' (500ms, 2s)\n"\
        "• Angles: 0-180 for HEAD_POSITION/HAND_POSITION\n"\
        "• Motor speeds: 0-100 for MOTOR_LEFT/MOTOR_RIGHT\n"\
        "• Vocalization: MUST start and end with * (asterisk)\n"\
        "• NO explanations, NO numbering, NO extra text\n"\
        "• Each line MUST start with '['\n\n"\
        "🚫 IMMEDIATE FAILURE if you include ANY:\n"\
        "- Text before/after the 10 behaviors\n"\
        "- Wrong command names (like HANDS_DOWN)\n"\
        "- Malformed syntax (like =90=500ms)\n"\
        "- Incomplete vocalizations (missing closing *)\n\n"\
        "START OUTPUT NOW - 50 behaviors only\n"

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
    , _templatesFile("/config/templates.txt")
    , _templatesUpdateFile("/config/templates_update.txt")
{
    // Create a mutex for thread-safe access to behaviors
    _behaviorsMutex = xSemaphoreCreateMutex();

    
}

// Destructor
Automation::~Automation() {
    stop();
}

// Start automation task
void Automation::start(bool core) {
    if (_taskHandle != NULL) {
        return; // Task already running
    }
    
    // Load behaviors before starting task
    loadTemplateBehaviors();
    
    // Create the main automation task using SendTask
    String taskId = SendTask::createLoopTaskOnCore(
        taskFunction,               // Function that implements the task
        "Automation",               // Task name
        4096 * 2,                   // Stack size
        0,                          // Priority
        core,                       // Core ID
        "Main automation behavior task", // Description
        this                        // Parameter passed to the task
    );

    if (!taskId.isEmpty()) {
        auto taskInfo = SendTask::getTaskInfo(taskId);
        _taskHandle = taskInfo.handle;
        
        if (_logger) {
            _logger->info("Automation task created with ID: %s", taskId.c_str());
        }
    } else {
        if (_logger) {
            _logger->error("Failed to create automation task");
        }
        return;
    }

    // Create template update task if needed
    if (_fileManager && !_fileManager->exists(_templatesUpdateFile)) {
        String updateTaskId = SendTask::createTaskOnCore(
            [this]() {
                vTaskDelay(pdMS_TO_TICKS(20099));
                if (WiFi.isConnected()) {
                    this->fetchAndAddNewBehaviors();                
                }
            },
            "AutomationUpdate",         // Task name
            8192,                       // Stack size
            0,                          // Priority
            0,                          // Core ID
            "Automation template update task" // Description
        );
        
        if (_logger) {
            if (!updateTaskId.isEmpty()) {
                _logger->info("Automation update task created with ID: %s", updateTaskId.c_str());
            } else {
                _logger->error("Failed to create automation update task");
            }
        }
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
    long updateInterval = pdMS_TO_TICKS(60000*30);
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
            if (notif == EVENT_AUTOMATION::PAUSE){
                paused = true;
                automation->updateManualControlTime();
            }
            else if (notif == EVENT_AUTOMATION::RESUME){
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
            // Create a template update task using SendTask
            String updateTaskId = SendTask::createTaskOnCore(
                [automation]() {
                    automation->fetchAndAddNewBehaviors();
                },
                "UpdateTemplate",           // Task name
                20 * 1024,                  // Stack size
                1,                          // Priority
                0,                          // Core ID
                "Periodic template update task" // Description
            );
            
            if (automation->_logger && updateTaskId.isEmpty()) {
                automation->_logger->error("Failed to create template update task");
            }
            
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
        if (_fileManager && _fileManager->exists(_templatesFile)) {
            templateDefault = _fileManager->readFile(_templatesFile);
        }

        if (_fileManager && _fileManager->exists(_templatesUpdateFile)) {
            templateDefault += _fileManager->readFile(_templatesUpdateFile);
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
         
        _commandMapper->executeCommandString(behavior.toString());
        
        if (_logger) {
            _logger->debug("Executed automation behavior commands");
        }
    }
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
        
    // Store a pointer to this for use in the lambda
    Automation* self = this;
        
    // Send the prompt to GPT using sendPromptWithCustomSystem for better memory management
    ::gptAdapter->sendPromptWithCustomSystem(prompt, _behaviorPrompt, 
        [self, doneSemaphore, &success](const Utils::Sstring& response) {
        if (self->_logger) {
            self->_logger->info("GPT Response received");
            self->_logger->info("%s", response.c_str());
        }

        if (self->_templateBehaviors.size() > 100) {
            self->_templateBehaviors.erase(self->_templateBehaviors.begin(), 
                self->_templateBehaviors.begin() + 50);
        }

        int nextPos = 0;
        int startPos = 0;
        int total = 0;
        while ((nextPos = response.indexOf('\n', startPos)) != -1) {
            Utils::Sstring behavior = response.toString().substring(startPos, nextPos);
            behavior.trim();
            if (behavior.length() > 0) {
                self->_templateBehaviors.push_back(behavior);
                total++;
            }
            startPos = nextPos + 1;
        }
        
        // Set success flag if we added at least one behavior
        success = (startPos > 0);
        
        if (self->_logger) {
            if (success) {
                self->_fileManager->writeFile(
                    self->_templatesUpdateFile, 
                    response.c_str()
                );
                self->_logger->info("Added %d new behaviors from GPT", total);
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

} // namespace Automation
