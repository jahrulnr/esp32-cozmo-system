#include <Arduino.h>
#include "app.h"
#include "lib/Automation/TemperatureBehavior.h"

// Template manager instance
Automation::TemplateManager* templateManager = nullptr;

// Simple grid map (for demonstration, 20x20 cells)
#define MAP_SIZE 20
int8_t explorationMap[MAP_SIZE][MAP_SIZE]; // -1: unknown, 0: free, 1: obstacle, 2: cliff
int robotX = MAP_SIZE/2, robotY = MAP_SIZE/2; // Start in the middle
float robotHeading = 0; // In degrees

// Gyroscope rotation tracking variables
float accumulatedZRotation = 0.0f;     // Accumulated rotation around Z axis in degrees
float lastZGyroValue = 0.0f;           // Previous gyro reading for calculating delta
unsigned long lastGyroReadTime = 0;    // Last time we read the gyro (for time delta)
int fullRotationCount = 0;             // Count of complete 360° rotations
bool inRotationSequence = false;       // Flag to indicate we're tracking a rotation sequence
unsigned long rotationStartTime = 0;   // When the current rotation sequence started
float rotationThreshold = 10.0f;       // Minimum degrees/sec to consider as intentional rotation

// Flag for learning mode
#define LEARNING_ENABLED true
#define MAP_SAVE_PATH "/data/map_data.txt"
#define DEFAULT_AUTOMATION_PATH "/data/default_automation.txt"
#define LEARNING_AUTOMATION_PATH "/data/learning_automation.txt"
#define ROTATION_LEARNING_PATH "/data/rotation_learning.txt"
#define AUTOMATION_INTERVAL 1800000 // 30 minutes in milliseconds

AutomationPattern defaultPattern = {
    "Default Exploration",
    {0, 2, 0, 3, 0, 2, 0, 3, 0, 0},  // Forward, Left, Forward, Right, Forward, Left, Forward, Right, Forward, Forward
    {500, 350, 600, 350, 700, 350, 800, 350, 500, 500},
    10
};

// Function prototypes for rotation tracking
void updateRotationTracking();
bool saveRotationLearningData(int direction, unsigned long duration);

// --- Automation Task for Obstacle Avoidance & Mapping ---
void markMapCell(int x, int y, int8_t value) {
    // value: -1=unknown, 0=free, 1=obstacle, 2=cliff
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
    // Cell values: -1=unknown, 0=free space, 1=obstacle, 2=cliff
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
            } else if (explorationMap[i][j] == 2) {
                row += "C"; // Cliff
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
    
    prompt += "Legend: R=Robot, X=Obstacle, .=Free Space, C=Cliff, ?=Unknown\n";
    prompt += "I'm facing " + String(robotHeading) + " degrees. What should I do next? Remember to avoid cliffs (C) - they are dangerous!";
    return prompt;
}

// Use GPT to provide navigation guidance (if enabled)
void askGPTForNavigation() {
    #if LEARNING_ENABLED && GPT_ENABLED
    if (!gptAdapter || !gptAdapter->isInitialized()) {
        logger->warning("GPT adapter not initialized for navigation guidance");
        return;
    }
    
    // Create a prompt based on the current map
    String prompt = getMapAsPrompt();
    
    // Create specialized navigation context for better command generation
    String navigationContext = "You are the navigation AI for a Cozmo IoT Robot. You specialize in path planning and exploration.\n\n";
    navigationContext += "CURRENT MAP SITUATION:\n" + prompt + "\n\n";
    navigationContext += "YOUR TASK: Analyze the map and create a sequence of movement commands that will help the robot:\n";
    navigationContext += "1. Explore unknown areas (marked as ?)\n";
    navigationContext += "2. ALWAYS avoid cliffs (marked as C) - these are dangerous!\n";
    navigationContext += "3. Navigate around obstacles (marked as X)\n";
    navigationContext += "4. Return to explored areas only when necessary\n\n";
    
    navigationContext += "COMMAND FORMAT RULES:\n";
    navigationContext += "- Use EXACTLY these movement commands: [MOVE_FORWARD=Xs], [MOVE_BACKWARD=Xs], [TURN_LEFT=Xs], [TURN_RIGHT=Xs]\n";
    navigationContext += "- X must be a number between 3-15 (seconds)\n";
    navigationContext += "- Provide EXACTLY 3-5 movement commands in sequence\n";
    navigationContext += "- Format your response as a series of commands followed by a brief explanation\n";
    navigationContext += "- Example: \"[MOVE_FORWARD=5s][TURN_RIGHT=3s][MOVE_FORWARD=10s] This path avoids the cliff to your left and explores the unknown area ahead.\"\n";
    
    // Send direct request to GPT adapter with specialized navigation context
    if (gptAdapter) {
        // Send request with the specialized navigation context
        gptAdapter->sendPrompt("Generate movement commands for the robot based on the map", navigationContext, 
        [](const String& gptResponse) {
            // Log the response
            logger->info("GPT navigation suggestion: " + gptResponse);
            
            // Create an automation pattern from the response
            AutomationPattern gptPattern = createAutomationFromGPT(gptResponse);
            
            // Save the pattern if it has valid steps
            if (gptPattern.stepCount > 0) {
                saveLearningAutomation(gptPattern);
            }
            
            if (screen) {
                screen->mutexClear();
                screen->drawCenteredText(20, "GPT: " + gptResponse.substring(0, 30) + "...");
                screen->mutexUpdate();
            }
            
            // Save this full response to a special file for offline use
            static Utils::FileManager fileManager;
            if (fileManager.init()) {
                fileManager.writeFile("/data/last_navigation.txt", gptResponse);
            }
        });
    }
    #else
    logger->info("GPT navigation unavailable - internet or GPT disabled");
    #endif
}

