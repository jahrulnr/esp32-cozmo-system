#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include <Arduino.h>
#include <functional>
#include <map>

// Forward declarations
class Request;
class Response;

class Middleware {
public:
    virtual ~Middleware() = default;
    virtual Response handle(Request& request, std::function<Response(Request&)> next) = 0;
};

// Auth middleware
class AuthMiddleware : public Middleware {
public:
    Response handle(Request& request, std::function<Response(Request&)> next) override;
};

// CORS middleware
class CorsMiddleware : public Middleware {
private:
    String allowedOrigins;
    String allowedMethods;
    String allowedHeaders;

public:
    CorsMiddleware(const String& origins = "*", 
                   const String& methods = "GET,POST,PUT,DELETE,PATCH,OPTIONS",
                   const String& headers = "Content-Type,Authorization");
    
    Response handle(Request& request, std::function<Response(Request&)> next) override;
};

// Rate limiting middleware
class RateLimitMiddleware : public Middleware {
private:
    int maxRequests;
    unsigned long windowMs;
    std::map<String, std::pair<int, unsigned long>> requests;

public:
    RateLimitMiddleware(int max = 100, unsigned long window = 60000); // 100 requests per minute
    Response handle(Request& request, std::function<Response(Request&)> next) override;
    
private:
    void cleanup();
};

// Logging middleware
class LoggingMiddleware : public Middleware {
public:
    Response handle(Request& request, std::function<Response(Request&)> next) override;
};

// JSON middleware
class JsonMiddleware : public Middleware {
public:
    Response handle(Request& request, std::function<Response(Request&)> next) override;
};

#endif
