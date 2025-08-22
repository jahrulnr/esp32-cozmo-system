#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <MVCFramework.h>

class Controller {
public:
    virtual ~Controller() = default;
    
protected:
    // Helper methods for controllers
    Response view(AsyncWebServerRequest* request, const String& template_name, const JsonDocument& data = JsonDocument());
    Response json(AsyncWebServerRequest* request, const JsonDocument& data);
    Response redirect(AsyncWebServerRequest* request, const String& url);
    Response back(AsyncWebServerRequest* request);
    
    // Validation helpers
    bool validate(const Request& request, const std::map<String, String>& rules);
    String getValidationError(const Request& request, const String& field, const String& rule);
    
    // Response helpers
    Response success(AsyncWebServerRequest* request, const String& message = "Success");
    Response error(AsyncWebServerRequest* request, const String& message = "Error", int code = 400);
    Response notFound(AsyncWebServerRequest* request, const String& message = "Not Found");
    Response unauthorized(AsyncWebServerRequest* request, const String& message = "Unauthorized");
};

// Resource controller interface
class ResourceController : public Controller {
public:
    // Standard RESTful methods
    virtual Response index(Request& request) { return error(request.getServerRequest(), "Method not implemented"); }
    virtual Response show(Request& request) { return error(request.getServerRequest(), "Method not implemented"); }
    virtual Response store(Request& request) { return error(request.getServerRequest(), "Method not implemented"); }
    virtual Response update(Request& request) { return error(request.getServerRequest(), "Method not implemented"); }
    virtual Response destroy(Request& request) { return error(request.getServerRequest(), "Method not implemented"); }
    virtual Response create(Request& request) { return error(request.getServerRequest(), "Method not implemented"); }
    virtual Response edit(Request& request) { return error(request.getServerRequest(), "Method not implemented"); }
};

#endif
