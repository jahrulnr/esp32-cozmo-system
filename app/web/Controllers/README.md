# How to Build Controllers

This guide explains how to create and implement controllers for the ESP32 Cozmo Robot System using the ESP32-MVC-Framework. Controllers handle the business logic for HTTP requests and API endpoints, providing a clean separation between routing and application logic.

## Overview

Controllers in the ESP32 Cozmo system follow the MVC (Model-View-Controller) pattern, where:
- **Controllers**: Handle HTTP requests and business logic
- **Models**: Data access layer (User, Configuration, etc.)
- **Views**: JSON responses or HTML templates served to clients

## Quick Start

### Step 1: Create Controller Header File

Create a header file (`.h`) that defines your controller class:

```cpp
#ifndef MY_CONTROLLER_H
#define MY_CONTROLLER_H

#include "Http/Controller.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include <ArduinoJson.h>

class MyController : public Controller {
public:
    // Static methods for handling routes
    static Response handleGet(Request& request);
    static Response handlePost(Request& request);
    
    // Instance methods for complex operations
    Response processData(Request& request);
    
private:
    // Helper methods
    static bool validateInput(const String& input);
    static JsonDocument formatResponse(const String& data);
};

#endif
```

### Step 2: Implement Controller Logic

Create the implementation file (`.cpp`):

```cpp
#include "MyController.h"

Response MyController::handleGet(Request& request) {
    JsonDocument response;
    response["success"] = true;
    response["message"] = "Hello from MyController";
    response["timestamp"] = millis();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response MyController::handlePost(Request& request) {
    String input = request.input("data");
    
    if (!validateInput(input)) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "Invalid input data";
        
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    // Process the input
    JsonDocument result = formatResponse(input);
    
    return Response(request.getServerRequest())
        .status(200)
        .json(result);
}

bool MyController::validateInput(const String& input) {
    return input.length() > 0 && input.length() < 100;
}

JsonDocument MyController::formatResponse(const String& data) {
    JsonDocument response;
    response["success"] = true;
    response["processed_data"] = data;
    response["length"] = data.length();
    return response;
}
```

### Step 3: Register Routes

Use your controller in the routing system:

```cpp
#include "Controllers/MyController.h"

void registerApiRoutes(Router* router) {
    router->group("/api/v1", [&](Router& api) {
        api.get("/my-endpoint", [](Request& request) -> Response {
            return MyController::handleGet(request);
        });
        
        api.post("/my-endpoint", [](Request& request) -> Response {
            return MyController::handlePost(request);
        });
    });
}
```

## Controller Architecture Patterns

### 1. Static Method Controllers (Recommended)

Best for simple, stateless operations:

```cpp
class StaticController : public Controller {
public:
    static Response getInfo(Request& request) {
        JsonDocument info;
        info["system"] = "ESP32 Cozmo";
        info["version"] = "1.0.0";
        
        return Response(request.getServerRequest()).json(info);
    }
    
    static Response updateSettings(Request& request) {
        String key = request.input("key");
        String value = request.input("value");
        
        // Validate and store settings
        if (Configuration::set(key, value)) {
            return Response(request.getServerRequest())
                .json("{\"success\": true}");
        }
        
        return Response(request.getServerRequest())
            .status(500)
            .json("{\"success\": false}");
    }
};
```

### 2. Instance-Based Controllers

Better for complex operations requiring state:

```cpp
class ComplexController : public Controller {
private:
    String currentUser;
    JsonDocument cache;
    
public:
    ComplexController() {
        currentUser = "";
        cache.clear();
    }
    
    Response handleAuthenticated(Request& request) {
        currentUser = AuthController::getCurrentUserUsername(request);
        
        if (currentUser.isEmpty()) {
            return unauthorized(request);
        }
        
        return processUserRequest(request);
    }
    
private:
    Response unauthorized(Request& request) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "Authentication required";
        
        return Response(request.getServerRequest())
            .status(401)
            .json(error);
    }
    
    Response processUserRequest(Request& request) {
        // Complex processing with user context
        JsonDocument result;
        result["user"] = currentUser;
        result["data"] = "processed data";
        
        return Response(request.getServerRequest()).json(result);
    }
};
```

