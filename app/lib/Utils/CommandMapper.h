#pragma once

#include <Arduino.h>
#include <functional>
#include <map>
#include <regex>
#include "lib/Motors/MotorControl.h"
#include "lib/Motors/ServoControl.h"
#include "lib/Sensors/OrientationSensor.h"
#include "lib/Sensors/DistanceSensor.h"
#include "lib/Screen/Screen.h"
#include "Logger.h"

namespace Utils {

class CommandMapper {
public:
    // Constructor with all required subsystems
    CommandMapper(Utils::Logger *logger, Screen::Screen* screen, Motors::MotorControl* motors, Motors::ServoControl* servos);

    // Execute a command string (format: [COMMAND] or [COMMAND=PARAM])
    bool executeCommand(const String& commandStr);
    
    // Execute a series of commands in a single string
    int executeCommandString(const String& multiCommandStr);
    
    // Extract expression commands from GPT response
    String extractCommands(const String& gptResponse);
    
    // Extract the natural language text (after commands)
    String extractText(const String& gptResponse);

private:
    Screen::Screen* _screen;
    Motors::MotorControl* _motors;
    Motors::ServoControl* _servos;
    Utils::Logger* _logger;
    
    // Motor control durations
    int _defaultMoveDuration = 500;  // milliseconds
    int _defaultTurnDuration = 400;  // milliseconds
    
    // Parse time parameters (e.g., "10s", "1m")
    int parseTimeParam(const String& param);
    
    // Commands and handlers
    typedef std::function<bool(const String&)> CommandHandler;
    std::map<String, CommandHandler> _commandHandlers;
    
    // Initialize all command handlers
    void initCommandHandlers();
    
    // Command regex pattern
    const String _cmdPattern = "\\[([A-Z_]+)(?:=([0-9msh]+))?\\]";
};

} // namespace Utils
