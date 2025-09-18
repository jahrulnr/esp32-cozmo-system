#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include <Arduino.h>
#include "Sstring.h"
#include "core/Utils/SpiAllocator.h"

// Forward declarations
class Request;
class Response;

class RobotController {
public:
    // Motor control endpoints
    static Response moveMotor(Request& request);
    static Response stopMotor(Request& request);
    static Response setMotorSpeed(Request& request);
    
    // Servo control endpoints
    static Response setServoPosition(Request& request);
    static Response getServoPosition(Request& request);
    static Response centerServos(Request& request);
    
    // Camera endpoints
    static Response getCameraFrame(Request& request);
    static Response setCameraSettings(Request& request);
    static Response getCameraStatus(Request& request);
    
    // Sensor endpoints
    static Response getSensorData(Request& request);
    static Response getDistanceSensor(Request& request);
    static Response getAccelerometer(Request& request);
    static Response getGyroscope(Request& request);
    
    // Emergency control
    static Response emergencyStop(Request& request);
    
    // Voice control
    static Response toggleVoiceControl(Request& request);
    static Response getVoiceStatus(Request& request);
    
    // Chat/AI endpoints
    static Response sendChatMessage(Request& request);
    static Response getChatHistory(Request& request);

private:
    // Helper methods
    static bool validateMotorSpeed(int speed);
    static bool validateServoAngle(int angle);
    static Utils::SpiJsonDocument createErrorResponse(const String& message);
    static Utils::SpiJsonDocument createSuccessResponse(const String& message = "");
};

#endif // ROBOT_CONTROLLER_H
