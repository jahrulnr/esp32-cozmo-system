#pragma once

#include <Arduino.h>
#include <vector>
#include "lib/Utils/FileManager.h"
#include "lib/Motors/MotorControl.h"
#include "lib/Motors/ServoControl.h"
#include "lib/Screen/Screen.h"

namespace Automation {

/**
 * Template action types
 */
enum ActionType {
    LOOK,        // Look in direction
    FACE,        // Set facial expression
    MOTOR,       // Motor movement
    SERVO,       // Servo movement
    SPEECH,      // Text to display/speak
    UNKNOWN      // Unknown action type
};

/**
 * Individual action within a template 
 */
struct TemplateAction {
    ActionType type;
    String subType;      // e.g. "LEFT" for LOOK_LEFT
    int duration;        // Duration in milliseconds
    String text;         // For SPEECH actions
};

/**
 * Represents a complete behavior template with multiple actions
 */
struct BehaviorTemplate {
    std::vector<TemplateAction> actions;
    String text;         // Speech text to display
    String category;     // For filtering: "happy", "scared", "curious", etc.
    
    // Helper method to add context-specific categories
    void assignCategory() {
        // Default assignment based on facial expressions and other cues
        if (text.indexOf("?") >= 0) {
            category = "curious";
        }
        
        for (auto& action : actions) {
            if (action.type == FACE) {
                if (action.subType == "HAPPY" || action.subType == "GLEE") {
                    category = "happy";
                } else if (action.subType == "SCARED") {
                    category = "scared";
                } else if (action.subType == "WORRIED") {
                    category = "worried";
                } else if (action.subType == "ANGRY" || action.subType == "FURIOUS") {
                    category = "angry";
                } else if (action.subType == "SKEPTIC" || action.subType == "SUSPICIOUS") {
                    category = "skeptical";
                } else if (action.subType == "SURPRISED" || action.subType == "AWE") {
                    category = "surprised";
                } else if (action.subType == "FOCUSED" || action.subType == "SQUINT") {
                    category = "focused";
                } else if (action.subType == "UNIMPRESSED") {
                    category = "unimpressed";
                }
            }
        }
    }
};

/**
 * Manages behavior templates loaded from templates.txt
 */
class TemplateManager {
public:
    /**
     * Constructor
     */
    TemplateManager();
    
    /**
     * Initialize the template manager
     * 
     * @return true if initialization successful
     */
    bool init();
    
    /**
     * Execute a random template from a specific category
     * 
     * @param category The category to select from, or empty for any
     * @return true if template was executed
     */
    bool executeRandomTemplate(String category = "");
    
    /**
     * Execute a specific template
     * 
     * @param index Index of the template to execute
     * @return true if template was executed successfully
     */
    bool executeTemplate(int index);
    
    /**
     * Execute a template for a specific event
     * 
     * @param event Event type (obstacle, cliff, temperature, etc.)
     * @return true if a template was found and executed
     */
    bool executeEventTemplate(String event);
    
    /**
     * Get total number of loaded templates
     */
    size_t getTemplateCount() const { return _templates.size(); }
    
    /**
     * Set dependencies
     */
    void setDependencies(Motors::MotorControl* motors, Motors::ServoControl* servos, Screen::Screen* screen) {
        _motors = motors;
        _servos = servos;
        _screen = screen;
    }
    
private:
    std::vector<BehaviorTemplate> _templates;
    Motors::MotorControl* _motors;
    Motors::ServoControl* _servos;
    Screen::Screen* _screen;
    bool _initialized;
    
    /**
     * Load templates from file
     */
    bool loadTemplates();
    
    /**
     * Parse a template line into actions
     */
    BehaviorTemplate parseTemplateLine(const String& line);
    
    /**
     * Execute a specific action
     */
    bool executeAction(const TemplateAction& action);
};

} // namespace Automation