// Convert GPT navigation suggestions into an automation pattern
AutomationPattern createAutomationFromGPT(const String& gptResponse) {
    AutomationPattern pattern;
    pattern.name = "GPT-Generated Pattern " + String(millis() % 10000);
    pattern.stepCount = 0;
    
    // Advanced parsing of movement commands using regex pattern matching
    std::regex moveForwardRegex("\\[MOVE_FORWARD=([0-9]+)s\\]");
    std::regex moveBackwardRegex("\\[MOVE_BACKWARD=([0-9]+)s\\]");
    std::regex turnLeftRegex("\\[TURN_LEFT=([0-9]+)s\\]");
    std::regex turnRightRegex("\\[TURN_RIGHT=([0-9]+)s\\]");
    
    std::string response = gptResponse.c_str();
    std::smatch matches;
    std::string::const_iterator searchStart(response.cbegin());
    
    // Extract each movement command from the response
    while (pattern.stepCount < 10 && searchStart != response.cend()) {
        bool foundMatch = false;
        
        // Check for MOVE_FORWARD
        if (std::regex_search(searchStart, response.cend(), matches, moveForwardRegex)) {
            int duration = std::stoi(matches[1]) * 1000; // Convert seconds to milliseconds
            pattern.moveSteps[pattern.stepCount] = 0; // Forward
            pattern.durations[pattern.stepCount] = duration;
            pattern.stepCount++;
            searchStart = matches.suffix().first;
            foundMatch = true;
            logger->debug("Found MOVE_FORWARD command: " + String(duration/1000) + "s");
        }
        // Check for MOVE_BACKWARD
        else if (std::regex_search(searchStart, response.cend(), matches, moveBackwardRegex)) {
            int duration = std::stoi(matches[1]) * 1000;
            pattern.moveSteps[pattern.stepCount] = 1; // Backward
            pattern.durations[pattern.stepCount] = duration;
            pattern.stepCount++;
            searchStart = matches.suffix().first;
            foundMatch = true;
            logger->debug("Found MOVE_BACKWARD command: " + String(duration/1000) + "s");
        }
        // Check for TURN_LEFT
        else if (std::regex_search(searchStart, response.cend(), matches, turnLeftRegex)) {
            int duration = std::stoi(matches[1]) * 1000;
            pattern.moveSteps[pattern.stepCount] = 2; // Left turn
            pattern.durations[pattern.stepCount] = duration;
            pattern.stepCount++;
            searchStart = matches.suffix().first;
            foundMatch = true;
            logger->debug("Found TURN_LEFT command: " + String(duration/1000) + "s");
        }
        // Check for TURN_RIGHT
        else if (std::regex_search(searchStart, response.cend(), matches, turnRightRegex)) {
            int duration = std::stoi(matches[1]) * 1000;
            pattern.moveSteps[pattern.stepCount] = 3; // Right turn
            pattern.durations[pattern.stepCount] = duration;
            pattern.stepCount++;
            searchStart = matches.suffix().first;
            foundMatch = true;
            logger->debug("Found TURN_RIGHT command: " + String(duration/1000) + "s");
        }
        
        if (!foundMatch) break;
    }
    
    // Add a default move forward if no commands were parsed
    if (pattern.stepCount == 0) {
        pattern.moveSteps[0] = 0; // Forward
        pattern.durations[0] = 500;
        pattern.moveSteps[1] = 2; // Turn left
        pattern.durations[1] = 350;
        pattern.moveSteps[2] = 0; // Forward
        pattern.durations[2] = 700;
        pattern.stepCount = 3;
        logger->warning("No movement commands found in GPT response, using default exploration pattern");
    }
    
    logger->info("Created automation pattern from GPT with " + String(pattern.stepCount) + " steps");
    return pattern;
}

