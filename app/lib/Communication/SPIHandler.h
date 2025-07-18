#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "Config.h"
#include "lib/Utils/Logger.h"
#include <queue>

namespace Communication {

/**
 * @brief Command codes for SPI communication
 * These must match with the slave device
 */
enum class SPICommand : uint8_t {
  PING = 0x01,
  PONG = 0x02,
  ACK = 0xAA,
  NACK = 0xFF
};

// Struct to store SPI data for the receive queue
struct SPIDataPacket {
  uint8_t* data;
  size_t length;
  bool processed;
  
  SPIDataPacket(const uint8_t* srcData, size_t len) {
    length = len;
    processed = false;
    data = new uint8_t[length];
    memcpy(data, srcData, length);
  }
  
  ~SPIDataPacket() {
    if (data) {
      delete[] data;
    }
  }
};

/**
 * @brief SPI master handler for ESP32
 * 
 * This class handles the SPI master functionality including:
 * - Initializing the SPI master interface
 * - Sending data to the slave
 * - Receiving data from the slave
 * - Implementing a ping-pong mechanism for testing connectivity
 */
class SPIHandler {
public:
  /**
   * @brief Get the singleton instance of SPIHandler
   * @return SPIHandler& Reference to the singleton instance
   */
  static SPIHandler& getInstance();

  /**
   * @brief Initialize the SPI master interface
   * @param frequency SPI clock frequency in Hz
   * @param mode SPI mode (0-3)
   * @param sckPin SCK pin number (optional, default is CONFIG defined)
   * @param misoPin MISO pin number (optional, default is CONFIG defined)
   * @param mosiPin MOSI pin number (optional, default is CONFIG defined)
   * @param csPin CS pin number (optional, default is CONFIG defined)
   * @return true if initialization was successful, false otherwise
   */
  bool init(uint32_t frequency = 1000000, uint8_t mode = SPI_MODE0,
           int sckPin = -1, int misoPin = -1, int mosiPin = -1, int csPin = -1);

  /**
   * @brief Set which SPI host to use (advanced)
   * This should be called before init()
   * @param host SPI host, typically HSPI(1) or VSPI(2) on ESP32
   */
  void setSPIHost(uint8_t host);
  
  /**
   * @brief Send data over SPI and queue a receive operation
   * @param txData Pointer to data to send
   * @param length Length of data to transfer in bytes
   * @return true if sending was successful, false otherwise
   */
  bool send(const uint8_t* txData, size_t length = SPI_BUFFER_SIZE);
  bool sendCommand(Communication::SPICommand cmd);

  /**
   * @brief Process the next pending receive data packet
   * @return true if a packet was processed, false if queue is empty
   */
  bool processNextReceive();
  
  /**
   * @brief Register a callback function for received data
   * @param callback Function to call when data is received
   */
  typedef void (*ReceiveCallback)(const uint8_t* data, size_t length);
  void setReceiveCallback(ReceiveCallback callback);
  
  /**
   * @brief Check if there are pending receive packets
   * @return Number of pending packets
   */
  size_t pendingReceiveCount();

private:
  SPIHandler();  // Private constructor for singleton
  ~SPIHandler();

  // Prevent copying
  SPIHandler(const SPIHandler&) = delete;
  SPIHandler& operator=(const SPIHandler&) = delete;

  /**
   * @brief Execute an SPI transaction to receive data
   * @param buffer Pointer to buffer for received data
   * @param length Length of data to receive in bytes
   * @return true if receiving was successful, false otherwise
   */
  bool receive(uint8_t* buffer, size_t length = SPI_BUFFER_SIZE);
  
  /**
   * @brief Handle received data internally or through callback
   * @param data Pointer to received data
   * @param length Length of received data
   */
  void handleReceivedData(const uint8_t* data, size_t length = SPI_BUFFER_SIZE);

  // Buffer management
  uint8_t* _txBuffer;
  uint8_t* _rxBuffer;
  size_t _bufferSize;
  
  // Queue for received data packets
  std::queue<SPIDataPacket*> _receiveQueue;
  
  // Callback for receive events
  ReceiveCallback _receiveCallback;
  
  // SPI instance
  SPIClass* _spi;
  uint32_t _frequency;
  uint8_t _mode;
  uint8_t _spiHost;  // SPI host number (HSPI or VSPI)
  
  // Logger instance
  Utils::Logger* _logger;

  // Flag to track initialization state
  bool _initialized;
  
  // SPI pins
  uint8_t _sckPin;
  uint8_t _misoPin;
  uint8_t _mosiPin;
  uint8_t _csPin;
};

} // namespace Communication
