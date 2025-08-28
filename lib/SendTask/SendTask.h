#pragma once

#include <Arduino.h>

namespace Command {
	using cmd = std::function<void(void)>;
	void Send(cmd command);
}