## Real-World Examples

### 1. Robot Control Controller

```cpp
#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include "Http/Controller.h"
#include "../../lib/Motors/MotorControl.h"
#include "../../lib/Sensors/DistanceSensor.h"

class RobotController : public Controller {
public:
    // Movement controls
    static Response moveForward(Request& request);
    static Response moveBackward(Request& request);
    static Response turnLeft(Request& request);
    static Response turnRight(Request& request);
    static Response stopMovement(Request& request);
    
    // Sensor readings
    static Response getSensorData(Request& request);
    static Response getDistance(Request& request);
    static Response getOrientation(Request& request);
    
    // Robot status
    static Response getRobotStatus(Request& request);
    static Response getBatteryLevel(Request& request);
    
private:
    static bool isMotorEnabled();
    static bool validateMovementParams(Request& request);
    static void updateManualControlTime();
    static JsonDocument formatSensorData();
};

#endif
```

Implementation:

```cpp
#include "RobotController.h"
#include "../../setup/setup.h"

Response RobotController::moveForward(Request& request) {
    if (!isMotorEnabled()) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "Motor system disabled";
        
        return Response(request.getServerRequest())
            .status(503)
            .json(error);
    }
    
    if (!validateMovementParams(request)) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "Invalid movement parameters";
        
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    int speed = request.input("speed", "50").toInt();
    int duration = request.input("duration", "1000").toInt();
    
    // Execute movement
    motors->forward(speed);
    updateManualControlTime();
    
    if (duration > 0) {
        delay(duration);
        motors->stop();
    }
    
    JsonDocument response;
    response["success"] = true;
    response["action"] = "forward";
    response["speed"] = speed;
    response["duration"] = duration;
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response RobotController::getSensorData(Request& request) {
    JsonDocument response;
    response["success"] = true;
    response["sensors"] = formatSensorData();
    response["timestamp"] = millis();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

bool RobotController::isMotorEnabled() {
    return MOTOR_ENABLED && motors != nullptr;
}

bool RobotController::validateMovementParams(Request& request) {
    String speedStr = request.input("speed", "50");
    String durationStr = request.input("duration", "1000");
    
    int speed = speedStr.toInt();
    int duration = durationStr.toInt();
    
    return (speed >= 0 && speed <= 100) && (duration >= 0 && duration <= 10000);
}

void RobotController::updateManualControlTime() {
    extern unsigned long lastManualControlTime;
    lastManualControlTime = millis();
}

JsonDocument RobotController::formatSensorData() {
    JsonDocument sensors;
    
    if (DISTANCE_ENABLED && distanceSensor) {
        sensors["distance"] = distanceSensor->readDistance();
    }
    
    if (ORIENTATION_ENABLED && orientationSensor) {
        auto orientation = orientationSensor->readOrientation();
        sensors["orientation"]["x"] = orientation.x;
        sensors["orientation"]["y"] = orientation.y;
        sensors["orientation"]["z"] = orientation.z;
    }
    
    if (TEMPERATURE_ENABLED && temperatureSensor) {
        sensors["temperature"] = temperatureSensor->readTemperature();
    }
    
    return sensors;
}
```

### 2. File Management Controller

```cpp
#ifndef FILE_CONTROLLER_H
#define FILE_CONTROLLER_H

#include "Http/Controller.h"
#include <SPIFFS.h>

class FileController : public Controller {
public:
    // File operations
    static Response listFiles(Request& request);
    static Response getFile(Request& request);
    static Response uploadFile(Request& request);
    static Response deleteFile(Request& request);
    
    // Directory operations
    static Response createDirectory(Request& request);
    static Response listDirectory(Request& request);
    
    // System info
    static Response getStorageInfo(Request& request);
    
private:
    static bool isValidPath(const String& path);
    static bool isAllowedFileType(const String& filename);
    static String sanitizePath(const String& path);
    static JsonDocument formatFileInfo(const String& path);
};

#endif
```

Implementation:

