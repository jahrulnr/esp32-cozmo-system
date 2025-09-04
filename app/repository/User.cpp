#include "User.h"
#include <Crypto.h>

bool User::authenticate(const String& password) const {
    // Simple password check (in production, use proper hashing)
    return getPassword() == password;
}

void User::hashPassword(const String& password) {
    // Simple hash (in production, use proper password hashing like bcrypt)
    // For now, just store as-is for demo purposes
    setPassword(password);
}

User* User::findByUsername(const String& username) {
    if (!database) {
        return nullptr;
    }
    
    std::map<String, String> where;
    where["username"] = username;
    
    auto record = database->findWhere("users", where);
    if (record.empty()) {
        return nullptr;
    }
    
    User* user = new User();
    user->fill(record);
    user->syncOriginal();
    user->exists = true;
    
    return user;
}

std::vector<User*> User::all() {
    if (!database) {
        return std::vector<User*>();
    }
    
    auto records = database->select("users");
    std::vector<User*> users;
    
    for (const auto& record : records) {
        User* user = new User();
        user->fill(record);
        user->syncOriginal();
        user->exists = true;
        users.push_back(user);
    }
    
    return users;
}

bool User::validate() const {
    return getValidationErrors().empty();
}

std::vector<String> User::getValidationErrors() const {
    std::vector<String> errors;
    
    if (getUsername().length() == 0) {
        errors.push_back("Username is required");
    } else if (getUsername().length() < 3) {
        errors.push_back("Username must be at least 3 characters");
    }
    
    if (getPassword().length() < 6) {
        errors.push_back("Password must be at least 6 characters");
    }
    
    return errors;
}

void User::touch() {
}
