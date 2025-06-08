#include <Arduino.h>
#include "app.h"
#include "lib/Automation/Automation.h"

// Global variables for automation control (kept for compatibility)
TaskHandle_t automationTaskHandle = NULL;
bool g_automationEnabled = AUTOMATION_ENABLED;
unsigned long g_lastManualControlTime = 0;
int g_automationBehaviorIndex = 0;

// Initialize and start automation
void setupAutomation() {
    if (automation == nullptr) {
        automation = new Automation::Automation(fileManager, commandMapper, logger, webSocket);
    }
}

// Update the time of last manual control
void updateManualControlTime() {
    if (automation) {
        automation->updateManualControlTime();
    }
    g_lastManualControlTime = millis(); // Update global var for compatibility
}

// Get automation enabled status
bool isAutomationEnabled() {
    if (automation) {
        return automation->isEnabled();
    }
    return g_automationEnabled;
}

// Set automation enabled status
void setAutomationEnabled(bool enabled) {
    if (automation) {
        automation->setEnabled(enabled);
    }
    g_automationEnabled = enabled; // Update global var for compatibility
}