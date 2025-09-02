#include "../setup.h"

Utils::FileManager *fileManager;

void setupFilemanager() {
  fileManager = new Utils::FileManager();
  if (!fileManager->init()) {
    logger->error("FileManager initialization failed");
  }
}