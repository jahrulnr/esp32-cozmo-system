# How to Make Routers

This guide explains how to create and implement routers for the ESP32 Cozmo Robot System using the ESP32-MVC-Framework. Learn how to build web routes, API endpoints, and WebSocket connections from scratch.

## Quick Start Guide

The routing system supports three types of routes:
- **Web Routes**: HTML pages, assets, and user-facing endpoints
- **API Routes**: RESTful JSON APIs with middleware support  
- **WebSocket Routes**: Real-time bidirectional communication

Follow this guide to create your own custom routes for the robot system.

## File Structure

```
Routes/
├── routes.h        # Header file with route registration declarations
├── web.cpp         # Web page routes and authentication endpoints
├── api.cpp         # RESTful API routes with versioning
├── websocket.cpp   # WebSocket connection handling
└── README.md       # This documentation file
```

## How to Create Routes

### Step 1: Set Up Route Registration

First, declare your route registration function in `routes.h`:

```cpp
#ifndef ROUTES_H
#define ROUTES_H

#include <MVCFramework.h>
#include "Routing/Router.h"

// Add your custom route registration function
void registerCustomRoutes(Router* router);

#endif
```

### Step 2: Create Your Route File

Create a new `.cpp` file (e.g., `custom.cpp`) and implement your routes:

```cpp
#include "routes.h"

void registerCustomRoutes(Router* router) {
    // Your routes go here
}
```

### Step 3: Register Routes in Main Application

In your main application, call the registration function:

```cpp
#include "Routes/routes.h"

void setup() {
    Router* router = new Router();
    
    // Register your custom routes
    registerCustomRoutes(router);
    
    // Start the server
    server.start();
}
```

## Creating Web Routes

Web routes handle traditional HTTP requests and serve HTML pages or assets.

### Basic GET Route

```cpp
void registerCustomRoutes(Router* router) {
    // Simple GET route
    router->get("/hello", [](Request& request) -> Response {
        return Response(request.getServerRequest())
            .html("<h1>Hello from Cozmo!</h1>");
    });
}
```

### Route with Parameters

```cpp
// Route with URL parameter
router->get("/robot/{action}", [](Request& request) -> Response {
    String action = request.route("action");
    
    String html = "<h1>Robot Action: " + action + "</h1>";
    return Response(request.getServerRequest())
        .html(html);
});
```

### Serving Files from SPIFFS

```cpp
// Serve static files
router->get("/files/{filename}", [](Request& request) -> Response {
    String filename = request.route("filename");
    String path = "/data/" + filename;
    
    if (SPIFFS.exists(path)) {
        return Response(request.getServerRequest())
            .file(path);
    }
    
    return Response(request.getServerRequest())
        .status(404)
        .html("<h1>File not found</h1>");
});
```

### POST Route with Form Data

```cpp
// Handle form submissions
router->post("/control", [](Request& request) -> Response {
    String command = request.input("command");
    String speed = request.input("speed");
    
    // Process the robot command
    processRobotCommand(command, speed.toInt());
    
    return Response(request.getServerRequest())
        .redirect("/dashboard");
});
```

## Creating API Routes

API routes return JSON responses and typically use middleware for authentication and data processing.

### Basic API Endpoint

```cpp
void registerCustomRoutes(Router* router) {
    // API route group
    router->group("/api/v1", [&](Router& api) {
        // Add JSON middleware
        api.middleware({"json"});
        
        // Simple API endpoint
        api.get("/status", [](Request& request) -> Response {
            Utils::SpiJsonDocument response;
            response["status"] = "online";
            response["timestamp"] = millis();
            response["free_memory"] = ESP.getFreeHeap();
            
            return Response(request.getServerRequest())
                .status(200)
                .json(response);
        });
    });
}
```

### API with Authentication

```cpp
// Protected API endpoint
router->group("/api/v1", [&](Router& api) {
    api.middleware({"json", "auth"});
    
    api.post("/move", [](Request& request) -> Response {
        Utils::SpiJsonDocument requestData = request.json();
        
        String direction = requestData["direction"];
        int duration = requestData["duration"];
        
        // Execute robot movement
        bool success = moveRobot(direction, duration);
        
        Utils::SpiJsonDocument response;
        response["success"] = success;
        response["message"] = success ? "Movement executed" : "Movement failed";
        
        return Response(request.getServerRequest())
            .status(success ? 200 : 400)
            .json(response);
    });
});
```

### API with Route Groups and Middleware