// Execute a single step from an automation pattern
void executeAutomationStep(int moveType, int duration) {
    if (!motors) return;
    
    // Execute the movement based on the type
    switch(moveType) {
        case 0: // Forward
            motors->move(Motors::MotorControl::FORWARD, duration);
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Happy();
            break;
        case 1: // Backward
            motors->move(Motors::MotorControl::BACKWARD, duration);
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Focused();
            break;
        case 2: // Left turn
            motors->move(Motors::MotorControl::LEFT, duration);
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Skeptic();
            break;
        case 3: // Right turn
            motors->move(Motors::MotorControl::RIGHT, duration);
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Skeptic();
            break;
        case 4: // Gyro-assisted 90° left turn
            rotateWithGyro(-1, 90, duration);
            break;
        case 5: // Gyro-assisted 90° right turn
            rotateWithGyro(1, 90, duration);
            break;
        case 6: // Gyro-assisted 180° turn
            rotateWithGyro(1, 180, duration);
            break;
        case 7: // Full 360° scan clockwise
            perform360Scan(1);
            break;
        case 8: // Full 360° scan counterclockwise
            perform360Scan(-1);
            break;
        default:
            // Invalid movement type
            return;
    }
    
    // Wait for the movement to complete
    vTaskDelay(pdMS_TO_TICKS(duration + 50));
}

// Execute an automation pattern
void runAutomationPattern(const AutomationPattern& pattern) {
    if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Focused();
    logger->info("Running automation pattern: " + pattern.name);
    
    // Execute each step in sequence
    for (int i = 0; i < pattern.stepCount; i++) {
        // Check for cliff/obstacle before each movement
        if (cliffLeftDetector) cliffLeftDetector->update();
        if (cliffRightDetector) cliffRightDetector->update();
        
        // Skip movement if obstacle or cliff detected
        if (cliffDetected()) {
            logger->warning("Skipping automation step due to cliff detection");
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Surprised();
            continue;
        }
        
        float distance = distanceSensor ? distanceSensor->measureDistance() : -1;
        bool obstacle = (distance > 0 && distance < 20);
        
        if (obstacle && pattern.moveSteps[i] == 0) { // Skip forward moves if obstacle detected
            logger->warning("Skipping forward automation step due to obstacle");
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Annoyed();
            continue;
        }
        
        // Execute the step
        executeAutomationStep(pattern.moveSteps[i], pattern.durations[i]);
        logger->debug("Executed automation step " + String(i+1) + "/" + String(pattern.stepCount));
    }
    
    logger->info("Completed automation pattern: " + pattern.name);
    if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Happy();
}

// Send a specialized navigation request to GPT with custom context
void sendGPTNavigation(const String &prompt, Communication::GPTAdapter::ResponseCallback callback){
	gptRequest *data = new gptRequest{
		prompt: prompt,
		callback: callback,
		saveToLog: GPT_LEARNING_ENABLED
	};

	// Create a specialized task with higher priority for navigation
	xTaskCreate(gptChatTask, "gptNavTask", 20 * 1024, data, 11, &gptTaskHandle);
}

