#include "../setup.h"

Utils::Logger *logger;

void setupLogger() {
  // Initialize Logger
  logger = &Utils::Logger::getInstance();
  logger->init(true, true);
  logger->setLogLevel(Utils::LogLevel::INFO);
  logger->info("Logger initialized");
}