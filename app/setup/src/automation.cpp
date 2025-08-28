#include <Arduino.h>
#include "setup/setup.h"
#include "lib/Automation/Automation.h"

// Global variables for automation control (kept for compatibility)
bool _enableAutomation = AUTOMATION_ENABLED;
Automation::Automation *automation;

// Initialize and start automation
void setupAutomation() {
    if (automation == nullptr) {
        automation = new Automation::Automation(fileManager, commandMapper, logger);
    }
}