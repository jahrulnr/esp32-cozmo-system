#include "TemplateManager.h"
#include "app.h"  // For logger

namespace Automation {

// Default path for template file
#define TEMPLATES_FILE "/data/config/templates.txt" 

TemplateManager::TemplateManager() : 
    _motors(nullptr),
    _servos(nullptr),
    _screen(nullptr),
    _initialized(false)
{
}

bool TemplateManager::init() {
    if (_initialized) {
        return true;
    }
    
    if (!loadTemplates()) {
        logger->error("Failed to load behavior templates");
        return false;
    }
    
    logger->info("Template manager initialized with " + String(_templates.size()) + " templates");
    _initialized = true;
    return true;
}

bool TemplateManager::loadTemplates() {
    Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for template loading");
        return false;
    }
    
    if (!fileManager.exists(TEMPLATES_FILE)) {
        logger->warning("Templates file not found: " + String(TEMPLATES_FILE));
        return false;
    }
    
    String content = fileManager.readFile(TEMPLATES_FILE);
    if (content.length() == 0) {
        logger->error("Empty templates file");
        return false;
    }
    
    // Split into lines and parse each template
    int start = 0;
    int end = content.indexOf('\n');
    
    while (end >= 0 || start < content.length()) {
        String line;
        if (end >= 0) {
            line = content.substring(start, end);
            start = end + 1;
        } else {
            line = content.substring(start);
            start = content.length();
        }
        
        // Skip empty lines and comments
        if (line.length() > 0 && !line.startsWith("//")) {
            BehaviorTemplate tmpl = parseTemplateLine(line);
            if (tmpl.actions.size() > 0) {
                _templates.push_back(tmpl);
            }
        }
        
        end = content.indexOf('\n', start);
    }
    
    if (_templates.size() == 0) {
        logger->warning("No valid templates found in file");
        return false;
    }
    
    // Assign categories to templates for better selection
    for (auto& tmpl : _templates) {
        tmpl.assignCategory();
    }
    
    logger->info("Loaded " + String(_templates.size()) + " behavior templates");
    return true;
}

BehaviorTemplate TemplateManager::parseTemplateLine(const String& line) {
    BehaviorTemplate result;
    
    // Find speech part (text in asterisks)
    int textStart = line.indexOf('*');
    int textEnd = line.lastIndexOf('*');
    
    if (textStart >= 0 && textEnd > textStart) {
        result.text = line.substring(textStart + 1, textEnd);
    }
    
    // Process action blocks [ACTION=duration]
    int pos = 0;
    while (pos < line.length()) {
        int actionStart = line.indexOf('[', pos);
        if (actionStart < 0) break;
        
        int actionEnd = line.indexOf(']', actionStart);
        if (actionEnd < 0) break;
        
        // Extract action content
        String actionStr = line.substring(actionStart + 1, actionEnd);
        TemplateAction action;
        action.type = UNKNOWN;
        
        // Parse the action type and value
        int equalsPos = actionStr.indexOf('=');
        if (equalsPos > 0) {
            String actionType = actionStr.substring(0, equalsPos);
            String durationStr = actionStr.substring(equalsPos + 1);
            
            // Parse duration (support for "1s" or "500ms" format)
            int duration = 0;
            if (durationStr.endsWith("ms")) {
                duration = durationStr.substring(0, durationStr.length() - 2).toInt();
            } else if (durationStr.endsWith("s")) {
                duration = durationStr.substring(0, durationStr.length() - 1).toInt() * 1000;
            } else {
                duration = durationStr.toInt();
            }
            action.duration = duration;
            
            // Extract action main type and subtype
            if (actionType.startsWith("LOOK_")) {
                action.type = LOOK;
                action.subType = actionType.substring(5);
            } else if (actionType.startsWith("FACE_")) {
                action.type = FACE;
                action.subType = actionType.substring(5);
            } else if (actionType.startsWith("MOTOR_")) {
                action.type = MOTOR;
                action.subType = actionType.substring(6);
            } else if (actionType.startsWith("SERVO_")) {
                action.type = SERVO;
                action.subType = actionType.substring(6);
            }
            
            // Add valid actions to the template
            if (action.type != UNKNOWN) {
                result.actions.push_back(action);
            }
        }
        
        pos = actionEnd + 1;
    }
    
    return result;
}

bool TemplateManager::executeRandomTemplate(String category) {
    if (!_initialized || _templates.size() == 0) {
        return false;
    }
    
    if (!_motors || !_screen) {
        logger->warning("Cannot execute template - dependencies not set");
        return false;
    }
    
    std::vector<int> candidateIndices;
    
    // Find templates matching the category
    if (category.length() > 0) {
        for (size_t i = 0; i < _templates.size(); i++) {
            if (_templates[i].category == category) {
                candidateIndices.push_back(i);
            }
        }
    } else {
        // If no category specified, use all templates
        for (size_t i = 0; i < _templates.size(); i++) {
            candidateIndices.push_back(i);
        }
    }
    
    // Select a random template from candidates
    if (candidateIndices.size() == 0) {
        logger->warning("No templates found for category: " + category);
        return false;
    }
    
    int randomIndex = candidateIndices[random(candidateIndices.size())];
    return executeTemplate(randomIndex);
}