```cpp
// Nested route groups with different middleware
router->group("/api/v1", [&](Router& api) {
    api.middleware({"cors", "json"});
    
    // Public endpoints (no auth required)
    api.get("/info", [](Request& request) -> Response {
        Utils::SpiJsonDocument info;
        info["robot_name"] = "Cozmo";
        info["version"] = "1.0.0";
        return Response(request.getServerRequest()).json(info);
    });
    
    // Protected endpoints
    api.group("/robot", [&](Router& robot) {
        robot.middleware({"auth"});
        
        robot.get("/sensors", [](Request& request) -> Response {
            Utils::SpiJsonDocument sensors;
            sensors["temperature"] = readTemperature();
            sensors["distance"] = readDistance();
            return Response(request.getServerRequest()).json(sensors);
        });
        
        robot.post("/speak", [](Request& request) -> Response {
            Utils::SpiJsonDocument data = request.json();
            String text = data["text"];
            
            bool success = speakText(text);
            
            Utils::SpiJsonDocument response;
            response["success"] = success;
            return Response(request.getServerRequest()).json(response);
        });
    });
});
```

## Creating WebSocket Routes

WebSocket routes enable real-time bidirectional communication between the robot and clients.

### Basic WebSocket Setup

```cpp
void registerCustomRoutes(Router* router) {
    router->websocket("/ws/robot")
        .onConnect([](WebSocketRequest& request) {
            Serial.printf("[WebSocket] Robot client %u connected\n", request.clientId());
            
            // Send initial robot status
            Utils::SpiJsonDocument status;
            status["type"] = "robot_status";
            status["battery"] = getBatteryLevel();
            status["mode"] = getCurrentMode();
            
            String message;
            serializeJson(status, message);
            request.send(message);
        })
        .onDisconnect([](WebSocketRequest& request) {
            Serial.printf("[WebSocket] Robot client %u disconnected\n", request.clientId());
        })
        .onMessage([](WebSocketRequest& request, const String& message) {
            Utils::SpiJsonDocument doc;
            DeserializationError error = deserializeJson(doc, message);
            
            if (error) {
                Serial.println("[WebSocket] Invalid JSON received");
                return;
            }
            
            handleRobotCommand(request, doc);
        });
}
```

### WebSocket Command Handler

```cpp
void handleRobotCommand(WebSocketRequest& request, Utils::SpiJsonDocument& command) {
    String type = command["type"];
    Utils::SpiJsonDocument response;
    
    if (type == "move") {
        String direction = command["direction"];
        int speed = command["speed"];
        
        bool success = executeMovement(direction, speed);
        
        response["type"] = "move_response";
        response["success"] = success;
        response["direction"] = direction;
        
    } else if (type == "get_sensors") {
        response["type"] = "sensor_data";
        response["temperature"] = readTemperature();
        response["distance"] = readDistance();
        response["orientation"] = readOrientation();
        
    } else if (type == "ping") {
        response["type"] = "pong";
        response["timestamp"] = millis();
        
    } else {
        response["type"] = "error";
        response["message"] = "Unknown command type";
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    request.send(responseStr);
}
```

### Broadcasting to All Clients

```cpp
// Store WebSocket clients for broadcasting
std::vector<uint32_t> connectedClients;

router->websocket("/ws/notifications")
    .onConnect([](WebSocketRequest& request) {
        connectedClients.push_back(request.clientId());
    })
    .onDisconnect([](WebSocketRequest& request) {
        auto it = std::find(connectedClients.begin(), connectedClients.end(), request.clientId());
        if (it != connectedClients.end()) {
            connectedClients.erase(it);
        }
    });

// Function to broadcast to all clients
void broadcastToAll(const String& message) {
    for (uint32_t clientId : connectedClients) {
        // Send to specific client (implementation depends on framework)
        webSocketServer.sendTXT(clientId, message);
    }
}
```

## Advanced Routing Techniques

### Route Middleware

Create custom middleware for common functionality:

```cpp
// Custom authentication middleware
class AuthMiddleware {
public:
    static bool handle(Request& request) {
        String token = request.header("Authorization");
        
        if (token.isEmpty()) {
            return false; // Reject request
        }
        
        // Validate token
        return validateToken(token);
    }
};

// Apply middleware to routes
router->group("/api", [&](Router& api) {
    api.middleware({"auth"}); // Use auth middleware
    
    // Your protected routes here
});
```

### Route Model Binding

Automatically bind route parameters to objects:

```cpp
struct RobotCommand {
    String action;
    int duration;
    int speed;
};

router->post("/api/command", [](Request& request) -> Response {
    RobotCommand cmd;
    Utils::SpiJsonDocument data = request.json();
    
    cmd.action = data["action"];
    cmd.duration = data["duration"];
    cmd.speed = data["speed"];
    
    executeCommand(cmd);
    
    return Response(request.getServerRequest())
        .json({{"success", true}});
});
```

