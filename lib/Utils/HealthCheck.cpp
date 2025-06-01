#include "HealthCheck.h"

namespace Utils {

HealthCheck::HealthCheck() : _overallStatus(HEALTHY), _checkInterval(10000), _lastCheckTime(0), _initialized(false) {
}

HealthCheck::~HealthCheck() {
    // Clean up resources if needed
}

bool HealthCheck::init(unsigned long checkIntervalMs) {
    _checkInterval = checkIntervalMs;
    _lastCheckTime = millis();
    _initialized = true;
    return true;
}

bool HealthCheck::addCheck(const String& name, std::function<Status()> checkFunction) {
    if (!_initialized) {
        return false;
    }
    
    Check check;
    check.name = name;
    check.checkFunction = checkFunction;
    check.lastStatus = HEALTHY;
    check.message = "Initial state";
    
    _checks.push_back(check);
    return true;
}

void HealthCheck::runChecks() {
    if (!_initialized) {
        return;
    }
    
    Status worstStatus = HEALTHY;
    
    for (auto& check : _checks) {
        Status previousStatus = check.lastStatus;
        check.lastStatus = check.checkFunction();
        
        // Update worst status
        if (check.lastStatus > worstStatus) {
            worstStatus = check.lastStatus;
        }
        
        // Notify on status change
        if (check.lastStatus != previousStatus && _statusChangeCallback) {
            _statusChangeCallback(check.name, previousStatus, check.lastStatus);
        }
    }
    
    _overallStatus = worstStatus;
    _lastCheckTime = millis();
}

HealthCheck::Status HealthCheck::getOverallStatus() const {
    return _overallStatus;
}

const std::vector<HealthCheck::Check>& HealthCheck::getChecks() const {
    return _checks;
}

void HealthCheck::setStatusChangeCallback(std::function<void(const String&, Status, Status)> callback) {
    _statusChangeCallback = callback;
}

void HealthCheck::update() {
    if (!_initialized) {
        return;
    }
    
    unsigned long currentTime = millis();
    if (currentTime - _lastCheckTime >= _checkInterval) {
        runChecks();
    }
}

String HealthCheck::statusToString(Status status) const {
    switch (status) {
        case HEALTHY:
            return "HEALTHY";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        case CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

} // namespace Utils
