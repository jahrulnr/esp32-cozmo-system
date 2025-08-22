#ifndef SERVICE_CONTAINER_H
#define SERVICE_CONTAINER_H

#include <Arduino.h>
#include <map>
#include <functional>
#include <memory>

class ServiceContainer {
private:
    std::map<String, std::function<void*()>> bindings;
    std::map<String, void*> singletons;

public:
    ServiceContainer() = default;
    ~ServiceContainer();
    
    template<typename T>
    void bind(const String& name, std::function<T*()> factory) {
        bindings[name] = [factory]() -> void* {
            return static_cast<void*>(factory());
        };
    }
    
    template<typename T>
    void singleton(const String& name, std::function<T*()> factory) {
        bind<T>(name, factory);
        // Mark as singleton for lazy loading
    }
    
    template<typename T>
    T* resolve(const String& name) {
        // Check if it's a singleton first
        if (singletons.find(name) != singletons.end()) {
            return static_cast<T*>(singletons[name]);
        }
        
        // Check if binding exists
        if (bindings.find(name) == bindings.end()) {
            return nullptr;
        }
        
        void* instance = bindings[name]();
        return static_cast<T*>(instance);
    }
    
    template<typename T>
    T* resolveSingleton(const String& name) {
        if (singletons.find(name) != singletons.end()) {
            return static_cast<T*>(singletons[name]);
        }
        
        T* instance = resolve<T>(name);
        if (instance) {
            singletons[name] = static_cast<void*>(instance);
        }
        
        return instance;
    }
    
    bool has(const String& name);
    void forget(const String& name);
    void flush();
};

#endif
