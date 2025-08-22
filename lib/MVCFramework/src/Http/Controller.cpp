#include "Controller.h"

Response Controller::view(AsyncWebServerRequest* request, const String& template_name, const JsonDocument& data) {
    // TODO: Implement proper view rendering
    String html = "<html><head><title>" + template_name + "</title></head>";
    html += "<body><h1>View: " + template_name + "</h1>";
    
    // Simple data rendering
    if (!data.isNull()) {
        html += "<pre>" + data.as<String>() + "</pre>";
    }
    
    html += "</body></html>";
    
    return Response(request).html(html);
}

Response Controller::json(AsyncWebServerRequest* request, const JsonDocument& data) {
    return Response(request).json(data);
}

Response Controller::redirect(AsyncWebServerRequest* request, const String& url) {
    return Response(request).redirect(url);
}

Response Controller::back(AsyncWebServerRequest* request) {
    return Response(request).back();
}

bool Controller::validate(const Request& request, const std::map<String, String>& rules) {
    for (const auto& rule : rules) {
        String field = rule.first;
        String validation = rule.second;
        
        if (validation == "required" && !request.filled(field)) {
            return false;
        }
        
        if (validation.startsWith("min:")) {
            int minLength = validation.substring(4).toInt();
            if (request.get(field).length() < minLength) {
                return false;
            }
        }
        
        if (validation.startsWith("max:")) {
            int maxLength = validation.substring(4).toInt();
            if (request.get(field).length() > maxLength) {
                return false;
            }
        }
        
        if (validation == "username") {
            String username = request.get(field);
            if (username.indexOf('@') == -1 || username.indexOf('.') == -1) {
                return false;
            }
        }
    }
    
    return true;
}

String Controller::getValidationError(const Request& request, const String& field, const String& rule) {
    if (rule == "required") {
        return field + " is required";
    }
    
    if (rule.startsWith("min:")) {
        return field + " must be at least " + rule.substring(4) + " characters";
    }
    
    if (rule.startsWith("max:")) {
        return field + " must not exceed " + rule.substring(4) + " characters";
    }
    
    if (rule == "username") {
        return field + " must be a valid username address";
    }
    
    return field + " is invalid";
}

Response Controller::success(AsyncWebServerRequest* request, const String& message) {
    JsonDocument data;
    data["success"] = true;
    data["message"] = message;
    
    return Response(request).json(data);
}

Response Controller::error(AsyncWebServerRequest* request, const String& message, int code) {
    JsonDocument data;
    data["success"] = false;
    data["error"] = message;
    
    return Response(request).status(code).json(data);
}

Response Controller::notFound(AsyncWebServerRequest* request, const String& message) {
    return error(request, message, 404);
}

Response Controller::unauthorized(AsyncWebServerRequest* request, const String& message) {
    return error(request, message, 401);
}