bool TemplateManager::executeEventTemplate(String event) {
    // Map events to appropriate template categories
    String category;
    
    if (event == "obstacle") {
        category = "surprised";
    } else if (event == "cliff") {
        category = "scared";
    } else if (event == "temperature_high") {
        category = "worried";
    } else if (event == "temperature_low") {
        category = "worried";
    } else if (event == "mapping") {
        category = "focused";
    } else if (event == "discovery") {
        category = "happy";
    } else if (event == "explore") {
        category = "curious";
    }
    
    if (category.length() > 0) {
        return executeRandomTemplate(category);
    }
    
    // Fallback to any random template
    return executeRandomTemplate();
}

bool TemplateManager::executeTemplate(int index) {
    if (!_initialized || index < 0 || index >= (int)_templates.size()) {
        return false;
    }
    
    const BehaviorTemplate& tmpl = _templates[index];
    logger->info("Executing behavior template with " + String(tmpl.actions.size()) + " actions");
    
    // Execute each action in sequence
    for (const auto& action : tmpl.actions) {
        executeAction(action);
    }
    
    // Display speech text if available
    if (tmpl.text.length() > 0 && _screen) {
        if (_screen->getFace()) {
            // TODO: Implement text display on face
            // _screen->getFace()->showSpeech(tmpl.text);
        }
        logger->info("Robot says: " + tmpl.text);
    }
    
    return true;
}

bool TemplateManager::executeAction(const TemplateAction& action) {
    switch (action.type) {
        case LOOK: {
            if (!_screen || !_screen->getFace()) return false;
            
            // Execute look actions
            if (action.subType == "LEFT") {
                _screen->getFace()->LookLeft();
            } else if (action.subType == "RIGHT") {
                _screen->getFace()->LookRight();
            } else if (action.subType == "TOP" || action.subType == "UP") {
                _screen->getFace()->LookTop();
            } else if (action.subType == "BOTTOM" || action.subType == "DOWN") {
                _screen->getFace()->LookBottom();
            } else if (action.subType == "FRONT") {
                _screen->getFace()->LookFront();
            }
            break;
        }
        
        case FACE: {
            if (!_screen || !_screen->getFace()) return false;
            
            // Execute facial expressions
            if (action.subType == "HAPPY") {
                _screen->getFace()->Expression.GoTo_Happy();
            } else if (action.subType == "SKEPTIC") {
                _screen->getFace()->Expression.GoTo_Skeptic();
            } else if (action.subType == "SURPRISED") {
                _screen->getFace()->Expression.GoTo_Surprised();
            } else if (action.subType == "FOCUSED") {
                _screen->getFace()->Expression.GoTo_Focused();
            } else if (action.subType == "GLEE") {
                _screen->getFace()->Expression.GoTo_Glee();
            } else if (action.subType == "WORRIED") {
                _screen->getFace()->Expression.GoTo_Worried();
            } else if (action.subType == "ANGRY") {
                _screen->getFace()->Expression.GoTo_Angry();
            } else if (action.subType == "SCARED") {
                _screen->getFace()->Expression.GoTo_Scared();
            } else if (action.subType == "UNIMPRESSED") {
                _screen->getFace()->Expression.GoTo_Unimpressed();
            } else if (action.subType == "AWE") {
                _screen->getFace()->Expression.GoTo_Awe();
            } else if (action.subType == "SQUINT") {
                _screen->getFace()->Expression.GoTo_Squint();
            } else if (action.subType == "FRUSTRATED") {
                _screen->getFace()->Expression.GoTo_Frustrated();
            } else if (action.subType == "SLEEPY") {
                _screen->getFace()->Expression.GoTo_Sleepy();
            } else if (action.subType == "FURIOUS") {
                _screen->getFace()->Expression.GoTo_Furious();
            } else if (action.subType == "SUSPICIOUS") {
                _screen->getFace()->Expression.GoTo_Suspicious();
            }
            break;
        }
        
        case MOTOR: {
            if (!_motors) return false;
            
            // Execute motor actions
            Motors::MotorControl::Direction direction = Motors::MotorControl::STOP;
            
            if (action.subType == "FORWARD") {
                direction = Motors::MotorControl::FORWARD;
            } else if (action.subType == "BACKWARD") {
                direction = Motors::MotorControl::BACKWARD;
            } else if (action.subType == "LEFT") {
                direction = Motors::MotorControl::LEFT;
            } else if (action.subType == "RIGHT") {
                direction = Motors::MotorControl::RIGHT;
            }
            
            if (direction != Motors::MotorControl::STOP) {
                _motors->move(direction, action.duration);
            }
            break;
        }
        
        case SERVO: {
            if (!_servos) return false;
            
            // Execute servo actions
            // Set head position based on servo angles
            if (action.subType == "UP") {
                _servos->setHead(150); // Up position (150 degrees)
                vTaskDelay(pdMS_TO_TICKS(action.duration));
                _servos->setHead(90);  // Return to center
            } else if (action.subType == "DOWN") {
                _servos->setHead(30);  // Down position (30 degrees)
                vTaskDelay(pdMS_TO_TICKS(action.duration));
                _servos->setHead(90);  // Return to center
            } else if (action.subType == "LEFT") {
                _servos->setHand(150); // Left position for hand servo
                vTaskDelay(pdMS_TO_TICKS(action.duration));
                _servos->setHand(90);  // Return to center
            } else if (action.subType == "RIGHT") {
                _servos->setHand(30);  // Right position for hand servo
                vTaskDelay(pdMS_TO_TICKS(action.duration));
                _servos->setHand(90);  // Return to center
            }
            break;
        }
        
        default:
            return false;
    }
    
    // Wait for the duration of the action
    vTaskDelay(pdMS_TO_TICKS(action.duration));
    return true;
}

} // namespace Automation
