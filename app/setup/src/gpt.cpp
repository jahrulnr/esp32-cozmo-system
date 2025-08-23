#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

void setupGPT(){
	gptAdapter = new Communication::GPTAdapter();
	#if GPT_ENABLED
	gptAdapter->init(GPT_API_KEY);
	gptAdapter->setModel(GPT_MODEL);
	gptAdapter->setMaxTokens(GPT_MAX_TOKENS);
	gptAdapter->setTemperature(GPT_TEMPERATURE);
  #endif
}



void sendGPT(const String &prompt, Communication::GPTAdapter::ResponseCallback callback){
	gptRequest *data = new gptRequest{
		prompt: prompt,
		callback: callback,
	};

	xTaskCreate(gptChatTask, "gptChatTask", 20 * 1024, data, 10, &gptTaskHandle);
}