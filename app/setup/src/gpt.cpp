#include <Arduino.h>
#include "setup/setup.h"
#include "tasks/register.h"

Services::GPTAdapter *gptAdapter;

void setupGPT(){
	gptAdapter = new Services::GPTAdapter();
	#if GPT_ENABLED
	gptAdapter->init(GPT_API_KEY);
	gptAdapter->setModel(GPT_MODEL);
	gptAdapter->setMaxTokens(GPT_MAX_TOKENS);
	gptAdapter->setTemperature(GPT_TEMPERATURE);
  #endif
}