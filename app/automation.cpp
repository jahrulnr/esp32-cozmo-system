#include <Arduino.h>
#include "init.h"

// Simple grid map (for demonstration, 20x20 cells)
#define MAP_SIZE 20
int8_t explorationMap[MAP_SIZE][MAP_SIZE]; // -1: unknown, 0: free, 1: obstacle
int robotX = MAP_SIZE/2, robotY = MAP_SIZE/2; // Start in the middle
float robotHeading = 0; // In degrees

// Flag for learning mode
#define LEARNING_ENABLED true
#define MAP_SAVE_PATH "/data/map_data.txt"

// --- Automation Task for Obstacle Avoidance & Mapping ---
void markMapCell(int x, int y, int8_t value) {
    if (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE) {
        explorationMap[x][y] = value;
    }
}

void resetMap() {
    for (int i = 0; i < MAP_SIZE; ++i)
        for (int j = 0; j < MAP_SIZE; ++j)
            explorationMap[i][j] = -1;
    robotX = MAP_SIZE/2;
    robotY = MAP_SIZE/2;
    robotHeading = 0;
}

// Save the current map to a file for learning
bool saveMapToFile() {
    #if LEARNING_ENABLED
    static Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for map saving");
        return false;
    }
    
    // Create directory if it doesn't exist
    String dirPath = "/data";
    if (!fileManager.exists(dirPath)) {
        fileManager.createDir(dirPath);
    }
    
    // Format map as JSON
    Utils::SpiJsonDocument mapData;
    JsonArray mapArray = mapData.to<JsonArray>();
    
    for (int i = 0; i < MAP_SIZE; ++i) {
        JsonArray row = mapArray.add<JsonArray>();
        for (int j = 0; j < MAP_SIZE; ++j) {
            row.add(explorationMap[i][j]);
        }
    }
    
    // Add robot position
    JsonObject robotPos = mapData.add<JsonObject>();
    robotPos["x"] = robotX;
    robotPos["y"] = robotY;
    robotPos["heading"] = robotHeading;
    robotPos["timestamp"] = millis();
    
    // Convert to string
    String mapJson;
    serializeJson(mapData, mapJson);
    
    // Save to file
    bool success = fileManager.writeFile(MAP_SAVE_PATH, mapJson);
    if (success) {
        logger->debug("Map saved to " + String(MAP_SAVE_PATH));
    } else {
        logger->error("Failed to save map to file");
    }
    return success;
    #else
    return false;
    #endif
}

// Load map from file (for continuing exploration)
bool loadMapFromFile() {
    #if LEARNING_ENABLED
    static Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for map loading");
        return false;
    }
    
    if (!fileManager.exists(MAP_SAVE_PATH)) {
        logger->warning("Map file does not exist, using default map");
        return false;
    }
    
    String mapJson = fileManager.readFile(MAP_SAVE_PATH);
    if (mapJson.length() == 0) {
        logger->error("Empty map file");
        return false;
    }
    
    Utils::SpiJsonDocument mapData;
    DeserializationError error = deserializeJson(mapData, mapJson);
    
    if (error) {
        logger->error("Failed to parse map JSON");
        return false;
    }
    
    // Extract map data
    if (mapData.is<JsonArray>() && mapData.size() >= MAP_SIZE + 1) { // +1 for robot position
        // Load map cells
        for (int i = 0; i < MAP_SIZE; i++) {
            if (mapData[i].is<JsonArray>() && mapData[i].size() == MAP_SIZE) {
                for (int j = 0; j < MAP_SIZE; j++) {
                    explorationMap[i][j] = mapData[i][j];
                }
            }
        }
        
        // Load robot position from the last element
        JsonObject robotPos = mapData[MAP_SIZE];
        if (!robotPos.isNull()) {
            robotX = robotPos["x"] | MAP_SIZE/2;
            robotY = robotPos["y"] | MAP_SIZE/2;
            robotHeading = robotPos["heading"] | 0.0f;
        }
        
        logger->info("Map loaded successfully from file");
        return true;
    }
    
    logger->warning("Invalid map format in file");
    return false;
    #else
    return false;
    #endif
}

// Return the map as JSON for visualization
String getMapAsJson() {
    Utils::SpiJsonDocument mapData;
    
    // Create map data structure
    JsonObject root = mapData.to<JsonObject>();
    root["size"] = MAP_SIZE;
    root["x"] = robotX;
    root["y"] = robotY;
    root["heading"] = robotHeading;
    
    // Create the map grid
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            root["grid"][i][j] = explorationMap[i][j];
        }
    }
    
    // Serialize to string
    String jsonStr;
    serializeJson(mapData, jsonStr);
    return jsonStr;
}

