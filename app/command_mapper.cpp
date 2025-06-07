#include <Arduino.h>
#include "app.h"

void setupCommandMapper() {
    if (screen && motors && servos) {
        logger->info("Setting up CommandMapper...");
        commandMapper = new Utils::CommandMapper(logger, screen, motors, servos);
        logger->info("CommandMapper initialized");
    } else {
        logger->error("Failed to initialize CommandMapper: missing required subsystems");
    }
}

// Execute commands from text and return the regular text content
String processTextCommands(const String& text) {
    if (!commandMapper) {
        logger->warning("CommandMapper not initialized, cannot process commands");
        return text;
    }
    
    // Extract and execute commands
    int commandCount = commandMapper->executeCommandString(text);
    
    if (commandCount > 0) {
        logger->debug("Executed " + String(commandCount) + " commands from text");
        
        // Return just the text content (without commands)
        return commandMapper->extractText(text);
    }
    
    // If no commands found, return the original text
    return text;
}
