#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <MVCFramework.h>
#include <core/Utils/SpiAllocator.h>
#include "repository/User.h"
#include <Sstring.h>

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
    static Utils::Sstring getCurrentUserUsername(Request& request);
    static class IModel::User* getCurrentUser(Request& request);

    // API method for getting current user info
    Response getUserInfo(Request& request);

private:
    bool validateCredentials(const Utils::Sstring& username, const Utils::Sstring& password);
    Utils::Sstring generateToken(const Utils::Sstring& username);
    bool verifyToken(const Utils::Sstring& token);
    Utils::Sstring extractUsernameFromToken(const Utils::Sstring& token);
};

#endif
