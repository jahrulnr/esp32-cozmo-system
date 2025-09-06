#pragma once

#include <Arduino.h>
#include <functional>
#include <map>
#include <regex>
#include <Sstring.h>
#include "core/Motors/MotorControl.h"
#include "core/Motors/ServoControl.h"
#include "core/Sensors/OrientationSensor.h"
#include "core/Sensors/DistanceSensor.h"
#include "display/Display.h"
#include "Logger.h"

namespace Utils {

class CommandMapper {
public:
    // Constructor with all required subsystems
    CommandMapper(Utils::Logger *logger, Display::Display* display, Motors::MotorControl* motors, Motors::ServoControl* servos);

    // Execute a command string (format: [COMMAND] or [COMMAND=PARAM])
    bool executeCommand(const Utils::Sstring& commandStr);
    
    // Execute a series of commands in a single string
    int executeCommandString(const Utils::Sstring& multiCommandStr);
    
    // Extract expression commands from GPT response
    Utils::Sstring extractCommands(const Utils::Sstring& gptResponse);
    
    // Extract the natural language text (after commands)
    Utils::Sstring extractText(const Utils::Sstring& gptResponse);

private:
    Display::Display* _display;
    Motors::MotorControl* _motors;
    Motors::ServoControl* _servos;
    Utils::Logger* _logger;
    
    // Motor control durations
    int _defaultMoveDuration = 500;  // milliseconds
    int _defaultTurnDuration = 400;  // milliseconds
    
    // Parse time parameters (e.g., "10s", "1m")
    int parseTimeParam(const Utils::Sstring& param);
    
    // Commands and handlers
    typedef std::function<bool(const Utils::Sstring&)> CommandHandler;
    std::map<String, CommandHandler> _commandHandlers;
    
    // Initialize all command handlers
    void initCommandHandlers();
    
    // Command regex pattern
    const Utils::Sstring _cmdPattern = "\\[([A-Z_]+)(?:=([0-9msh]+))?\\]";
};

} // namespace Utils
