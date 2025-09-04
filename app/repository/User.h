#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <MVCFramework.h>

// DECLARE_MODEL(User, "users")

class User : public Model {
public:
    User() : Model("users") {}
    
    // Attributes
    String getUsername() const { return get("username"); }
    void setUsername(const String& username) { set("username", username); }
    
    String getPassword() const { return get("password"); }
    void setPassword(const String& password) { set("password", password); }
    
    // Methods
    bool authenticate(const String& password) const;
    void hashPassword(const String& password);
    
    // Static methods
    static User* findByUsername(const String& username);
    static std::vector<User*> all();
    
    // Validation
    bool validate() const;
    std::vector<String> getValidationErrors() const;
    
protected:
    void touch() override;
};

#endif
