#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <MVCFramework.h>

// DECLARE_MODEL(User, "users")

class User : public Model {
public:
    User() : Model("users") {
        initTable();
    }
    
    // Attributes
    String getUsername() const { return get("username"); }
    void setUsername(const String& username) { set("username", username); }
    
    String getPassword() const { return get("password"); }
    void setPassword(const String& password) { set("password", password); }
    
    // Methods
    bool authenticate(const String& password) const {
        return getPassword() == password;
    };
    void hashPassword(const String& password) {
        setPassword(password);
    };
    
    // Static methods
    static User* findByUsername(const String& username){
        if (!database) {
            (new User())->initTable();
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
    };
    static std::vector<User*> all() {
        if (!database) {
            (new User())->initTable();
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
    };
    
    // Validation
    bool validate() const {
        return getValidationErrors().empty();
    };
    std::vector<String> getValidationErrors() const {
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
    };

private:
    // Initialize administrative regions table
    bool initTable() {
        // Get the database from Model
        CsvDatabase* db = Model::getDatabase();
        if (!db) return false;
        
        // Create table if it doesn't exist
        if (!db->tableExists("users")) {
            std::vector<String> columns = {"username", "password"};
            return db->createTable("users", columns);
        }
        
        return true;
    }
    
protected:
    void touch() override {};
};

#endif
