#include <Arduino.h>
#include <unity.h>
#include "app.h"

// Buffer to receive camera data
#define MAX_CAMERA_DATA_SIZE 32768  // 32KB buffer for camera data
uint8_t cameraDataBuffer[MAX_CAMERA_DATA_SIZE];

void setUp(void) {
  // Set up required for each test
  setupSPI();
  delay(500);  // Give some time for SPI to initialize
}

void tearDown(void) {
  // Clean up after each test
}

void test_ping_slave_connection() {
  TEST_ASSERT_TRUE_MESSAGE(sendPingToSlave(1000), "Failed to ping SPI slave");
}

void test_camera_data_request() {
  // Clear buffer
  memset(cameraDataBuffer, 0, MAX_CAMERA_DATA_SIZE);
  
  // Request camera data
  int bytesReceived = requestCameraDataFromSlave(cameraDataBuffer, MAX_CAMERA_DATA_SIZE);
  
  // Check if we got a valid response
  TEST_ASSERT_MESSAGE(bytesReceived > 0, "Failed to receive camera data or received 0 bytes");
  
  // Basic validation of the data
  if (bytesReceived > 0) {
    // Print info about the received data
    Serial.printf("Received %d bytes of camera data\n", bytesReceived);
    
    // Optional: Print first few bytes for debugging
    Serial.println("First 16 bytes of camera data:");
    for (int i = 0; i < min(16, bytesReceived); i++) {
      Serial.printf("%02X ", cameraDataBuffer[i]);
    }
    Serial.println();
    
    // If this is image data, these might be JPEG header bytes (0xFF, 0xD8)
    // but this depends on the format used by the camera module on the slave
    if (bytesReceived > 1) {
      Serial.printf("First two bytes: %02X %02X\n", 
                    cameraDataBuffer[0], cameraDataBuffer[1]);
    }
  }
}

void test_camera_data_parsing() {
  // This test would be expanded based on the specific format of the camera data
  // For now, it just requests data and checks if it's parseable in some way
  
  memset(cameraDataBuffer, 0, MAX_CAMERA_DATA_SIZE);
  int bytesReceived = requestCameraDataFromSlave(cameraDataBuffer, MAX_CAMERA_DATA_SIZE);
  
  if (bytesReceived > 0) {
    // Check if data seems to be valid (this is a very basic check, update based on your data format)
    // For example, if the data is a JPEG image, check for JPEG header
    bool isValidFormat = false;
    
    // Check for JPEG header (0xFF, 0xD8) at the start
    if (bytesReceived >= 2 && cameraDataBuffer[0] == 0xFF && cameraDataBuffer[1] == 0xD8) {
      isValidFormat = true;
      Serial.println("Data appears to be in JPEG format");
    } 
    // Add other format checks as needed
    else {
      Serial.println("Data format not recognized or not an image");
    }
    
    TEST_ASSERT_MESSAGE(isValidFormat, "Camera data received but format is not recognized");
  } else {
    // Skip test if no data received
    TEST_IGNORE_MESSAGE("No camera data received, skipping format test");
  }
}

void setup() {
  delay(2000); // wait for the serial port to connect
  UNITY_BEGIN();
  
  RUN_TEST(test_ping_slave_connection);
  RUN_TEST(test_camera_data_request);
  RUN_TEST(test_camera_data_parsing);
  
  UNITY_END();
}

void loop() {
  // Nothing to do here
}
