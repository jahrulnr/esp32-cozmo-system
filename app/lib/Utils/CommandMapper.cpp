#include "CommandMapper.h"
#include "app.h"  // For access to updateManualControlTime()

namespace Utils {

CommandMapper::CommandMapper(Utils::Logger *logger, Screen::Screen* screen, Motors::MotorControl* motors, Motors::ServoControl* servos)
    : _screen(screen), _motors(motors), _servos(servos) {
		_logger = logger;
    initCommandHandlers();
}

void CommandMapper::initCommandHandlers() {
    // Face expression commands
    _commandHandlers["FACE_NORMAL"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Normal();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_ANGRY"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Angry();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_GLEE"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Glee();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_HAPPY"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Happy();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_SAD"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Sad();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_WORRIED"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Worried();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_FOCUSED"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Focused();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_ANNOYED"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Annoyed();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_SURPRISED"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Surprised();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_SKEPTIC"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Skeptic();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_FRUSTRATED"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Frustrated();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_UNIMPRESSED"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Unimpressed();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_SLEEPY"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Sleepy();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_SUSPICIOUS"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Suspicious();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_SQUINT"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Squint();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_FURIOUS"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Furious();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_SCARED"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Scared();
            return true;
        }
        return false;
    };
    
    _commandHandlers["FACE_AWE"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Awe();
            return true;
        }
        return false;
    };
    
    // Look direction commands
    _commandHandlers["LOOK_LEFT"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->LookLeft();
            return true;
        }
        return false;
    };
    
    _commandHandlers["LOOK_RIGHT"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->LookRight();
            return true;
        }
        return false;
    };
    
    _commandHandlers["LOOK_FRONT"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->LookFront();
            return true;
        }
        return false;
    };
    
    _commandHandlers["LOOK_TOP"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->LookTop();
            return true;
        }
        return false;
    };
    
    _commandHandlers["LOOK_BOTTOM"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->LookBottom();
            return true;
        }
        return false;
    };
    
    _commandHandlers["BLINK"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->DoBlink();
            return true;
        }
        return false;
    };
    
    // Motor movement commands
    _commandHandlers["MOVE_FORWARD"] = [this](const String& param) -> bool {
        if (_motors) {
            int duration = param.isEmpty() ? _defaultMoveDuration : parseTimeParam(param);
            _motors->move(Motors::MotorControl::FORWARD, duration);
            _logger->debug("Moving forward for " + String(duration) + "ms");
            delay(duration);  // Block until movement completes
            return true;
        }
        return false;
    };
    
    _commandHandlers["MOVE_BACKWARD"] = [this](const String& param) -> bool {
        if (_motors) {
            int duration = param.isEmpty() ? _defaultMoveDuration : parseTimeParam(param);
            _motors->move(Motors::MotorControl::BACKWARD, duration);
            _logger->debug("Moving backward for " + String(duration) + "ms");
            delay(duration);  // Block until movement completes
            return true;
        }
        return false;
    };
    
    _commandHandlers["TURN_LEFT"] = [this](const String& param) -> bool {
        if (_motors) {
            int duration = param.isEmpty() ? _defaultTurnDuration : parseTimeParam(param);
            _motors->move(Motors::MotorControl::LEFT, duration);
            _logger->debug("Turning left for " + String(duration) + "ms");
            delay(duration);  // Block until movement completes
            return true;
        }
        return false;
    };
    
    _commandHandlers["TURN_RIGHT"] = [this](const String& param) -> bool {
        if (_motors) {
            int duration = param.isEmpty() ? _defaultTurnDuration : parseTimeParam(param);
            _motors->move(Motors::MotorControl::RIGHT, duration);
            _logger->debug("Turning right for " + String(duration) + "ms");
            delay(duration);  // Block until movement completes
            return true;
        }
        return false;
    };
    
    _commandHandlers["STOP"] = [this](const String& param) -> bool {
        if (_motors) {
            _motors->stop();
            _logger->debug("Motors stopped");
            return true;
        }
        return false;
    };
    
    // Servo commands
    _commandHandlers["HEAD_UP"] = [this](const String& param) -> bool {
        if (_servos) {
            _servos->setHead(180);
            _logger->debug("Head up");
            return true;
        }
        return false;
    };
    
    _commandHandlers["HEAD_DOWN"] = [this](const String& param) -> bool {
        if (_servos) {
            _servos->setHead(0);
            _logger->debug("Head down");
            return true;
        }
        return false;
    };
    
    _commandHandlers["HEAD_CENTER"] = [this](const String& param) -> bool {
        if (_servos) {
            _servos->setHead(90);
            _logger->debug("Head centered");
            return true;
        }
        return false;
    };
    
    _commandHandlers["HAND_UP"] = [this](const String& param) -> bool {
        if (_servos) {
            _servos->setHand(180);
            _logger->debug("hand up");
            return true;
        }
        return false;
    };
    
    _commandHandlers["HAND_DOWN"] = [this](const String& param) -> bool {
        if (_servos) {
            _servos->setHand(0);
            _logger->debug("hand down");
            return true;
        }
        return false;
    };
    
    _commandHandlers["HAND_CENTER"] = [this](const String& param) -> bool {
        if (_servos) {
            _servos->setHand(90);
            _logger->debug("hand centered");
            return true;
        }
        return false;
    };
    
    // Custom position commands
    _commandHandlers["HEAD_POSITION"] = [this](const String& param) -> bool {
        if (_servos) {
            int angle = param.isEmpty() ? 90 : param.toInt();
            // Constrain the angle to valid range
            angle = constrain(angle, 0, 180);
            _servos->setHead(angle);
            _logger->debug("head position set to " + String(angle));
            return true;
        }
        return false;
    };
    
    _commandHandlers["HAND_POSITION"] = [this](const String& param) -> bool {
        if (_servos) {
            int angle = param.isEmpty() ? 90 : param.toInt();
            // Constrain the angle to valid range
            angle = constrain(angle, 0, 180);
            _servos->setHand(angle);
            _logger->debug("hand position set to " + String(angle));
            return true;
        }
        return false;
    };
    
    // Custom motor movement commands with duration control
    _commandHandlers["MOTOR_LEFT"] = [this](const String& param) -> bool {
        if (_motors) {
            int duration = param.isEmpty() ? 100 : param.toInt();
            // TODO: Implement motor duration control when available
            _motors->move(Motors::MotorControl::LEFT, duration);
            _logger->debug("Left motor activated at duration " + String(duration) + " for " + String(duration) + "ms");
            return true;
        }
        return false;
    };
    
    _commandHandlers["MOTOR_RIGHT"] = [this](const String& param) -> bool {
        if (_motors) {
            int duration = param.isEmpty() ? 100 : param.toInt();
            // TODO: Implement motor duration control when available
            _motors->move(Motors::MotorControl::RIGHT, duration);
            _logger->debug("Right motor activated at duration " + String(duration) + " for " + String(duration) + "ms");
            return true;
        }
        return false;
    };
    
    // Combined movements
    _commandHandlers["DANCE_SPIN"] = [this](const String& param) -> bool {
        if (_motors && _screen && _screen->getFace()) {
            _screen->getFace()->Expression.GoTo_Happy();
            _motors->move(Motors::MotorControl::LEFT, 500);
            vTaskDelay(pdMS_TO_TICKS(500));
            _motors->move(Motors::MotorControl::RIGHT, 500);
            vTaskDelay(pdMS_TO_TICKS(500));
            _motors->move(Motors::MotorControl::LEFT, 500);
            vTaskDelay(pdMS_TO_TICKS(500));
            _motors->stop();
            _logger->debug("Performed spin dance");
            return true;
        }
        return false;
    };
    
    _commandHandlers["LOOK_AROUND"] = [this](const String& param) -> bool {
        if (_screen && _screen->getFace()) {
            _screen->getFace()->LookLeft();
            vTaskDelay(pdMS_TO_TICKS(500));
            _screen->getFace()->LookRight();
            vTaskDelay(pdMS_TO_TICKS(500));
            _screen->getFace()->LookTop();
            vTaskDelay(pdMS_TO_TICKS(500));
            _screen->getFace()->LookBottom();
            vTaskDelay(pdMS_TO_TICKS(500));
            _screen->getFace()->LookFront();
            _logger->debug("Looked around");
            return true;
        }
        return false;
    };
}

