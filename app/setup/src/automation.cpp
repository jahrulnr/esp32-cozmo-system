#include <Arduino.h>
#include "setup/setup.h"
#include "lib/Automation/Automation.h"

// Global variables for automation control (kept for compatibility)
bool _enableAutomation = AUTOMATION_ENABLED;
unsigned long _lastManualControlTime = 0;
int g_automationBehaviorIndex = 0;

Automation::Automation *automation;

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
    _lastManualControlTime = millis(); // Update global var for compatibility
}

// Get automation enabled status
bool isAutomationEnabled() {
    if (automation) {
        return automation->isEnabled();
    }
    return _enableAutomation;
}

// Set automation enabled status
void setAutomationEnabled(bool enabled) {
    if (automation) {
        automation->setEnabled(enabled);
    }
    _enableAutomation = enabled; // Update global var for compatibility
}