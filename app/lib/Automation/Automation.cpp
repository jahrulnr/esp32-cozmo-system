#include "Automation.h"
#include "app.h"

namespace Automation {

// Constructor
Automation::Automation(Utils::FileManager* fileManager, 
                       Utils::CommandMapper* commandMapper,
                       Utils::Logger* logger,
                       Communication::WebSocketHandler* webSocket)
    : m_fileManager(fileManager)
    , m_commandMapper(commandMapper)
    , m_logger(logger)
    , m_webSocket(webSocket)
    , m_taskHandle(NULL)
    , m_enabled(AUTOMATION_ENABLED)
    , m_lastManualControlTime(0)
    , m_behaviorIndex(0) {
    // Create a mutex for thread-safe access to behaviors
    m_behaviorsMutex = xSemaphoreCreateMutex();
}

// Destructor
Automation::~Automation() {
    stop();
}

// Start automation task
void Automation::start() {
    if (m_taskHandle != NULL) {
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
        &m_taskHandle,   // Task handle
        0
    );
    
    if (m_logger) {
        m_logger->info("Automation task started");
    }
}

// Stop automation task
void Automation::stop() {
    if (m_taskHandle != NULL) {
        vTaskDelete(m_taskHandle);
        m_taskHandle = NULL;
        
        if (m_logger) {
            m_logger->info("Automation task stopped");
        }
    }
}

// Update the time of last manual control
void Automation::updateManualControlTime() {
    m_lastManualControlTime = millis();
}

// Get automation enabled status
bool Automation::isEnabled() const {
    return m_enabled;
}

// Set automation enabled status
void Automation::setEnabled(bool enabled) {
    m_enabled = enabled;
    
    // Send status to all connected clients
    if (m_webSocket) {
        Utils::SpiJsonDocument statusDoc;
        statusDoc["enabled"] = m_enabled;
        m_webSocket->sendJsonMessage(-1, "automation_status", statusDoc);
    }
    
    if (m_logger) {
        m_logger->info(String("Automation ") + (enabled ? "enabled" : "disabled"));
    }
}

// Static task function that receives the class instance
void Automation::taskFunction(void* parameter) {
    // Convert parameter to Automation instance
    Automation* automation = static_cast<Automation*>(parameter);
		long timer = millis();
		long interval = pdMS_TO_TICKS(60000);
    
    // Run automation forever
    while (true) {
        // Check if automation is enabled and no manual control for a while
        if (automation->m_enabled && 
            (millis() - automation->m_lastManualControlTime > AUTOMATION_INACTIVITY_TIMEOUT)) {
            
            // Take the mutex to safely access the behaviors list
            if (xSemaphoreTake(automation->m_behaviorsMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                // Only run if we have templates available
                if (!automation->m_templateBehaviors.empty()) {
                    // Get the next behavior from templates
                    Utils::Sstring behavior = automation->m_templateBehaviors[automation->m_behaviorIndex];
                    automation->m_behaviorIndex = (automation->m_behaviorIndex + 1) % 
                                                automation->m_templateBehaviors.size();
                    
                    // Release the mutex before executing the behavior
                    xSemaphoreGive(automation->m_behaviorsMutex);
                    
                    // Execute the behavior
                    automation->executeBehavior(behavior);
                    
                    // Reset the manual control timer to prevent rapid successive behaviors
                    automation->m_lastManualControlTime = millis();
                    
                    // Wait before next behavior (random delay between 5-10 seconds)
                    int randomDelay = random(5000, 10000);
                    vTaskDelay(pdMS_TO_TICKS(randomDelay));
                } else {
                    // Release the mutex if no behaviors to execute
                    xSemaphoreGive(automation->m_behaviorsMutex);
                }
            }
        }

				if (millis() - timer > interval) {
					xTaskCreate([](void * param){
    				Automation* automation = static_cast<Automation*>(param);
						automation->fetchAndAddNewBehaviors();
						vTaskDelete(NULL);
					}, "UpdateTemplate", 20 * 1024, automation, 8, NULL);
					timer = millis();
				}
        
        // Check at regular intervals
        vTaskDelay(pdMS_TO_TICKS(AUTOMATION_CHECK_INTERVAL));
    }
}

// Load template behaviors from file
void Automation::loadTemplateBehaviors() {
    // Take the mutex to safely modify the behaviors list
    if (xSemaphoreTake(m_behaviorsMutex, portMAX_DELAY) == pdTRUE) {
        m_templateBehaviors.clear();
        
        // Load template behaviors from file
        Utils::Sstring templateBehaviorsStr = "";
        if (m_fileManager && m_fileManager->exists("/config/templates.txt")) {
            templateBehaviorsStr = m_fileManager->readFile("/config/templates.txt");
        }
        
        // Split template behaviors into an array
        int startPos = 0;
        int nextPos = 0;
        while ((nextPos = templateBehaviorsStr.indexOf('\n', startPos)) != -1) {
            Utils::Sstring behavior = templateBehaviorsStr.substring(startPos, nextPos).trim();
            if (behavior.length() > 0) {
                m_templateBehaviors.push_back(behavior);
            }
            startPos = nextPos + 1;
        }
        
        // Add the remaining text as the last behavior if it exists
        Utils::Sstring lastBehavior = templateBehaviorsStr.substring(startPos).trim();
        if (lastBehavior.length() > 0) {
            m_templateBehaviors.push_back(lastBehavior);
        }
        
        // Give the mutex back
        xSemaphoreGive(m_behaviorsMutex);
        
        if (m_logger) {
            m_logger->info("Loaded " + String(m_templateBehaviors.size()) + " template behaviors");
        }
    }
}

// Execute a specific behavior
void Automation::executeBehavior(const Utils::Sstring& behavior) {
    if (m_commandMapper) {
        if (m_logger) {
            m_logger->debug("Executing automation behavior: " + behavior.toString());
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
            int commandCount = m_commandMapper->executeCommandString(behavior.toString());
            
            // Give the message some time to be visible, if it's not a long running action
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            // Just execute the commands without showing the message
            int commandCount = m_commandMapper->executeCommandString(behavior.toString());
        }
        
        if (m_logger) {
            m_logger->debug("Executed automation behavior commands");
        }
    }
}

// Add a new behavior to the templates
bool Automation::addNewBehavior(const String& behavior) {
    if (behavior.isEmpty()) {
        return false;
    }
    
    // Take the mutex to safely modify the behaviors list
    if (xSemaphoreTake(m_behaviorsMutex, portMAX_DELAY) == pdTRUE) {
        // Add the new behavior to our list
        m_templateBehaviors.push_back(Utils::Sstring(behavior));
        
        // Save to file
        bool saveResult = saveBehaviorsToFile();
        
        // Give the mutex back
        xSemaphoreGive(m_behaviorsMutex);
        
        if (m_logger) {
            if (saveResult) {
                m_logger->info("New behavior added: " + behavior);
            } else {
                m_logger->error("Failed to save new behavior to file");
            }
        }
        
        return saveResult;
    }
    
    return false;
}

// Fetch new behaviors from GPT and add them
bool Automation::fetchAndAddNewBehaviors(const String& prompt) {
    if (!::gptAdapter) {
        if (m_logger) {
            m_logger->error("GPT adapter not available for generating behaviors");
        }
        return false;
    }
    
    if (m_logger) {
        m_logger->info("Requesting new behaviors from GPT with prompt: " + prompt);
    }
    
    // Create a semaphore for synchronizing with the callback
    SemaphoreHandle_t doneSemaphore = xSemaphoreCreateBinary();
    bool success = false;
    
    // Build a list of existing behaviors to avoid duplicates
    String existingBehaviorsList = "";
    int exampleCount = 0;
    
    // Take the mutex to safely access the behaviors list
    if (xSemaphoreTake(m_behaviorsMutex, portMAX_DELAY) == pdTRUE) {
        // Get up to 5 random examples from existing behaviors
        if (!m_templateBehaviors.empty()) {
            // Create a list of indices
            std::vector<int> indices(m_templateBehaviors.size());
            for (size_t i = 0; i < indices.size(); ++i) {
                indices[i] = static_cast<int>(i);
            }
            
            // Shuffle to get random behaviors
            for (size_t i = 0; i < indices.size(); ++i) {
                size_t j = random(0, indices.size());
                std::swap(indices[i], indices[j]);
            }
            
            // Get up to 5 examples
            for (size_t i = 0; i < std::min(static_cast<size_t>(5), m_templateBehaviors.size()); ++i) {
                existingBehaviorsList += "Example " + String(exampleCount + 1) + ": " + 
                                         m_templateBehaviors[indices[i]].toString() + "\n";
                exampleCount++;
            }
        }
        
        // Release the mutex
        xSemaphoreGive(m_behaviorsMutex);
    }
    
    // Additional command to guide GPT in generating appropriate behaviors
    String additionalCommand = 
        "Generate 5 new robot behaviors in the exact format of existing templates. "
        "Each behavior should be on a new line and use only this format: "
        "[ACTION=time][ACTION2=time] *Robot vocalization*\n"
        "Valid actions are: MOVE_FORWARD, MOVE_BACKWARD, TURN_LEFT, TURN_RIGHT, STOP, "
        "LOOK_LEFT, LOOK_RIGHT, LOOK_TOP, LOOK_BOTTOM, LOOK_FRONT, "
        "HEAD_UP, HEAD_DOWN, HAND_UP, HAND_DOWN, "
        "FACE_HAPPY, FACE_SAD, FACE_ANGRY, FACE_SURPRISED, FACE_WORRIED, "
        "FACE_SKEPTIC, FACE_FOCUSED, FACE_UNIMPRESSED, FACE_FRUSTRATED, "
        "FACE_SQUINT, FACE_AWE, FACE_GLEE, FACE_FURIOUS, FACE_SUSPICIOUS, FACE_SCARED, FACE_SLEEPY. "
        "Times should be specified in ms (500ms) or s (2s). "
        "Make behaviors unique and different from existing ones. "
        "Each behavior should represent a cohesive action with matching facial expression and vocalization. "
        "Do not include any explanations, numbering, or extra text.\n\n"
        "Here are some examples of current behaviors:\n" + existingBehaviorsList;
        
    // Store a pointer to this for use in the lambda
    Automation* self = this;
        
    // Send the prompt to GPT using sendPromptWithCustomSystem for better memory management
    ::gptAdapter->sendPromptWithCustomSystem(prompt, additionalCommand, 
        [self, doneSemaphore, &success](const String& response) {
        // Process the response from GPT
        int addedCount = 0;
        
        if (self->m_logger) {
            self->m_logger->debug("GPT Response received");
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
        
        if (self->m_logger) {
            if (success) {
                self->m_logger->info("Added " + String(addedCount) + " new behaviors from GPT");
            } else {
                self->m_logger->warning("No valid behaviors found in GPT response");
            }
        }
        
        // Signal that we're done processing
        xSemaphoreGive(doneSemaphore);
    });
    
    // Wait for the callback to complete (with a timeout)
    if (xSemaphoreTake(doneSemaphore, pdMS_TO_TICKS(30000)) != pdTRUE) {
        // Timed out waiting for GPT response
        if (m_logger) {
            m_logger->error("Timed out waiting for GPT to generate behaviors");
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
    if (!m_fileManager) {
        return false;
    }
    
    // Build the content to save
    String content = "";
    for (const auto& behavior : m_templateBehaviors) {
        content += behavior.toString() + "\n";
    }
    
    // Save to file
    bool success = m_fileManager->writeFile("/config/templates.txt", content);
    
    // Log the result
    if (m_logger) {
        if (success) {
            m_logger->info("Successfully saved " + String(m_templateBehaviors.size()) + " behaviors to templates.txt");
        } else {
            m_logger->error("Failed to save behaviors to templates.txt");
        }
    }
    
    return success;
}

} // namespace Automation