bool CommandMapper::executeCommand(const String& commandStr) {
    // Extract command and parameter using regex
    std::regex cmdRegex("\\[([A-Z_]+)(?:=([0-9msh]+))?\\]");
    std::cmatch matches;
    std::string cmdStrStd = commandStr.c_str();
    
    if (std::regex_match(cmdStrStd.c_str(), matches, cmdRegex)) {
        String command = String(matches[1].str().c_str());
        String parameter = matches.size() > 2 ? String(matches[2].str().c_str()) : "";
        
        _logger->debug("Executing command: " + command + (parameter.isEmpty() ? "" : " with param: " + parameter));
        
        // Mark as manual control to pause automation
        updateManualControlTime();
        
        // Look up command handler
        if (_commandHandlers.count(command.c_str()) > 0) {
            return _commandHandlers[command.c_str()](parameter);
        } else {
            _logger->warning("Unknown command: " + command);
            return false;
        }
    }
    
    _logger->warning("Invalid command format: " + commandStr);
    return false;
}

int CommandMapper::executeCommandString(const String& multiCommandStr) {
    // Extract all commands from string
    std::regex cmdRegex("\\[([A-Z_]+)(?:=([0-9msh]+))?\\]");
    std::string multiCmdStd = multiCommandStr.c_str();
    std::sregex_iterator it(multiCmdStd.begin(), multiCmdStd.end(), cmdRegex);
    std::sregex_iterator end;
    
    int successCount = 0;
    
    // Execute each command
    for (; it != end; ++it) {
        std::smatch match = *it;
        String cmdStr = match.str(0).c_str();
        
        if (executeCommand(cmdStr)) {
            successCount++;
        }
    }
    
    return successCount;
}

