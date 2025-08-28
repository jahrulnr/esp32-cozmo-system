#include "SendTask.h"

namespace Command {

	void Send(cmd command){
		cmd* commandPtr = new cmd(command);
		xTaskCreate([](void* param){
			cmd* command = static_cast<cmd*>(param);
			(*command)();
			delete command;
			vTaskDelete(NULL);
		}, 
			"CommandTask",
			1024*4, commandPtr, 15, nullptr);
	}

}