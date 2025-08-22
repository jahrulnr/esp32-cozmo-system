#ifndef VIEW_H
#define VIEW_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

class View {
private:
    String templatePath;
    JsonDocument data;
    String compiledContent;

public:
    View(const String& path);
    
    // Data binding
    View& with(const String& key, const String& value);
    View& with(const String& key, int value);
    View& with(const String& key, bool value);
    View& with(const JsonDocument& data);
    View& withErrors(const std::vector<String>& errors);
    
    // Template compilation
    String render();
    String compile();
    
    // Template helpers
    static String asset(const String& path);
    static String route(const String& name, const std::map<String, String>& parameters = {});
    static String url(const String& path);
    
private:
    String loadTemplate(const String& path);
    String parseTemplate(const String& content);
    String replaceVariables(const String& content);
    String processDirectives(const String& content);
    String processIncludes(const String& content);
    String processLoops(const String& content);
    String processConditions(const String& content);
};

// Template engine for simple templating
class TemplateEngine {
private:
    static TemplateEngine* instance;
    std::map<String, String> globals;
    String viewsPath;

public:
    static TemplateEngine* getInstance();
    
    void setViewsPath(const String& path) { viewsPath = path; }
    String getViewsPath() const { return viewsPath; }
    
    void addGlobal(const String& key, const String& value);
    String getGlobal(const String& key, const String& defaultValue = "") const;
    
    String render(const String& template_name, const JsonDocument& data = JsonDocument());
    
private:
    TemplateEngine() : viewsPath("/views") {}
};

// Helper functions for view creation
View view(const String& template_name, const JsonDocument& data = JsonDocument());
String renderView(const String& template_name, const JsonDocument& data = JsonDocument());

#endif
