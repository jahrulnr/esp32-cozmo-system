#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

Services::GPTService *gptService;

void setupGPT(){
	gptService = new Services::GPTService();
	#if GPT_ENABLED
	gptService->init(GPT_API_KEY);
	gptService->setModel(GPT_MODEL);
	gptService->setMaxTokens(GPT_MAX_TOKENS);
	gptService->setTemperature(GPT_TEMPERATURE);
  #endif
}