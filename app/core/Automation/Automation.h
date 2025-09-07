#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <Arduino.h>
#include <vector>
#include "Config.h"
#include "Logger.h"
#include "Sstring.h"
#include "FileManager.h"
#include "core/Utils/CommandMapper.h"
#include "core/Communication/GPTAdapter.h"

namespace Automation {

class Automation {
public:
    Automation(Utils::FileManager* fileManager, 
               Utils::CommandMapper* commandMapper,
               Utils::Logger* logger);
    ~Automation();

    // core 0 or 1
    void start(bool core = false);
    void stop();
    void updateManualControlTime();
    bool isEnabled() const;
    void setEnabled(bool enabled);
    bool isRandomBehaviorOrder() const;
    void setRandomBehaviorOrder(bool randomOrder = true);
    bool fetchAndAddNewBehaviors(const Utils::Sstring& prompt = "Generate new robot behaviors");
    
    static void taskFunction(void* parameter);

private:
    Utils::FileManager* _fileManager;
    Utils::CommandMapper* _commandMapper;
    Utils::Logger* _logger;
    
    TaskHandle_t _taskHandle;
    bool _enabled;
    bool _randomBehaviorOrder;
    unsigned long _lastManualControlTime;
    int _behaviorIndex;
    std::vector<Utils::Sstring> _templateBehaviors;
    SemaphoreHandle_t _behaviorsMutex;

    long _timer;
    
    void loadTemplateBehaviors();
    void executeBehavior(const Utils::Sstring& behavior);

    const Utils::Sstring _behaviorPrompt;
    const char* _templatesFile;
    const char* _templatesUpdateFile;
};

} // namespace Automation

#endif // AUTOMATION_H
