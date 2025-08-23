#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <MVCFramework.h>
#include "../../repository/User.h"

class AuthController : public Controller {
public:
    // GET /login - Show login form
    Response showLogin(Request& request);
    
    // POST /login - Process login
    Response login(Request& request);
    
    // POST /logout - Process logout
    Response logout(Request& request);
    
    // GET /dashboard - Show dashboard (requires auth)
    Response dashboard(Request& request);
    
    // Static helper methods for other controllers
    static String getCurrentUserUsername(Request& request);
    static class User* getCurrentUser(Request& request);
    
    // API method for getting current user info
    Response getUserInfo(Request& request);
    
private:
    bool validateCredentials(const String& username, const String& password);
    String generateToken(const String& username);
    bool verifyToken(const String& token);
    String extractUsernameFromToken(const String& token);
};

#endif
