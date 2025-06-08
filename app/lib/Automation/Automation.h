#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <Arduino.h>
#include <vector>
#include "Config.h"
#include "lib/Utils/Sstring.h"
#include "lib/Utils/FileManager.h"
#include "lib/Utils/CommandMapper.h"
#include "lib/Utils/Logger.h"
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
    bool addNewBehavior(const String& behavior);
    bool fetchAndAddNewBehaviors(const String& prompt = "Generate new robot behaviors");
    
    static void taskFunction(void* parameter);

private:
    Utils::FileManager* m_fileManager;
    Utils::CommandMapper* m_commandMapper;
    Utils::Logger* m_logger;
    Communication::WebSocketHandler* m_webSocket;
    
    TaskHandle_t m_taskHandle;
    bool m_enabled;
    unsigned long m_lastManualControlTime;
    int m_behaviorIndex;
    std::vector<Utils::Sstring> m_templateBehaviors;
    SemaphoreHandle_t m_behaviorsMutex;
    
    void loadTemplateBehaviors();
    bool saveBehaviorsToFile();
    void executeBehavior(const Utils::Sstring& behavior);
};

} // namespace Automation

#endif // AUTOMATION_H
