#ifndef APPLICATION_TEMPLATES_H
#define APPLICATION_TEMPLATES_H

#include "Application.h"
#include "ServiceContainer.h"

// Template method implementations for Application class
// Include this file when you need to use the template methods

template<typename T>
void Application::bind(const String& name, std::function<T*()> factory) {
    container->bind<T>(name, factory);
}

template<typename T>
T* Application::resolve(const String& name) {
    return container->resolve<T>(name);
}

#endif
