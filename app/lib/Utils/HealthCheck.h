#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>

namespace Utils {

class HealthCheck {
public:
    enum Status {
        HEALTHY,
        WARNING,
        ERROR,
        CRITICAL
    };

    struct Check {
        String name;
        std::function<Status()> checkFunction;
        Status lastStatus;
        String message;
    };

    HealthCheck();
    ~HealthCheck();

    /**
     * Initialize health check
     * @param checkIntervalMs Interval between checks in milliseconds
     * @return true if initialization was successful, false otherwise
     */
    bool init(unsigned long checkIntervalMs = 10000);

    /**
     * Add a check
     * @param name The name of the check
     * @param checkFunction The function to call for the check
     * @return true if the check was added successfully, false otherwise
     */
    bool addCheck(const String& name, std::function<Status()> checkFunction);

    /**
     * Run all checks
     */
    void runChecks();

    /**
     * Get the overall status
     * @return The most severe status from all checks
     */
    Status getOverallStatus() const;

    /**
     * Get check results
     * @return Vector of Check structures with results
     */
    const std::vector<Check>& getChecks() const;

    /**
     * Set callback for status change
     * @param callback The callback function
     */
    void setStatusChangeCallback(std::function<void(const String&, Status, Status)> callback);

    /**
     * Update in the main loop
     * Runs checks at the specified interval
     */
    void update();

private:
    std::vector<Check> _checks;
    Status _overallStatus;
    unsigned long _checkInterval;
    unsigned long _lastCheckTime;
    bool _initialized;
    std::function<void(const String&, Status, Status)> _statusChangeCallback;
    
    // Convert status to string
    String statusToString(Status status) const;
};

} // namespace Utils