### Error Handling

```cpp
router->get("/api/sensors/{sensor}", [](Request& request) -> Response {
    String sensorName = request.route("sensor");
    
    try {
        float value = readSensor(sensorName);
        
        Utils::SpiJsonDocument response;
        response["sensor"] = sensorName;
        response["value"] = value;
        response["timestamp"] = millis();
        
        return Response(request.getServerRequest())
            .status(200)
            .json(response);
            
    } catch (const std::exception& e) {
        Utils::SpiJsonDocument error;
        error["error"] = "Sensor not found";
        error["message"] = e.what();
        
        return Response(request.getServerRequest())
            .status(404)
            .json(error);
    }
});
```

## Testing Your Routes

### Web Route Testing

Test web routes using a browser or curl:

```bash
# Test basic GET route
curl http://robot-ip/hello

# Test route with parameters
curl http://robot-ip/robot/dance

# Test POST route with form data
curl -X POST -d "command=forward&speed=50" http://robot-ip/control
```

### API Route Testing

Test API endpoints with JSON data:

```bash
# Test API status endpoint
curl -H "Content-Type: application/json" http://robot-ip/api/v1/status

# Test authenticated API endpoint
curl -H "Authorization: Bearer your-token" \
     -H "Content-Type: application/json" \
     -d '{"direction":"forward","duration":1000}' \
     http://robot-ip/api/v1/move
```

### WebSocket Testing

Test WebSocket connections using JavaScript:

```javascript
const ws = new WebSocket('ws://robot-ip/ws/robot');

ws.onopen = function() {
    console.log('Connected to robot');
    
    // Send a command
    ws.send(JSON.stringify({
        type: 'move',
        direction: 'forward',
        speed: 50
    }));
};

ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Robot response:', data);
};
```

## Best Practices

### 1. Use Route Groups for Organization

```cpp
// Group related routes together
router->group("/api/v1/robot", [&](Router& robot) {
    robot.get("/status", statusHandler);
    robot.post("/move", moveHandler);
    robot.post("/speak", speakHandler);
});
```

### 2. Implement Proper Error Handling

```cpp
router->get("/api/sensor", [](Request& request) -> Response {
    try {
        float value = readSensor();
        return Response(request.getServerRequest())
            .json({{"value", value}});
    } catch (...) {
        return Response(request.getServerRequest())
            .status(500)
            .json({{"error", "Sensor read failed"}});
    }
});
```

### 3. Use Middleware for Cross-Cutting Concerns

```cpp
// Apply middleware to entire route groups
router->group("/api", [&](Router& api) {
    api.middleware({"cors", "json", "auth", "ratelimit"});
    // All routes in this group will use these middleware
});
```

### 4. Validate Input Data

```cpp
router->post("/api/move", [](Request& request) -> Response {
    Utils::SpiJsonDocument data = request.json();
    
    if (!data.containsKey("direction") || !data.containsKey("speed")) {
        return Response(request.getServerRequest())
            .status(400)
            .json({{"error", "Missing required fields"}});
    }
    
    // Process valid request
});
```

### 5. Use Consistent Response Formats

```cpp
// Standard success response
Utils::SpiJsonDocument successResponse;
successResponse["success"] = true;
successResponse["data"] = responseData;
successResponse["timestamp"] = millis();

// Standard error response  
Utils::SpiJsonDocument errorResponse;
errorResponse["success"] = false;
errorResponse["error"] = "Error message";
errorResponse["timestamp"] = millis();
```

## Troubleshooting

### Common Issues

1. **Routes not working**: Ensure routes are registered in the correct order
2. **JSON parsing errors**: Check content-type headers and JSON format
3. **WebSocket disconnections**: Implement ping/pong for connection health
4. **Memory issues**: Use references and avoid creating large objects in handlers
5. **CORS errors**: Add CORS middleware to API routes

### Debug Tips

```cpp
// Add logging to route handlers
router->get("/debug", [](Request& request) -> Response {
    Serial.println("Debug route accessed");
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    
    return Response(request.getServerRequest())
        .json({{"debug", "ok"}});
});
```

## File Structure

```
Routes/
├── routes.h        # Route registration declarations
├── web.cpp         # Web page routes and authentication
├── api.cpp         # RESTful API routes with middleware
├── websocket.cpp   # WebSocket connection handling
├── custom.cpp      # Your custom routes (add as needed)
└── README.md       # This guide
```
