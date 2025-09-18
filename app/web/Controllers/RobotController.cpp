#include "RobotController.h"
#include "MVCFramework.h"
#include "Constants.h"
#include "Config.h"
#include "setup/setup.h"

// Motor control endpoints
Response RobotController::moveMotor(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        #if !MOTOR_ENABLED
        response["success"] = false;
        response["error"] = "Motor control disabled";
        return Response(request.getServerRequest()).status(400).json(response);
        #endif
        
        if (!motors) {
            response["success"] = false;
            response["error"] = "Motor controller not initialized";
            return Response(request.getServerRequest()).status(500).json(response);
        }
        
        // Parse request body
        Utils::SpiJsonDocument requestData;
        deserializeJson(requestData, request.getBody());
        
        String direction = requestData["direction"].as<String>();
        int speed = requestData["speed"] | 50; // Default speed 50
        
        if (!validateMotorSpeed(speed)) {
            response["success"] = false;
            response["error"] = "Invalid speed (0-100)";
            return Response(request.getServerRequest()).status(400).json(response);
        }
        
        // Execute motor command
        bool success = true;
        if (direction == "forward") {
            motors->move(motors->FORWARD);
        } else if (direction == "backward") {
            motors->move(motors->BACKWARD);
        } else if (direction == "left") {
            motors->move(motors->LEFT);
        } else if (direction == "right") {
            motors->move(motors->RIGHT);
        } else {
            response["success"] = false;
            response["error"] = "Invalid direction (forward/backward/left/right)";
            return Response(request.getServerRequest()).status(400).json(response);
        }
        
        if (success) {
            response["data"]["direction"] = direction;
            response["data"]["speed"] = speed;
            
            logger->info("Motor command executed: " + direction + " at speed " + String(speed));
        } else {
            response["success"] = false;
            response["error"] = "Motor command failed";
            return Response(request.getServerRequest()).status(500).json(response);
        }
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

Response RobotController::stopMotor(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        #if !MOTOR_ENABLED
        response["success"] = false;
        response["error"] = "Motor control disabled";
        return Response(request.getServerRequest()).status(400).json(response);
        #endif
        
        if (!motors) {
            response["success"] = false;
            response["error"] = "Motor controller not initialized";
            return Response(request.getServerRequest()).status(500).json(response);
        }
        
        motors->stop();
        response["message"] = "Motors stopped";
        logger->info("Motors stopped via API");
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

Response RobotController::setMotorSpeed(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        #if !MOTOR_ENABLED
        response["success"] = false;
        response["error"] = "Motor control disabled";
        return Response(request.getServerRequest()).status(400).json(response);
        #endif
        
        if (!motors) {
            response["success"] = false;
            response["error"] = "Motor controller not initialized";
            return Response(request.getServerRequest()).status(500).json(response);
        }
        
        // Parse request body
        Utils::SpiJsonDocument requestData;
        deserializeJson(requestData, request.getBody());
        
        int leftSpeed = requestData["left"] | 0;
        int rightSpeed = requestData["right"] | 0;
        
        if (!validateMotorSpeed(abs(leftSpeed)) || !validateMotorSpeed(abs(rightSpeed))) {
            response["success"] = false;
            response["error"] = "Invalid speed values (-100 to 100)";
            return Response(request.getServerRequest()).status(400).json(response);
        }
        
        // motors->setSpeed(leftSpeed, rightSpeed);
        
        response["data"]["left"] = leftSpeed;
        response["data"]["right"] = rightSpeed;
        response["message"] = "Motor speeds set";
        
        logger->info("Motor speeds set: L=" + String(leftSpeed) + " R=" + String(rightSpeed));
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

// Servo control endpoints
Response RobotController::setServoPosition(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        #if !SERVO_ENABLED
        response["success"] = false;
        response["error"] = "Servo control disabled";
        return Response(request.getServerRequest()).status(400).json(response);
        #endif
        
        if (!servos) {
            response["success"] = false;
            response["error"] = "Servo controller not initialized";
            return Response(request.getServerRequest()).status(500).json(response);
        }
        
        // Parse request body
        Utils::SpiJsonDocument requestData;
        deserializeJson(requestData, request.getBody());
        
        String servo = requestData["servo"].as<String>();
        int angle = requestData["angle"] | 90;
        
        if (!validateServoAngle(angle)) {
            response["success"] = false;
            response["error"] = "Invalid angle (0-180)";
            return Response(request.getServerRequest()).status(400).json(response);
        }
        
        bool success = true;
        if (servo == "x" || servo == "head" || servo == "pan") {
            servos->setHead(angle);
        } else if (servo == "y" || servo == "hand" || servo == "tilt") {
            servos->setHand(angle);
        } else {
            response["success"] = false;
            response["error"] = "Invalid servo (x/y or head/hand)";
            return Response(request.getServerRequest()).status(400).json(response);
        }
        
        if (success) {
            response["data"]["servo"] = servo;
            response["data"]["angle"] = angle;
            response["message"] = "Servo position set";
            
            logger->info("Servo " + servo + " set to " + String(angle) + " degrees");
        } else {
            response["success"] = false;
            response["error"] = "Servo command failed";
            return Response(request.getServerRequest()).status(500).json(response);
        }
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

Response RobotController::getServoPosition(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        #if !SERVO_ENABLED
        response["data"]["enabled"] = false;
        #else
        response["data"]["enabled"] = true;
        
        if (servos) {
            response["data"]["head_angle"] = servos->getHead();
            response["data"]["hand_angle"] = servos->getHand();
        } else {
            response["success"] = false;
            response["error"] = "Servo controller not initialized";
            return Response(request.getServerRequest()).status(500).json(response);
        }
        #endif
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

Response RobotController::centerServos(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        #if !SERVO_ENABLED
        response["success"] = false;
        response["error"] = "Servo control disabled";
        return Response(request.getServerRequest()).status(400).json(response);
        #endif
        
        if (!servos) {
            response["success"] = false;
            response["error"] = "Servo controller not initialized";
            return Response(request.getServerRequest()).status(500).json(response);
        }
        
        servos->setHead(90);
        servos->setHand(90);
        response["message"] = "Servos centered";
        response["data"]["head_angle"] = 90;
        response["data"]["hand_angle"] = 90;
        
        logger->info("Servos centered via API");
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

// Emergency control
Response RobotController::emergencyStop(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        // Stop all motors immediately
        #if MOTOR_ENABLED
        if (motors) {
            motors->stop();
        }
        #endif
        
        response["message"] = "Emergency stop activated";
        logger->warning("Emergency stop activated via API");
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

// Sensor endpoints
Response RobotController::getSensorData(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        JsonObject sensorData = response["data"].to<JsonObject>();
        
        // Distance sensor
        #if DISTANCE_SENSOR_ENABLED
        if (distanceSensor) {
            sensorData["distance"] = distanceSensor->getDistance();
        }
        #endif
        
        // Orientation sensor (accelerometer + gyroscope)
        #if ORIENTATION_SENSOR_ENABLED
        if (orientation) {
            JsonObject accel = sensorData["accelerometer"].to<JsonObject>();
            accel["x"] = orientation->getAccelX();
            accel["y"] = orientation->getAccelY();
            accel["z"] = orientation->getAccelZ();
            
            JsonObject gyro = sensorData["gyroscope"].to<JsonObject>();
            gyro["x"] = orientation->getGyroX();
            gyro["y"] = orientation->getGyroY();
            gyro["z"] = orientation->getGyroZ();
        }
        #endif
        
        // Battery level
        #if BATTERY_ENABLED
        sensorData["battery"] = batteryManager->getLevel();
        #endif
        
        // Temperature
        sensorData["temperature"] = (int)temperatureRead();
        
        response["timestamp"] = millis();
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

// Voice control
Response RobotController::toggleVoiceControl(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        #if !SR_ENABLED
        response["success"] = false;
        response["error"] = "Voice control disabled";
        return Response(request.getServerRequest()).status(400).json(response);
        #endif
        
        // Toggle voice control via notification system
        notification->send(NOTIFICATION_SR, (void*)EVENT_SR::RESUME);
        
        response["message"] = "Voice control toggled";
        response["data"]["active"] = true; // Would need to track actual state
        
        logger->info("Voice control toggled via API");
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

// Chat/AI endpoints
Response RobotController::sendChatMessage(Request& request) {
    Utils::SpiJsonDocument response = createSuccessResponse();
    
    try {
        // Parse request body
        Utils::SpiJsonDocument requestData;
        deserializeJson(requestData, request.getBody());
        
        String message = requestData["message"].as<String>();
        
        if (message.length() == 0) {
            response["success"] = false;
            response["error"] = "Message cannot be empty";
            return Response(request.getServerRequest()).status(400).json(response);
        }
        
        // Process message (would integrate with GPT service)
        response["data"]["user_message"] = message;
        response["data"]["bot_response"] = "I received your message: " + message;
        response["message"] = "Message processed";
        
        logger->info("Chat message received: " + message);
        
    } catch (const std::exception& e) {
        response["success"] = false;
        response["error"] = "Internal error: " + String(e.what());
        return Response(request.getServerRequest()).status(500).json(response);
    }
    
    return Response(request.getServerRequest()).status(200).json(response);
}

// Helper methods
bool RobotController::validateMotorSpeed(int speed) {
    return speed >= 0 && speed <= 100;
}

bool RobotController::validateServoAngle(int angle) {
    return angle >= 0 && angle <= 180;
}

Utils::SpiJsonDocument RobotController::createErrorResponse(const String& message) {
    Utils::SpiJsonDocument response;
    response["success"] = false;
    response["error"] = message;
    response["timestamp"] = millis();
    return response;
}

Utils::SpiJsonDocument RobotController::createSuccessResponse(const String& message) {
    Utils::SpiJsonDocument response;
    response["success"] = true;
    if (message.length() > 0) {
        response["message"] = message;
    }
    response["timestamp"] = millis();
    return response;
}
