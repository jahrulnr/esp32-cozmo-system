#include <Arduino.h>
#include "setup/setup.h"

Logic::ScanArea *scanArea;

void setupScanArea() {
  scanArea = new Logic::ScanArea(orientation, distanceSensor);
  scanArea->update();
}
