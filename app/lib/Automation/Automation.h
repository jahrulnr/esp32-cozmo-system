#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <Arduino.h>
#include <vector>
#include "Config.h"
#include "Sstring.h"
#include "FileManager.h"
#include "lib/Utils/CommandMapper.h"
#include "Logger.h"
#include "lib/Communication/WebSocketHandler.h"
#include "lib/Communication/GPTAdapter.h"

namespace Automation {

class Automation {
public:
    Automation(Utils::FileManager* fileManager, 
               Utils::CommandMapper* commandMapper,
               Utils::Logger* logger,
               Communication::WebSocketHandler* webSocket);
    ~Automation();

    void start();
    void stop();
    void updateManualControlTime();
    bool isEnabled() const;
    void setEnabled(bool enabled);
    bool isRandomBehaviorOrder() const;
    void setRandomBehaviorOrder(bool randomOrder = true);
    bool addNewBehavior(const String& behavior);
    bool fetchAndAddNewBehaviors(const String& prompt = "Generate new robot behaviors");
    
    static void taskFunction(void* parameter);

private:
    Utils::FileManager* _fileManager;
    Utils::CommandMapper* _commandMapper;
    Utils::Logger* _logger;
    Communication::WebSocketHandler* _webSocket;
    
    TaskHandle_t _taskHandle;
    bool _enabled;
    bool _randomBehaviorOrder;
    unsigned long _lastManualControlTime;
    int _behaviorIndex;
    std::vector<Utils::Sstring> _templateBehaviors;
    SemaphoreHandle_t _behaviorsMutex;

    long _timer;
    
    void loadTemplateBehaviors();
    bool saveBehaviorsToFile();
    void executeBehavior(const Utils::Sstring& behavior);
};

} // namespace Automation

#endif // AUTOMATION_H
