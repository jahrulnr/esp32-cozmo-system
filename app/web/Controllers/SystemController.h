#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

#include "Http/Controller.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "../../repository/Configuration.h"

class SystemController : public Controller {
public:
    // Get system statistics
    static Response getStats(Request& request);
    
    // Restart system
    static Response restart(Request& request);
    
    // Get network information
    static Response getNetworkInfo(Request& request);
    
    // Get memory information
    static Response getMemoryInfo(Request& request);
    
    // Get hostname information
    static Response getHostname(Request& request);
    
    // Update hostname
    static Response updateHostname(Request& request);
    
    // Get all configurations
    static Response getConfigurations(Request& request);
    
    // Update a configuration value
    static Response updateConfiguration(Request& request);

private:
    // Helper methods
    static String formatUptime(unsigned long milliseconds);
    static String formatBytes(size_t bytes);
    static JsonDocument getSystemInfo();
};

#endif