void automationTask(void* parameter) {
    resetMap();
    
    // Initialize template manager for offline actions
    templateManager = new Automation::TemplateManager();
    if (templateManager) {
        templateManager->setDependencies(motors, servos, screen);
        if (templateManager->init()) {
            logger->info("Behavior template manager initialized successfully");
            // Execute a "hello" template to show we're ready
            templateManager->executeRandomTemplate("happy");
        } else {
            logger->warning("Failed to initialize template manager - offline behaviors will be limited");
        }
    }
    
    // Initialize temperature templates
    setupTemperatureTemplates();
    
    // Ensure default automation pattern is saved
    if (!saveDefaultAutomation()) {
        logger->warning("Failed to save default automation pattern, will use hardcoded pattern");
    }
    
    // Try to load saved map if available
    #if LEARNING_ENABLED
    loadMapFromFile();
    #endif
    
    logger->info("Automation task started (obstacle avoidance & mapping)");
    
    int moveStep = 1; // 1 cell per move
    int moveDuration = 5000; // ms per move
    int turnDuration = 350; // ms per 90 deg turn (adjust as needed)
    int8_t lastObstacle = 0;
    unsigned long lastMapSaveTime = 0;
    unsigned long lastGPTConsultTime = 0;
    unsigned long lastAutomationRunTime = 0;
    unsigned long lastTemplateTime = 0;
    unsigned long lastTemperatureCheckTime = 0;
    
    // Load our automation patterns
    AutomationPattern activePattern = defaultPattern;
    bool defaultPatternLoaded = loadAutomationPattern(DEFAULT_AUTOMATION_PATH, activePattern);
    if (!defaultPatternLoaded) {
        logger->info("Using built-in default automation pattern");
    }
    
    while (true) {
        // Update cliff detectors
        if (cliffLeftDetector) cliffLeftDetector->update();
        if (cliffRightDetector) cliffRightDetector->update();
        
        // Update rotation tracking from gyroscope
        updateRotationTracking();
        
        // Check for cliff detection
        bool cliff = cliffDetected();
        
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
        
        // 3. Cliff and obstacle avoidance logic
        if (cliff) {
            // Cliff detected - back up and turn
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Surprised();
            logger->warning("Cliff detected! Backing up");
            
            // Stop immediately and back up
            motors->stop();
            vTaskDelay(pdMS_TO_TICKS(100));
            motors->move(Motors::MotorControl::BACKWARD, moveDuration);
            vTaskDelay(pdMS_TO_TICKS(moveDuration + 50));
            
            // Update position after backing up
            int dx = round(cos(robotHeading * DEG_TO_RAD));
            int dy = round(sin(robotHeading * DEG_TO_RAD));
            robotX -= dx * moveStep; // Move backward
            robotY -= dy * moveStep;
            
            // Turn around (180 degrees) using gyro-assisted rotation if available
            if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Focused();
            
            if (orientation) {
                // Use precision gyro-based rotation
                rotateWithGyro(-1, 180, turnDuration * 2);
            } else {
                // Fallback to timed rotation
                motors->move(Motors::MotorControl::LEFT, turnDuration * 2);
                
                // Update heading (180 degree turn)
                robotHeading += 180;
                if (robotHeading >= 360) robotHeading -= 360;
                
                vTaskDelay(pdMS_TO_TICKS(turnDuration * 2 + 100));
            }
            
            // Mark the cell in front as a cliff (special value 2)
            dx = round(cos((robotHeading - 180) * DEG_TO_RAD)); // Original direction
            dy = round(sin((robotHeading - 180) * DEG_TO_RAD));
            markMapCell(robotX + dx, robotY + dy, 2); // 2 indicates cliff
        }
        else if (obstacle) {
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
        
        // Consult GPT for navigation advice (every 2 minutes when internet is available)
        bool isInternetAvailable = (WiFi.status() == WL_CONNECTED);
        
        if (millis() - lastGPTConsultTime > 120000) {
            if (isInternetAvailable && gptAdapter && gptAdapter->isInitialized()) {
                logger->info("Internet available - asking GPT for navigation advice");
                askGPTForNavigation();
            } else {
                logger->info("No internet connection - skipping GPT navigation query");
            }
            lastGPTConsultTime = millis();
        }
        
        // Check temperature periodically (every 60 seconds)
        if (temperatureSensor && millis() - lastTemperatureCheckTime > 60000) {
            logger->debug("Performing scheduled temperature check");
            checkTemperature();
            lastTemperatureCheckTime = millis();
        }
        
        // Run automation pattern every 30 minutes
        if (millis() - lastAutomationRunTime > AUTOMATION_INTERVAL) {
            logger->info("Running scheduled automation pattern");
            
            // Pattern selection priority:
            // 1. Learning pattern from API if internet available
            // 2. Saved offline navigation pattern
            // 3. Default pattern as fallback
            
            AutomationPattern pattern = activePattern;
            bool patternLoaded = false;
            
            // Try to load the learning pattern first
            if (isInternetAvailable) {
                patternLoaded = loadAutomationPattern(LEARNING_AUTOMATION_PATH, pattern);
                if (patternLoaded) {
                    logger->info("Using learning-based automation pattern");
                }
            }
            
            // If no learning pattern available or no internet, try offline navigation
            if (!patternLoaded) {
                patternLoaded = loadOfflineNavigationPattern(pattern);
                if (patternLoaded) {
                    logger->info("Using offline navigation pattern");
                }
            }
            
            // If still no pattern, use default
            if (!patternLoaded) {
                logger->info("Using default automation pattern (no internet or saved patterns)");
                pattern = activePattern;
            }
            
            lastAutomationRunTime = millis();
        }
        #endif
        
        // Temperature monitoring and response
        #if TEMPERATURE_ENABLED
        if (millis() - lastTemperatureCheckTime > 10000) { // Check every 10 seconds
            float temperature = temperatureSensor ? temperatureSensor->readTemperature() : 0;
            logger->info("Current temperature: " + String(temperature) + " °C");
            
            // Execute temperature-based behaviors
            if (temperature > 30) {
                // High temperature - activate cooling behavior
                logger->warning("High temperature detected! Activating cooling behavior.");
                templateManager->executeTemplate("cooling");
            } else if (temperature < 10) {
                // Low temperature - activate heating behavior
                logger->warning("Low temperature detected! Activating heating behavior.");
                templateManager->executeTemplate("heating");
            }
            
            lastTemperatureCheckTime = millis();
        }
        #endif
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Save the default automation pattern to a file
bool saveDefaultAutomation() {
    static Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for default automation saving");
        return false;
    }
    
    // Create directory if it doesn't exist
    String dirPath = "/data";
    if (!fileManager.exists(dirPath)) {
        fileManager.createDir(dirPath);
    }
    
    // Format pattern as JSON
    Utils::SpiJsonDocument doc;
    doc["name"] = defaultPattern.name;
    doc["stepCount"] = defaultPattern.stepCount;
    
    for (int i = 0; i < defaultPattern.stepCount; i++) {
        doc["moveSteps"].add(defaultPattern.moveSteps[i]);
        doc["durations"].add(defaultPattern.durations[i]);
    }
    
    // Convert to string
    String patternJson;
    serializeJson(doc, patternJson);
    
    // Save to file
    bool success = fileManager.writeFile(DEFAULT_AUTOMATION_PATH, patternJson);
    if (success) {
        logger->debug("Default automation pattern saved to " + String(DEFAULT_AUTOMATION_PATH));
    } else {
        logger->error("Failed to save default automation pattern");
    }
    return success;
}

// Load automation pattern from file
bool loadAutomationPattern(const String& filePath, AutomationPattern& pattern) {
    static Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for automation pattern loading");
        return false;
    }
    
    if (!fileManager.exists(filePath)) {
        logger->warning("Automation pattern file does not exist: " + filePath);
        return false;
    }
    
    String patternJson = fileManager.readFile(filePath);
    if (patternJson.length() == 0) {
        logger->error("Empty automation pattern file: " + filePath);
        return false;
    }
    
    Utils::SpiJsonDocument doc;
    DeserializationError error = deserializeJson(doc, patternJson);
    
    if (error) {
        logger->error("Failed to parse automation pattern JSON");
        return false;
    }
    
    pattern.name = doc["name"].as<String>();
    pattern.stepCount = doc["stepCount"] | 0;
    
    // Ensure we don't exceed array bounds
    pattern.stepCount = min(pattern.stepCount, 10);
    
    // Load move steps and durations
    JsonArray moveStepsArray = doc["moveSteps"].as<JsonArray>();
    JsonArray durationsArray = doc["durations"].as<JsonArray>();
    
    int i = 0;
    for (JsonVariant step : moveStepsArray) {
        if (i < pattern.stepCount) {
            pattern.moveSteps[i] = step.as<int>();
        }
        i++;
    }
    
    i = 0;
    for (JsonVariant duration : durationsArray) {
        if (i < pattern.stepCount) {
            pattern.durations[i] = duration.as<int>();
        }
        i++;
    }
    
    logger->info("Loaded automation pattern '" + pattern.name + "' with " + 
                String(pattern.stepCount) + " steps from " + filePath);
    return true;
}

// Save a learning-based automation pattern from GPT suggestions
bool saveLearningAutomation(AutomationPattern& pattern) {
    static Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for learning automation saving");
        return false;
    }
    
    // Format pattern as JSON
    Utils::SpiJsonDocument doc;
    doc["name"] = pattern.name;
    doc["stepCount"] = pattern.stepCount;
    doc["timestamp"] = millis();
    
    for (int i = 0; i < pattern.stepCount; i++) {
        doc["moveSteps"].add(pattern.moveSteps[i]);
        doc["durations"].add(pattern.durations[i]);
    }
    
    // Convert to string
    String patternJson;
    serializeJson(doc, patternJson);
    
    // Save to file
    bool success = fileManager.writeFile(LEARNING_AUTOMATION_PATH, patternJson);
    if (success) {
        logger->debug("Learning automation pattern saved to " + String(LEARNING_AUTOMATION_PATH));
    } else {
        logger->error("Failed to save learning automation pattern");
    }
    return success;
}