```cpp
#include "FileController.h"

Response FileController::listFiles(Request& request) {
    String directory = request.input("directory", "/");
    directory = sanitizePath(directory);
    
    if (!isValidPath(directory)) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "Invalid directory path";
        
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    JsonDocument response;
    JsonArray files = response["files"].to<JsonArray>();
    
    File dir = SPIFFS.open(directory);
    if (!dir || !dir.isDirectory()) {
        response["success"] = false;
        response["message"] = "Directory not found";
        
        return Response(request.getServerRequest())
            .status(404)
            .json(response);
    }
    
    File file = dir.openNextFile();
    while (file) {
        JsonObject fileInfo = files.add<JsonObject>();
        fileInfo["name"] = String(file.name());
        fileInfo["size"] = file.size();
        fileInfo["is_directory"] = file.isDirectory();
        
        file = dir.openNextFile();
    }
    
    response["success"] = true;
    response["directory"] = directory;
    response["count"] = files.size();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response FileController::uploadFile(Request& request) {
    String filename = request.input("filename");
    String content = request.input("content");
    
    if (!isAllowedFileType(filename)) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "File type not allowed";
        
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    String path = "/" + sanitizePath(filename);
    
    File file = SPIFFS.open(path, "w");
    if (!file) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "Failed to create file";
        
        return Response(request.getServerRequest())
            .status(500)
            .json(error);
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    JsonDocument response;
    response["success"] = true;
    response["filename"] = filename;
    response["bytes_written"] = bytesWritten;
    response["path"] = path;
    
    return Response(request.getServerRequest())
        .status(201)
        .json(response);
}

bool FileController::isValidPath(const String& path) {
    return path.startsWith("/") && 
           !path.contains("..") && 
           path.length() < 256;
}

bool FileController::isAllowedFileType(const String& filename) {
    return filename.endsWith(".txt") || 
           filename.endsWith(".json") || 
           filename.endsWith(".html") || 
           filename.endsWith(".css") || 
           filename.endsWith(".js");
}

String FileController::sanitizePath(const String& path) {
    String cleaned = path;
    cleaned.replace("../", "");
    cleaned.replace("..\\", "");
    
    if (!cleaned.startsWith("/")) {
        cleaned = "/" + cleaned;
    }
    
    return cleaned;
}
```

## Authentication Integration

### Using AuthController for Protected Routes

```cpp
class ProtectedController : public Controller {
public:
    static Response protectedAction(Request& request) {
        // Check authentication
        User* user = AuthController::getCurrentUser(request);
        if (!user) {
            JsonDocument error;
            error["success"] = false;
            error["message"] = "Authentication required";
            
            return Response(request.getServerRequest())
                .status(401)
                .json(error);
        }
        
        // Check permissions
        String username = user->getUsername();
        bool isAdmin = (username == "admin");
        
        if (!isAdmin) {
            JsonDocument error;
            error["success"] = false;
            error["message"] = "Admin access required";
            
            delete user;
            return Response(request.getServerRequest())
                .status(403)
                .json(error);
        }
        
        // Process authorized request
        JsonDocument response;
        response["success"] = true;
        response["user"] = username;
        response["action"] = "performed";
        
        delete user;
        return Response(request.getServerRequest())
            .status(200)
            .json(response);
    }
};
```

## Error Handling Best Practices

### 1. Consistent Error Responses

```cpp
class ErrorController : public Controller {
private:
    static Response createErrorResponse(Request& request, int status, const String& message, const String& code = "") {
        JsonDocument error;
        error["success"] = false;
        error["message"] = message;
        error["timestamp"] = millis();
        
        if (!code.isEmpty()) {
            error["error_code"] = code;
        }
        
        return Response(request.getServerRequest())
            .status(status)
            .json(error);
    }
    
public:
    static Response badRequest(Request& request, const String& message) {
        return createErrorResponse(request, 400, message, "BAD_REQUEST");
    }
    
    static Response unauthorized(Request& request) {
        return createErrorResponse(request, 401, "Authentication required", "UNAUTHORIZED");
    }
    
    static Response forbidden(Request& request) {
        return createErrorResponse(request, 403, "Access denied", "FORBIDDEN");
    }
    
    static Response notFound(Request& request) {
        return createErrorResponse(request, 404, "Resource not found", "NOT_FOUND");
    }
    
    static Response internalError(Request& request, const String& message = "Internal server error") {
        return createErrorResponse(request, 500, message, "INTERNAL_ERROR");
    }
};
```

