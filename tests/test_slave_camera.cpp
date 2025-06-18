#include <Arduino.h>
#include <unity.h>
#include "app.h"

// Buffer for receiving camera data
uint8_t cameraBuffer[32768]; // 32KB buffer

void setUp(void) {
  // Setup code for each test
  setupSPI();  // Initialize SPI
  delay(500);  // Give time for initialization
}

void tearDown(void) {
  // Clean up after each test
}

// Test that we can ping the slave device
void test_ping_slave() {
  bool pingResult = sendPingToSlave(1000);
  TEST_ASSERT_TRUE_MESSAGE(pingResult, "Failed to ping slave device");
  if (pingResult) {
    Serial.println("Successfully pinged slave device");
  }
}

// Test requesting a single camera frame
void test_request_camera_frame() {
  // First make sure we can ping the slave
  TEST_ASSERT_TRUE_MESSAGE(sendPingToSlave(1000), "Failed to ping slave before camera test");
  
  // Clear buffer
  memset(cameraBuffer, 0, sizeof(cameraBuffer));
  
  // Request camera data from slave
  int bytesReceived = requestCameraDataFromSlave(cameraBuffer, sizeof(cameraBuffer), 2000);
  
  // Check if we got data
  TEST_ASSERT_MESSAGE(bytesReceived > 0, "Failed to receive camera data from slave");
  
  // Print information about the data received
  Serial.printf("Received %d bytes of camera data from slave\n", bytesReceived);
  
  // Check for JPEG image format (FF D8 header)
  bool isJpeg = false;
  if (bytesReceived >= 10) {
    // Check if there's a header and then JPEG data
    if (cameraBuffer[0] == 0x21) { // CAMERA_DATA_RESPONSE
      // Extract width, height from header
      uint16_t width = (cameraBuffer[2] << 8) | cameraBuffer[1];
      uint16_t height = (cameraBuffer[4] << 8) | cameraBuffer[3];
      
      Serial.printf("Camera image dimensions: %dx%d\n", width, height);
      
      // Check for JPEG header after our 8-byte header
      if (bytesReceived >= 10 && cameraBuffer[8] == 0xFF && cameraBuffer[9] == 0xD8) {
        isJpeg = true;
        Serial.println("Received data is a JPEG image with header");
      }
    }
    // Otherwise check if it's a raw JPEG
    else if (cameraBuffer[0] == 0xFF && cameraBuffer[1] == 0xD8) {
      isJpeg = true;
      Serial.println("Received data is a raw JPEG image");
    }
  }
  
  // Print first 16 bytes for debugging
  Serial.print("First 16 bytes: ");
  for (int i = 0; i < min(16, bytesReceived); i++) {
    Serial.printf("%02X ", cameraBuffer[i]);
  }
  Serial.println();
  
  // Assert that we received a JPEG image
  TEST_ASSERT_MESSAGE(isJpeg, "Did not receive a valid JPEG image");
}

void setup() {
  // Wait for serial to be ready
  delay(2000);
  
  UNITY_BEGIN();
  RUN_TEST(test_ping_slave);
  RUN_TEST(test_request_camera_frame);
  UNITY_END();
}

void loop() {
  // Empty
}