// Helper function to load offline navigation patterns
// This is useful when internet is not available but we have previous GPT responses saved
bool loadOfflineNavigationPattern(AutomationPattern& pattern) {
    static Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for offline navigation");
        return false;
    }
    
    // Check for saved navigation response
    if (!fileManager.exists("/data/last_navigation.txt")) {
        logger->warning("No offline navigation data available");
        return false;
    }
    
    // Load the last navigation response
    String lastNavigation = fileManager.readFile("/data/last_navigation.txt");
    if (lastNavigation.length() == 0) {
        logger->warning("Empty navigation data file");
        return false;
    }
    
    // Try to extract a pattern from the saved response
    pattern = createAutomationFromGPT(lastNavigation);
    
    // Check if we got a valid pattern (more than default)
    if (pattern.stepCount > 0) {
        pattern.name = "Offline Navigation Pattern";
        logger->info("Successfully loaded offline navigation pattern with " + 
                    String(pattern.stepCount) + " steps");
        return true;
    }
    
    return false;
}

// Function to update rotation tracking based on gyroscope data
void updateRotationTracking() {
    if (!orientation) return;

    // Get current time for delta time calculation
    unsigned long currentTime = millis();
    float deltaTime = 0;
    
    // Calculate time difference since last reading
    if (lastGyroReadTime > 0) {
        deltaTime = (currentTime - lastGyroReadTime) / 1000.0f; // Convert to seconds
    }
    lastGyroReadTime = currentTime;
    
    // If this is the first reading or too much time passed, skip accumulation
    if (deltaTime <= 0 || deltaTime > 0.1) return;
    
    // Read current gyroscope data (Z-axis rotation)
    orientation->update();
    float currentZValue = orientation->getZ();
    
    // Calculate rotation delta in degrees
    float rotationDelta = currentZValue * deltaTime;
    
    // Check if this is significant rotation (filter out noise/drift)
    if (fabs(currentZValue) > rotationThreshold) {
        // If not already in a rotation sequence, start one
        if (!inRotationSequence) {
            inRotationSequence = true;
            rotationStartTime = currentTime;
            accumulatedZRotation = 0.0f;
            logger->debug("Starting rotation tracking sequence");
        }
        
        // Accumulate rotation
        accumulatedZRotation += rotationDelta;
        
        // Debug output every ~45 degrees
        if (fabs(accumulatedZRotation) >= 45.0f && 
            fabs(accumulatedZRotation) <= 46.0f) {
            logger->debug("Accumulated rotation: " + String(accumulatedZRotation) + " degrees");
        }
        
        // Check for full 360-degree rotation (with some tolerance)
        if (fabs(accumulatedZRotation) >= 355.0f) {
            // Determine rotation direction
            int direction = (accumulatedZRotation > 0) ? 1 : -1;
            
            // Record the full rotation
            fullRotationCount += direction;
            
            // Reset accumulation but keep tracking
            accumulatedZRotation = 0.0f;
            
            // Log and save the rotation data
            unsigned long rotationDuration = currentTime - rotationStartTime;
            logger->info("Detected full 360° rotation! Direction: " + 
                        String(direction > 0 ? "clockwise" : "counterclockwise") + 
                        ", Duration: " + String(rotationDuration) + "ms");
            
            // Save rotation data for learning
            saveRotationLearningData(direction, rotationDuration);
            
            // Update the rotation start time for the next potential rotation
            rotationStartTime = currentTime;
        }
    }
    // If rotation speed falls below threshold, end the sequence
    else if (inRotationSequence && fabs(currentZValue) < rotationThreshold) {
        // Only log if we accumulated meaningful rotation
        if (fabs(accumulatedZRotation) > 45.0f) {
            logger->debug("Ending rotation sequence, accumulated " + 
                        String(accumulatedZRotation) + " degrees");
        }
        inRotationSequence = false;
        accumulatedZRotation = 0.0f;
    }
    
    // Update last value for next calculation
    lastZGyroValue = currentZValue;
}

