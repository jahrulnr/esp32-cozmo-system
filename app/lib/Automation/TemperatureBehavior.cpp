#include <Arduino.h>
#include "app.h"
#include "TemperatureBehavior.h"

// Temperature behavior thresholds
const float HOT_THRESHOLD = 30.0;
const float WARM_THRESHOLD = 25.0;
const float COOL_THRESHOLD = 15.0;
const float COLD_THRESHOLD = 10.0;

// Initialize the temperature-based behavior templates
void setupTemperatureTemplates() {
    if (!templateManager || !templateManager->init()) {
        logger->warning("Failed to initialize template manager for temperature behaviors");
        return;
    }
    
    logger->info("Temperature templates initialized");
}

// Execute a temperature-related behavior based on the current temperature
void executeTemperatureBehavior() {
    if (!templateManager || !temperatureSensor) {
        return;
    }
    
    float temperature = temperatureSensor->readTemperature();
    
    // Skip invalid temperature readings
    if (isnan(temperature)) {
        return;
    }
    
    // Determine appropriate behavior based on temperature
    String category = "";
    String text = "";
    String eventType = "";
    
    if (temperature >= HOT_THRESHOLD) {
        category = "worried";
        eventType = "temperature_high";
        text = "It's getting hot in here! " + String(temperature, 1) + "°C";
        logger->info("Temperature high: " + String(temperature) + "°C");
    } 
    else if (temperature >= WARM_THRESHOLD) {
        category = "unimpressed";
        eventType = "temperature_warm";
        text = "It's pretty warm... " + String(temperature, 1) + "°C";
        logger->info("Temperature warm: " + String(temperature) + "°C");
    }
    else if (temperature <= COLD_THRESHOLD) {
        category = "worried"; 
        eventType = "temperature_low";
        text = "Brr, it's cold! " + String(temperature, 1) + "°C";
        logger->info("Temperature low: " + String(temperature) + "°C");
    }
    else if (temperature <= COOL_THRESHOLD) {
        category = "surprised";
        eventType = "temperature_cool";
        text = "It's getting chilly. " + String(temperature, 1) + "°C";
        logger->info("Temperature cool: " + String(temperature) + "°C");
    }
    else {
        // Comfortable temperature range
        return;
    }
    
    // Execute appropriate template based on event type
    if (eventType.length() > 0) {
        bool success = templateManager->executeEventTemplate(eventType);
        if (!success) {
            // Fall back to category-based template if event template fails
            templateManager->executeRandomTemplate(category);
        }
    }
    
    // Show temperature in display if applicable
    if (screen && text.length() > 0) {
        // Display temperature message
        screen->drawCenteredText(20, text);
        
        // Give time for the user to see the message
        delay(2000);
    }
}
