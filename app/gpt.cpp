#include <Arduino.h>
#include "init.h"

TaskHandle_t gptTaskHandle = nullptr;

// Define a data logging path for GPT interactions
#define GPT_DATA_LOG_PATH "/data/gpt_interactions.txt"
#define GPT_LEARNING_ENABLED true

struct gptRequest
{
	String prompt;
	Communication::GPTAdapter::ResponseCallback callback;
	bool saveToLog;  // Flag to indicate if this interaction should be logged
};


void setupGPT(){
	gptAdapter = new Communication::GPTAdapter();
	#if GPT_ENABLED
	gptAdapter->init(GPT_API_KEY);
	gptAdapter->setModel(GPT_MODEL);
	gptAdapter->setMaxTokens(GPT_MAX_TOKENS);
	gptAdapter->setTemperature(GPT_TEMPERATURE);
  #endif
  
  // Create directory for data storage if enabled
  #if GPT_LEARNING_ENABLED
  static Utils::FileManager fileManager;
  if (fileManager.init()) {
    // Extract directory part from path
    String dirPath = "/data";
    if (!fileManager.exists(dirPath)) {
      fileManager.createDir(dirPath);
      logger->info("Created directory for GPT learning data");
    }
  }
  #endif
}

// Log a GPT interaction (prompt + response) to a file for learning purposes
bool logGPTInteraction(const String& prompt, const String& response) {
  #if GPT_LEARNING_ENABLED
  static Utils::FileManager fileManager;
  if (!fileManager.init()) {
    logger->error("Failed to initialize FileManager for GPT logging");
    return false;
  }
  
  String logEntry = "{\"timestamp\":" + String(millis()) + 
                   ",\"prompt\":\"" + prompt + 
                   "\",\"response\":\"" + response + "\"}\n";
  
  bool success = fileManager.appendFile(GPT_DATA_LOG_PATH, logEntry);
  if (success) {
    logger->debug("Logged GPT interaction to " + String(GPT_DATA_LOG_PATH));
  } else {
    logger->error("Failed to log GPT interaction");
  }
  return success;
  #else
  return false;
  #endif
}

void gptChatTask(void * param) {
	gptRequest *data = (gptRequest*) param;
	
	if (data != nullptr){
		gptAdapter->sendPrompt(data->prompt, [data](const String& gptResponse){
			// Process commands in the response if CommandMapper is available
			String processedResponse = gptResponse;
			
			if (commandMapper != nullptr) {
				// Extract and execute any commands in the response
				logger->debug("Processing commands in GPT response");
				int commandCount = commandMapper->executeCommandString(gptResponse);
				
				if (commandCount > 0) {
					logger->debug("Executed " + String(commandCount) + " commands from GPT response");
					
					// Get just the text without commands
					processedResponse = commandMapper->extractText(gptResponse);
				}
			}

			// Call the callback with the processed response
			data->callback(processedResponse);

			if (screen){
				screen->mutexClear();
				screen->drawCenteredText(20, processedResponse);
				screen->mutexUpdate();
			}
			
			// Log the interaction if requested
			if (data->saveToLog) {
				logGPTInteraction(data->prompt, gptResponse);
			}
		});
	}

	gptTaskHandle = nullptr;
	vTaskDelete(NULL);
}

void sendGPT(const String &prompt, Communication::GPTAdapter::ResponseCallback callback){
	gptRequest *data = new gptRequest{
		prompt: prompt,
		callback: callback,
		saveToLog: GPT_LEARNING_ENABLED
	};

	xTaskCreate(gptChatTask, "gptChatTask", 20 * 1024, data, 10, &gptTaskHandle);
}

// Function to retrieve learning data (can be called by automation tasks)
String getGPTLearningData() {
  #if GPT_LEARNING_ENABLED
  static Utils::FileManager fileManager;
  if (!fileManager.init()) {
    logger->error("Failed to initialize FileManager for GPT data retrieval");
    return "";
  }
  
  if (!fileManager.exists(GPT_DATA_LOG_PATH)) {
    logger->warning("GPT learning data file does not exist");
    return "";
  }
  
  return fileManager.readFile(GPT_DATA_LOG_PATH);
  #else
  return "";
  #endif
}

// Clear learning data (useful for testing or resetting learning)
bool clearGPTLearningData() {
  #if GPT_LEARNING_ENABLED
  static Utils::FileManager fileManager;
  if (!fileManager.init()) {
    logger->error("Failed to initialize FileManager for GPT data clearing");
    return false;
  }
  
  if (!fileManager.exists(GPT_DATA_LOG_PATH)) {
    return true; // File doesn't exist, consider it "cleared"
  }
  
  bool success = fileManager.deleteFile(GPT_DATA_LOG_PATH);
  if (success) {
    logger->info("GPT learning data cleared");
  }
  return success;
  #else
  return false;
  #endif
}