// Save rotation data to a file for learning
bool saveRotationLearningData(int direction, unsigned long duration) {
    #if LEARNING_ENABLED
    static Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for rotation data saving");
        return false;
    }
    
    // Create directory if it doesn't exist
    String dirPath = "/data";
    if (!fileManager.exists(dirPath)) {
        fileManager.createDir(dirPath);
    }
    
    // Create JSON data structure for rotation learning
    Utils::SpiJsonDocument rotationData;
    
    // Check if file exists to append or create new
    String existingData;
    if (fileManager.exists(ROTATION_LEARNING_PATH)) {
        existingData = fileManager.readFile(ROTATION_LEARNING_PATH);
    }
    
    if (!existingData.isEmpty()) {
        // Parse existing data
        DeserializationError error = deserializeJson(rotationData, existingData);
        if (error) {
            logger->error("Failed to parse existing rotation data, creating new file");
            rotationData = Utils::SpiJsonDocument();
        }
    }
    
    // If this is first entry, create the base structure
    if (rotationData.isNull()) {
        rotationData = Utils::SpiJsonDocument();
        rotationData["rotations"] = JsonArray();
    }
    
    // Add this rotation to the array
    JsonObject rotation = rotationData["rotations"].add<JsonObject>();
    rotation["timestamp"] = millis();
    rotation["direction"] = direction > 0 ? "clockwise" : "counterclockwise";
    rotation["duration_ms"] = duration;
    rotation["x"] = robotX;
    rotation["y"] = robotY;
    
    // Convert to string
    String jsonData;
    serializeJson(rotationData, jsonData);
    
    // Save to file
    bool success = fileManager.writeFile(ROTATION_LEARNING_PATH, jsonData);
    if (success) {
        logger->debug("Rotation learning data saved to " + String(ROTATION_LEARNING_PATH));
    } else {
        logger->error("Failed to save rotation learning data");
    }
    return success;
    #else
    return false;
    #endif
}

