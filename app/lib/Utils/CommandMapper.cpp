#include "CommandMapper.h"
#include "setup/setup.h"  // For access to updateManualControlTime()
#include <tasks/register.h>

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

    // Microphone commands
    #if MICROPHONE_ANALOG
    _commandHandlers["MIC_CALIBRATE"] = [this](const String& param) -> bool {
        amicrophone->calibrateBaseline();
        _logger->debug("Microphone calibration initiated");
        return true;
    };

    _commandHandlers["MIC_GAIN_LOW"] = [this](const String& param) -> bool {
        amicrophone->setGain(LOW);
        _logger->debug("Microphone gain set to LOW (40dB)");
        return true;
    };

    _commandHandlers["MIC_GAIN_MID"] = [this](const String& param) -> bool {
        amicrophone->setGain(HIGH);
        _logger->debug("Microphone gain set to MID (50dB)");
        return true;
    };

    _commandHandlers["MIC_GAIN_HIGH"] = [this](const String& param) -> bool {
        amicrophone->setGain(INPUT); // Floating pin for 60dB
        _logger->debug("Microphone gain set to HIGH (60dB)");
        return true;
    };
    #endif

    // Audio/Speaker commands
    _commandHandlers["PLAY_BEEP"] = [this](const String& param) -> bool {
        int volume = param.isEmpty() ? 50 : param.toInt();
        volume = constrain(volume, 0, 100);
        playSpeakerBeep(volume);
        _logger->debug("Played beep at volume " + String(volume));
        return true;
    };

    _commandHandlers["PLAY_TONE"] = [this](const String& param) -> bool {
        // Format: frequency,duration,volume (e.g., "440,1000,50")
        if (param.isEmpty()) {
            playSpeakerTone(440, 1000, 50); // Default 440Hz for 1s at 50% volume
        } else {
            int commaIndex1 = param.indexOf(',');
            int commaIndex2 = param.lastIndexOf(',');
            
            int frequency = 440;
            int duration = 1000;
            int volume = 50;
            
            if (commaIndex1 > 0) {
                frequency = param.substring(0, commaIndex1).toInt();
                if (commaIndex2 > commaIndex1) {
                    duration = param.substring(commaIndex1 + 1, commaIndex2).toInt();
                    volume = param.substring(commaIndex2 + 1).toInt();
                } else {
                    duration = param.substring(commaIndex1 + 1).toInt();
                }
            } else {
                frequency = param.toInt();
            }
            
            frequency = constrain(frequency, 20, 20000);
            duration = constrain(duration, 10, 10000);
            volume = constrain(volume, 0, 100);
            
            playSpeakerTone(frequency, duration, volume);
        }
        _logger->debug("Played tone");
        return true;
    };

    _commandHandlers["PLAY_CONFIRMATION"] = [this](const String& param) -> bool {
        int volume = param.isEmpty() ? 50 : param.toInt();
        volume = constrain(volume, 0, 100);
        playSpeakerConfirmation(volume);
        _logger->debug("Played confirmation sound at volume " + String(volume));
        return true;
    };

    _commandHandlers["PLAY_ERROR"] = [this](const String& param) -> bool {
        int volume = param.isEmpty() ? 50 : param.toInt();
        volume = constrain(volume, 0, 100);
        playSpeakerError(volume);
        _logger->debug("Played error sound at volume " + String(volume));
        return true;
    };

    _commandHandlers["PLAY_NOTIFICATION"] = [this](const String& param) -> bool {
        int volume = param.isEmpty() ? 50 : param.toInt();
        volume = constrain(volume, 0, 100);
        playSpeakerNotification(volume);
        _logger->debug("Played notification sound at volume " + String(volume));
        return true;
    };

    _commandHandlers["PLAY_AUDIO_FILE"] = [this](const String& param) -> bool {
        // Format: filepath,volume (e.g., "/sounds/alert.czmo,60")
        if (param.isEmpty()) {
            _logger->warning("PLAY_AUDIO_FILE requires filepath parameter");
            return false;
        }
        
        String filePath;
        int volume = 50;
        
        int commaIndex = param.lastIndexOf(',');
        if (commaIndex > 0) {
            filePath = param.substring(0, commaIndex);
            volume = param.substring(commaIndex + 1).toInt();
        } else {
            filePath = param;
        }
        
        volume = constrain(volume, 0, 100);
        
        bool success = playSpeakerAudioFile(filePath, volume);
        if (success) {
            _logger->debug("Playing audio file: " + filePath + " at volume " + String(volume));
        } else {
            _logger->error("Failed to play audio file: " + filePath);
        }
        return success;
    };

    _commandHandlers["STOP_AUDIO"] = [this](const String& param) -> bool {
        stopSpeaker();
        _logger->debug("Stopped audio playback");
        return true;
    };

    _commandHandlers["SET_VOLUME"] = [this](const String& param) -> bool {
        int volume = param.isEmpty() ? 50 : param.toInt();
        volume = constrain(volume, 0, 100);
        setSpeakerVolume(volume);
        _logger->debug("Set speaker volume to " + String(volume));
        return true;
    };

    _commandHandlers["PLAY_MP3_FILE"] = [this](const String& param) -> bool {
        // Format: filepath,volume (e.g., "/sounds/music.mp3,60")
        if (param.isEmpty()) {
            _logger->warning("PLAY_MP3_FILE requires filepath parameter");
            return false;
        }
        
        String filePath;
        int volume = 50;
        
        int commaIndex = param.lastIndexOf(',');
        if (commaIndex > 0) {
            filePath = param.substring(0, commaIndex);
            volume = param.substring(commaIndex + 1).toInt();
        } else {
            filePath = param;
        }
        
        volume = constrain(volume, 0, 100);
        
        bool success = playSpeakerMP3File(filePath, volume);
        if (success) {
            _logger->debug("Playing MP3 file: " + filePath + " at volume " + String(volume));
        } else {
            _logger->error("Failed to play MP3 file: " + filePath);
        }
        return success;
    };

    _commandHandlers["MP3_INFO"] = [this](const String& param) -> bool {
        if (param.isEmpty()) {
            _logger->warning("MP3_INFO requires filepath parameter");
            return false;
        }
        
        int sampleRate, channels, bitRate, duration;
        bool success = getMP3FileInfo(param, &sampleRate, &channels, &bitRate, &duration);
        
        if (success) {
            _logger->info("MP3 Info: " + param + " - " + String(sampleRate) + "Hz, " + 
                         String(channels) + "ch, " + String(bitRate) + "kbps, " + String(duration) + "s");
        } else {
            _logger->error("Failed to get MP3 info for: " + param);
        }
        return success;
    };

    _commandHandlers["CONVERT_MP3"] = [this](const String& param) -> bool {
        // Format: source.mp3,destination.czmo
        if (param.isEmpty()) {
            _logger->warning("CONVERT_MP3 requires source,destination parameters");
            return false;
        }
        
        int commaIndex = param.indexOf(',');
        if (commaIndex <= 0) {
            _logger->warning("CONVERT_MP3 format: source.mp3,destination.czmo");
            return false;
        }
        
        String sourcePath = param.substring(0, commaIndex);
        String destPath = param.substring(commaIndex + 1);
        
        bool success = convertMP3ToAudioFile(sourcePath, destPath);
        if (success) {
            _logger->debug("Converted MP3: " + sourcePath + " -> " + destPath);
        } else {
            _logger->error("Failed to convert MP3: " + sourcePath);
        }
        return success;
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