String CommandMapper::extractCommands(const String& gptResponse) {
    // Extract all commands from GPT response
    std::regex cmdRegex("\\[([A-Z_]+)(?:=([0-9msh]+))?\\]");
    std::string responseStd = gptResponse.c_str();
    
    std::string result;
    std::sregex_iterator it(responseStd.begin(), responseStd.end(), cmdRegex);
    std::sregex_iterator end;
    
    // Concatenate all commands
    for (; it != end; ++it) {
        std::smatch match = *it;
        result += match.str(0);
    }
    
    return String(result.c_str());
}

String CommandMapper::extractText(const String& gptResponse) {
    // Remove all commands from GPT response to get just the text
    std::regex cmdRegex("\\[([A-Z_]+)(?:=([0-9msh]+))?\\]");
    std::string responseStd = gptResponse.c_str();
    
    // Replace all commands with empty string
    std::string result = std::regex_replace(responseStd, cmdRegex, "");
    
    // Trim leading/trailing whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    return String(result.c_str());
}

int CommandMapper::parseTimeParam(const String& param) {
    int duration = 0;
    
    // Default if parsing fails
    if (param.isEmpty()) {
        return _defaultMoveDuration;
    }
    
    String numPart = "";
    String unit = "s";  // Default to seconds
    
    // Extract number and unit
    for (size_t i = 0; i < param.length(); i++) {
        if (isDigit(param[i])) {
            numPart += param[i];
        } else {
            unit = param.substring(i);
            break;
        }
    }
    
    // Parse number
    int value = numPart.toInt();
    if (value == 0) {
        value = 1;  // Default if parsing fails
    }
    
    // Convert to milliseconds based on unit
    if (unit.equals("s")) {
        duration = value * 1000;
    } else if (unit.equals("m")) {
        duration = value * 60000;
    } else if (unit.equals("h")) {
        duration = value * 3600000;
    } else if (unit.equals("ms")) {
        duration = value;
    } else {
        duration = value * 1000;  // Default to seconds
    }
    
    // Enforce a minimum duration to prevent very short actions
    if (duration < 100) {
        duration = 100;  // Minimum 100 ms
    }
    
    return duration;
}

} // namespace Utils