// Get rotation learning data as JSON string
String getRotationLearningData() {
    static Utils::FileManager fileManager;
    if (!fileManager.init()) {
        logger->error("Failed to initialize FileManager for reading rotation data");
        return "{}";
    }
    
    if (!fileManager.exists(ROTATION_LEARNING_PATH)) {
        return "{\"rotations\":[]}";
    }
    
    String data = fileManager.readFile(ROTATION_LEARNING_PATH);
    if (data.isEmpty()) {
        return "{\"rotations\":[]}";
    }
    
    return data;
}

// Rotate using gyroscope data for precise turning
bool rotateWithGyro(int direction, float targetDegrees, int maxTimeMs = 5000) {
    if (!orientation || !motors) return false;
    
    // Reset accumulated rotation
    accumulatedZRotation = 0.0f;
    lastGyroReadTime = millis();
    
    // Direction: 1 for clockwise, -1 for counterclockwise
    Motors::MotorControl::Direction turnDir = (direction > 0) ? 
                                              Motors::MotorControl::RIGHT : 
                                              Motors::MotorControl::LEFT;
    
    // Start rotating
    motors->move(turnDir, maxTimeMs);
    
    // Set initial time for timeout
    unsigned long startTime = millis();
    
    // Update screen to show we're rotating
    if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Skeptic();
    
    logger->debug("Starting gyro-assisted rotation of " + String(targetDegrees) + " degrees");
    
    // Loop until we reach the target rotation or timeout
    while (fabs(accumulatedZRotation) < targetDegrees && 
           (millis() - startTime) < maxTimeMs) {
        
        // Get current time and calculate delta
        unsigned long currentTime = millis();
        float deltaTime = (currentTime - lastGyroReadTime) / 1000.0f;
        lastGyroReadTime = currentTime;
        
        // Skip if time delta is invalid
        if (deltaTime <= 0 || deltaTime > 0.1) continue;
        
        // Read gyroscope
        orientation->update();
        float zGyro = orientation->getZ();
        
        // Calculate rotation this frame and accumulate
        float rotationThisFrame = zGyro * deltaTime;
        accumulatedZRotation += rotationThisFrame;
        
        // Debugging every ~45 degrees
        if (fmod(fabs(accumulatedZRotation), 45.0f) < 1.0f) {
            logger->debug("Rotation progress: " + String(accumulatedZRotation) + " degrees");
        }
        
        // Small delay to prevent CPU hogging
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    // Stop motors
    motors->stop();
    
    // Update heading based on actual rotation
    robotHeading += direction * fabs(accumulatedZRotation);
    if (robotHeading < 0) robotHeading += 360;
    if (robotHeading >= 360) robotHeading -= 360;
    
    // Was the rotation successful?
    bool success = fabs(accumulatedZRotation) >= (targetDegrees * 0.9); // 90% accuracy
    
    logger->info("Gyro rotation completed: " + String(accumulatedZRotation) + 
                " degrees, target was " + String(targetDegrees) + 
                (success ? " (success)" : " (incomplete)"));
    
    // Reset accumulation
    accumulatedZRotation = 0.0f;
    
    return success;
}

// Perform a complete 360-degree rotation for mapping and learning
bool perform360Scan(int direction = 1) {
    if (!motors || !orientation) return false;
    
    logger->info("Starting 360-degree scan for mapping and learning");
    
    // Set face to scanning expression
    if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Focused();
    
    // Reset tracking variables
    accumulatedZRotation = 0.0f;
    lastGyroReadTime = millis();
    rotationStartTime = millis();
    
    // Direction (1 = clockwise, -1 = counterclockwise)
    direction = (direction >= 0) ? 1 : -1;
    Motors::MotorControl::Direction turnDir = (direction > 0) ? 
                                             Motors::MotorControl::RIGHT : 
                                             Motors::MotorControl::LEFT;
    
    // Start rotation
    motors->move(turnDir);
    
    // Target is slightly more than 360 degrees to ensure we complete the full circle
    float targetDegrees = 365.0f;
    unsigned long startTime = millis();
    unsigned long timeoutMs = 10000; // 10 second timeout
    
    // Map data collection points
    int mapPoints = 8; // Take 8 readings during rotation
    float degreeIncrement = 360.0f / mapPoints;
    float nextMappingPoint = degreeIncrement;
    
    // Loop until we reach full rotation or timeout
    logger->debug("Beginning rotation, target: " + String(targetDegrees) + " degrees");
    
    while (fabs(accumulatedZRotation) < targetDegrees && 
           (millis() - startTime) < timeoutMs) {
        
        // Update time and gyroscope reading
        unsigned long currentTime = millis();
        float deltaTime = (currentTime - lastGyroReadTime) / 1000.0f;
        lastGyroReadTime = currentTime;
        
        // Skip if time delta is invalid
        if (deltaTime <= 0 || deltaTime > 0.1) continue;
        
        // Read gyroscope
        orientation->update();
        float zGyro = orientation->getZ();
        
        // Calculate rotation this frame and accumulate
        float rotationThisFrame = zGyro * deltaTime;
        accumulatedZRotation += rotationThisFrame;
        
        // Check if we've reached the next mapping point
        if (fabs(accumulatedZRotation) >= nextMappingPoint) {
            // Take distance reading at this point
            float distance = distanceSensor ? distanceSensor->measureDistance() : -1;
            
            // Calculate angle in world coordinates
            float worldAngle = robotHeading + (direction * fabs(accumulatedZRotation));
            if (worldAngle < 0) worldAngle += 360;
            if (worldAngle >= 360) worldAngle -= 360;
            
            logger->debug("Mapping point at " + String(nextMappingPoint) + 
                         " degrees, distance: " + String(distance) + " cm");
            
            // If we have valid distance data, mark it on the map
            if (distance > 0 && distance < 400) {
                float radians = worldAngle * DEG_TO_RAD;
                int cellDistance = round(distance / 20); // Convert to cell units (approx)
                int dx = round(cos(radians) * cellDistance);
                int dy = round(sin(radians) * cellDistance);
                
                // Mark the cell on the map
                if (distance < 20) {
                    markMapCell(robotX + dx, robotY + dy, 1); // Obstacle
                } else {
                    markMapCell(robotX + dx, robotY + dy, 0); // Free space
                }
            }
            
            // Update next mapping point
            nextMappingPoint += degreeIncrement;
        }
        
        // Debug output every 90 degrees
        if (fmod(fabs(accumulatedZRotation), 90.0f) < 2.0f && 
            fabs(accumulatedZRotation) > 5.0f) {
            logger->debug("Rotation progress: " + String(accumulatedZRotation) + " degrees");
        }
        
        // Small delay to prevent CPU hogging
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    // Stop motors
    motors->stop();
    
    // Calculate actual duration
    unsigned long duration = millis() - rotationStartTime;
    
    // Was the rotation successful?
    bool success = fabs(accumulatedZRotation) >= 350.0f; // At least 350 degrees
    
    if (success) {
        logger->info("360-degree scan completed: " + String(accumulatedZRotation) + 
                    " degrees in " + String(duration) + "ms");
        
        // Save rotation data
        saveRotationLearningData(direction, duration);
        
        // Update the map
        saveMapToFile();
        
        // Return to happy face
        if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Happy();
    } else {
        logger->warning("360-degree scan incomplete: only reached " + 
                       String(accumulatedZRotation) + " degrees");
        
        // Return to concerned face
        if (screen && screen->getFace()) screen->getFace()->Expression.GoTo_Skeptic();
    }
    
    // Reset accumulation
    accumulatedZRotation = 0.0f;
    
    return success;
}