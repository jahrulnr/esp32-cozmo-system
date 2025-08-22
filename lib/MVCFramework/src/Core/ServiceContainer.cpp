#include "ServiceContainer.h"

ServiceContainer::~ServiceContainer() {
    flush();
}

bool ServiceContainer::has(const String& name) {
    return bindings.find(name) != bindings.end();
}

void ServiceContainer::forget(const String& name) {
    // Remove from bindings
    auto bindingIt = bindings.find(name);
    if (bindingIt != bindings.end()) {
        bindings.erase(bindingIt);
    }
    
    // Remove from singletons and cleanup
    auto singletonIt = singletons.find(name);
    if (singletonIt != singletons.end()) {
        // Note: We don't delete the pointer here as it might be managed elsewhere
        singletons.erase(singletonIt);
    }
}

void ServiceContainer::flush() {
    bindings.clear();
    
    // Clear singletons (careful with memory management)
    for (auto& pair : singletons) {
        // Note: We don't delete pointers here as they might be managed elsewhere
        // In a real implementation, you'd need proper memory management
    }
    singletons.clear();
}