### 2. Input Validation

```cpp
class ValidationController : public Controller {
private:
    static bool validateRequired(Request& request, const String& field, JsonDocument& error) {
        if (request.input(field).isEmpty()) {
            error["success"] = false;
            error["message"] = "Field '" + field + "' is required";
            error["field"] = field;
            return false;
        }
        return true;
    }
    
    static bool validateLength(Request& request, const String& field, int minLen, int maxLen, JsonDocument& error) {
        String value = request.input(field);
        if (value.length() < minLen || value.length() > maxLen) {
            error["success"] = false;
            error["message"] = "Field '" + field + "' must be between " + String(minLen) + " and " + String(maxLen) + " characters";
            error["field"] = field;
            return false;
        }
        return true;
    }
    
public:
    static Response validateUserInput(Request& request) {
        JsonDocument error;
        
        if (!validateRequired(request, "username", error)) {
            return Response(request.getServerRequest()).status(400).json(error);
        }
        
        if (!validateLength(request, "username", 3, 32, error)) {
            return Response(request.getServerRequest()).status(400).json(error);
        }
        
        if (!validateRequired(request, "password", error)) {
            return Response(request.getServerRequest()).status(400).json(error);
        }
        
        if (!validateLength(request, "password", 6, 128, error)) {
            return Response(request.getServerRequest()).status(400).json(error);
        }
        
        // If all validations pass
        JsonDocument success;
        success["success"] = true;
        success["message"] = "Validation passed";
        
        return Response(request.getServerRequest()).status(200).json(success);
    }
};
```

## Testing Controllers

### Unit Testing Setup

```cpp
// Test helper functions
class TestHelper {
public:
    static Request createMockRequest(const String& method, const String& path) {
        // Create mock request for testing
        // Implementation depends on your testing framework
    }
    
    static void assertJsonResponse(Response& response, int expectedStatus) {
        // Assert response status and JSON format
    }
    
    static JsonDocument parseResponse(Response& response) {
        // Parse JSON response for testing
    }
};

// Example test
void testSystemController() {
    Request mockRequest = TestHelper::createMockRequest("GET", "/api/v1/system/stats");
    Response response = SystemController::getStats(mockRequest);
    
    TestHelper::assertJsonResponse(response, 200);
    
    JsonDocument data = TestHelper::parseResponse(response);
    assert(data["success"] == true);
    assert(data.containsKey("data"));
}
```

## File Structure

```
Controllers/
├── AuthController.h        # Authentication and user management
├── AuthController.cpp      
├── SystemController.h      # System information and configuration
├── SystemController.cpp    
├── RobotController.h       # Robot movement and sensor control (add as needed)
├── RobotController.cpp     
├── FileController.h        # File operations (add as needed)
├── FileController.cpp      
└── README.md               # This guide
```

## Best Practices

### 1. Keep Controllers Focused
- Each controller should handle one domain (auth, system, robot, files)
- Use static methods for stateless operations
- Delegate complex logic to service classes

### 2. Consistent Response Format
```cpp
// Success response
{
    "success": true,
    "data": { ... },
    "timestamp": 1234567890
}

// Error response
{
    "success": false,
    "message": "Error description",
    "error_code": "ERROR_CODE",
    "timestamp": 1234567890
}
```

### 3. Memory Management
- Always delete allocated objects (especially User* instances)
- Use JsonDocument efficiently
- Avoid large objects in static methods

### 4. Security
- Always validate input parameters
- Check authentication for protected endpoints
- Sanitize file paths and user input
- Use proper HTTP status codes

### 5. Error Handling
- Handle all possible error conditions
- Provide meaningful error messages
- Log errors for debugging
- Return appropriate HTTP status codes

This controller architecture provides a solid foundation for building scalable, maintainable web APIs for the ESP32 Cozmo Robot System while following MVC best practices and ESP32 hardware constraints.
