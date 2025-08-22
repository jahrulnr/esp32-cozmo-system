#include "Middleware.h"
#include "Request.h"
#include "Response.h"

// AuthMiddleware implementation
Response AuthMiddleware::handle(Request& request, std::function<Response(Request&)> next) {
    // Check for authentication
    String token = request.header("Authorization");
    
    if (token.length() == 0 || !token.startsWith("Bearer ")) {
        // For web requests, redirect to login
        if (!request.wantsJson()) {
            return Response(request.getServerRequest())
                .redirect("/login?redirect=" + request.path());
        }
        
        // For API requests, return JSON error
        JsonDocument error;
        error["error"] = "Unauthorized";
        error["message"] = "Authentication required";
        
        return Response(request.getServerRequest())
            .status(401)
            .json(error);
    }
    
    // Extract and validate token
    String authToken = token.substring(7); // Remove "Bearer " prefix
    
    // Simple token validation - in production use proper JWT verification
    if (!authToken.startsWith("cozmo_token_") || authToken.length() < 20) {
        if (!request.wantsJson()) {
            return Response(request.getServerRequest())
                .redirect("/login?redirect=" + request.path());
        }
        
        JsonDocument error;
        error["error"] = "Unauthorized";
        error["message"] = "Invalid token";
        
        return Response(request.getServerRequest())
            .status(401)
            .json(error);
    }
    
    // Token is valid, continue to the next middleware/handler
    return next(request);
}

// CorsMiddleware implementation
CorsMiddleware::CorsMiddleware(const String& origins, const String& methods, const String& headers)
    : allowedOrigins(origins), allowedMethods(methods), allowedHeaders(headers) {
}

Response CorsMiddleware::handle(Request& request, std::function<Response(Request&)> next) {
    // Handle preflight requests
    if (request.method() == "OPTIONS") {
        return Response(request.getServerRequest())
            .header("Access-Control-Allow-Origin", allowedOrigins)
            .header("Access-Control-Allow-Methods", allowedMethods)
            .header("Access-Control-Allow-Headers", allowedHeaders)
            .header("Access-Control-Max-Age", "86400")
            .status(200)
            .content("");
    }
    
    // Continue to next middleware/handler
    Response response = next(request);
    
    // Add CORS headers to response
    response.header("Access-Control-Allow-Origin", allowedOrigins)
           .header("Access-Control-Allow-Methods", allowedMethods)
           .header("Access-Control-Allow-Headers", allowedHeaders);
    
    return response;
}

// RateLimitMiddleware implementation
RateLimitMiddleware::RateLimitMiddleware(int max, unsigned long window)
    : maxRequests(max), windowMs(window) {
}

Response RateLimitMiddleware::handle(Request& request, std::function<Response(Request&)> next) {
    String clientIp = request.ip();
    unsigned long now = millis();
    
    // Cleanup old entries
    cleanup();
    
    // Check current request count for this IP
    auto it = requests.find(clientIp);
    if (it != requests.end()) {
        if (now - it->second.second < windowMs) {
            if (it->second.first >= maxRequests) {
                JsonDocument error;
                error["error"] = "Too Many Requests";
                error["message"] = "Rate limit exceeded";
                error["retry_after"] = (windowMs - (now - it->second.second)) / 1000;
                
                return Response(request.getServerRequest())
                    .status(429)
                    .header("Retry-After", String((windowMs - (now - it->second.second)) / 1000))
                    .json(error);
            }
            it->second.first++;
        } else {
            // Reset counter for new window
            it->second.first = 1;
            it->second.second = now;
        }
    } else {
        // First request from this IP
        requests[clientIp] = {1, now};
    }
    
    return next(request);
}

void RateLimitMiddleware::cleanup() {
    unsigned long now = millis();
    auto it = requests.begin();
    
    while (it != requests.end()) {
        if (now - it->second.second > windowMs) {
            it = requests.erase(it);
        } else {
            ++it;
        }
    }
}

// LoggingMiddleware implementation
Response LoggingMiddleware::handle(Request& request, std::function<Response(Request&)> next) {
    unsigned long startTime = millis();
    
    // Log request
    Serial.printf("[%lu] %s %s from %s\n", 
                  startTime, 
                  request.method().c_str(), 
                  request.path().c_str(), 
                  request.ip().c_str());
    
    // Continue to next middleware/handler
    Response response = next(request);
    
    // Log response
    unsigned long duration = millis() - startTime;
    Serial.printf("[%lu] Response: %d in %lums\n", 
                  millis(), 
                  response.getStatusCode(), 
                  duration);
    
    return response;
}

// JsonMiddleware implementation
Response JsonMiddleware::handle(Request& request, std::function<Response(Request&)> next) {
    // Set default content type for API responses
    Response response = next(request);
    
    // If no content type is set and we have content, assume JSON
    if (response.getContentType() == "text/html" && 
        response.getContent().startsWith("{")) {
        response.contentType("application/json");
    }
    
    return response;
}
