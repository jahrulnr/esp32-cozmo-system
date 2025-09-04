#include "../setup.h"

Utils::Logger *logger;

void setupLogger() {
  // Initialize Logger
  logger = &Utils::Logger::getInstance();
  logger->init(true, false);
  logger->setLogLevel(Utils::LogLevel::DEBUG);
  logger->info("Logger initialized");
}