// Get a GPT learning prompt based on map data
String getMapAsPrompt() {
    String prompt = "I'm a robot exploring a space. My current map looks like this:\n";
    
    // Add map to prompt (simplified)
    for (int i = 0; i < MAP_SIZE; ++i) {
        String row;
        for (int j = 0; j < MAP_SIZE; ++j) {
            if (i == robotY && j == robotX) {
                row += "R"; // Robot position
            } else if (explorationMap[i][j] == 1) {
                row += "X"; // Obstacle
            } else if (explorationMap[i][j] == 0) {
                row += "."; // Free space
            } else {
                row += "?"; // Unknown
            }
        }
        prompt += row + "\n";
    }
    
    prompt += "I'm facing " + String(robotHeading) + " degrees. What should I do next?";
    return prompt;
}

// Use GPT to provide navigation guidance (if enabled)
void askGPTForNavigation() {
    #if LEARNING_ENABLED && GPT_ENABLED
    // Create a prompt based on the current map
    String prompt = getMapAsPrompt();
    
    // Send to GPT and store response
    sendGPT(prompt, [](const String& gptResponse) {
        // This could be used to influence the robot's decisions
        // For now, we'll just log the response
        logger->info("GPT navigation suggestion: " + gptResponse);
        
        if (screen) {
            screen->mutexClear();
            screen->drawCenteredText(20, "GPT: " + gptResponse.substring(0, 30) + "...");
            screen->mutexUpdate();
        }
    });
    #endif
}

void automationTask(void* parameter) {
    resetMap();
    
    // Try to load saved map if available
    #if LEARNING_ENABLED
    loadMapFromFile();
    #endif
    
    logger->info("Automation task started (obstacle avoidance & mapping)");
    
    int moveStep = 1; // 1 cell per move
    int moveDuration = 400; // ms per move
    int turnDuration = 350; // ms per 90 deg turn (adjust as needed)
    int8_t lastObstacle = 0;
    unsigned long lastMapSaveTime = 0;
    unsigned long lastGPTConsultTime = 0;
    
    while (true) {
        // 1. Check for obstacle
        float distance = distanceSensor ? distanceSensor->measureDistance() : -1;
        bool obstacle = (distance > 0 && distance < 20);
        
        // 2. Mark map
        if (obstacle) {
            // Mark cell in front as obstacle
            int dx = round(cos(robotHeading * DEG_TO_RAD));
            int dy = round(sin(robotHeading * DEG_TO_RAD));
            markMapCell(robotX + dx, robotY + dy, 1);
        } else {
            // Mark cell in front as free
            int dx = round(cos(robotHeading * DEG_TO_RAD));
            int dy = round(sin(robotHeading * DEG_TO_RAD));
            markMapCell(robotX + dx, robotY + dy, 0);
        }
        
        // 3. Obstacle avoidance logic
        if (obstacle) {
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Surprised();
            motors->stop();
            vTaskDelay(pdMS_TO_TICKS(200));
            // Randomly turn left or right
            int turnDir = random(0, 2) == 0 ? Motors::MotorControl::LEFT : Motors::MotorControl::RIGHT;
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Focused();
            motors->move((Motors::MotorControl::Direction)turnDir, turnDuration);
            // Update heading
            robotHeading += (turnDir == Motors::MotorControl::LEFT ? -90 : 90);
            if (robotHeading < 0) robotHeading += 360;
            if (robotHeading >= 360) robotHeading -= 360;
            vTaskDelay(pdMS_TO_TICKS(turnDuration + 100));
        } else {
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Happy();
            // Move forward
            motors->move(Motors::MotorControl::FORWARD, moveDuration);
            // Update position
            int dx = round(cos(robotHeading * DEG_TO_RAD));
            int dy = round(sin(robotHeading * DEG_TO_RAD));
            robotX += dx * moveStep;
            robotY += dy * moveStep;
            vTaskDelay(pdMS_TO_TICKS(moveDuration + 50));
        }
        
        // 4. Learning-related activities
        #if LEARNING_ENABLED
        // Save map periodically (every 30 seconds)
        if (millis() - lastMapSaveTime > 30000) {
            saveMapToFile();
            lastMapSaveTime = millis();
        }
        
        // Consult GPT occasionally for navigation advice (every 2 minutes)
        if (millis() - lastGPTConsultTime > 120000) {
            askGPTForNavigation();
            lastGPTConsultTime = millis();
        }
        #endif
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}