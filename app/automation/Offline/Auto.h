#pragma once

#include <setup/setup.h>

namespace Auto {

class OfflineMode {
private:
	const char* _tag = "Auto";

	typedef enum {
		ACTIVITY = 0,
		DANCE,
	} AutoType_t;

	long _lastExec = 0;
	Utils::Sstring _lastCmd = "";
	const Utils::Sstring _baseTemplate = "/config/template.txt";
	const Utils::Sstring _danceTemplate = "/config/dance_template.txt";
	std::map<Utils::Sstring, Utils::Sstring> _commands;
	esp_err_t _parseTemplate(const char* txtFile, std::vector<Utils::Sstring>* result) {
		if(!fileManager || !fileManager->exists(txtFile)){
			ESP_LOGW(_tag, "FileManager or file %s is empty", txtFile);
			return ESP_ERR_INVALID_STATE;
		}

		Utils::Sstring rTemplate = fileManager->readFile(txtFile);
		*result = rTemplate.split("\n");
		return ESP_OK;
	}

public:
	OfflineMode(){
		_commands.clear();

		std::vector<Utils::Sstring> result;
		esp_err_t parseCommands = _parseTemplate(_baseTemplate.c_str(), &result);
		if (parseCommands == ESP_OK)
			_commands.insert_or_assign("activity", result);

		result.clear();
		parseCommands = _parseTemplate(_danceTemplate.c_str(), &result);
		if (parseCommands == ESP_OK)
			_commands.insert_or_assign("dance", result);
	}

	esp_err_t doSomething();
};

} // end namespace