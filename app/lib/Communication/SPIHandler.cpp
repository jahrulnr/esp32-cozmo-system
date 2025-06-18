#include "SPIHandler.h"
#include <esp_camera.h>

namespace Communication {

SPIHandler& SPIHandler::getInstance() {
  static SPIHandler instance;
  return instance;
}

SPIHandler::SPIHandler() : 
  _txBuffer(nullptr), 
  _rxBuffer(nullptr), 
  _bufferSize(SPI_BUFFER_SIZE), // Default buffer size
  _receiveCallback(nullptr),
  _spi(nullptr), 
  _frequency(1000000), 
  _mode(SPI_MODE0), 
  _spiHost(HSPI), 
  _initialized(false),
  _sckPin(0),
  _misoPin(0),
  _mosiPin(0),
  _csPin(0) {
  
  _logger = &Utils::Logger::getInstance();
  
  // Allocate buffers
  _txBuffer = new uint8_t[_bufferSize];
  _rxBuffer = new uint8_t[_bufferSize];
  
  if (!_txBuffer || !_rxBuffer) {
    _logger->error("SPIHandler: Failed to allocate buffers");
  }
}

SPIHandler::~SPIHandler() {
  // Clean up SPI instance
  if (_spi) {
    _spi->end();
    delete _spi;
    _spi = nullptr;
  }
  
  // Clean up buffers
  if (_txBuffer) {
    delete[] _txBuffer;
    _txBuffer = nullptr;
  }
  
  if (_rxBuffer) {
    delete[] _rxBuffer;
    _rxBuffer = nullptr;
  }
  
  // Clean up receive queue
  while (!_receiveQueue.empty()) {
    SPIDataPacket* packet = _receiveQueue.front();
    _receiveQueue.pop();
    delete packet;
  }
}

void SPIHandler::setSPIHost(uint8_t host) {
  if (!_initialized) {
    _spiHost = host;
  } else {
    _logger->warning("SPIHandler: Cannot change SPI host after initialization");
  }
}

bool SPIHandler::init(uint32_t frequency, uint8_t mode,
                     int sckPin, int misoPin, int mosiPin, int csPin) {
  if (_initialized) {
    _logger->warning("SPIHandler: Already initialized");
    return true;
  }
  
  _frequency = frequency;
  _mode = mode;
  
  // Use provided pins or defaults from Config
  _sckPin = (sckPin != -1) ? sckPin : SPI_SCK_PIN;
  _misoPin = (misoPin != -1) ? misoPin : SPI_MISO_PIN;
  _mosiPin = (mosiPin != -1) ? mosiPin : SPI_MOSI_PIN;
  _csPin = (csPin != -1) ? csPin : SPI_ESP32_SS;
  
  _logger->info("SPIHandler: Initializing with SCK=%d, MISO=%d, MOSI=%d, CS=%d", 
               _sckPin, _misoPin, _mosiPin, _csPin);
  
  // Configure CS pin as OUTPUT
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH); // Deselect slave
  
  // Create and configure SPI instance
  _spi = new SPIClass(_spiHost);
  if (!_spi) {
    _logger->error("SPIHandler: Failed to create SPI instance");
    return false;
  }
  
  _spi->begin(_sckPin, _misoPin, _mosiPin, _csPin);
  _spi->setBitOrder(MSBFIRST);
  _spi->setDataMode(_mode);
  _spi->setFrequency(_frequency);
  
  _initialized = true;
  _logger->info("SPIHandler: Initialized successfully");
  return true;
}

bool SPIHandler::send(const uint8_t* txData, size_t length) {
  if (!_initialized || !_spi) {
    _logger->error("SPIHandler: Not initialized");
    return false;
  }
  
  if (!txData || length == 0 || length > _bufferSize) {
    _logger->error("SPIHandler: Invalid data or length");
    return false;
  }
  
  _logger->debug("SPIHandler: Sending %d bytes", length);
  
  // Prepare response buffer
  memset(_rxBuffer, 0, length);
  
  // Begin transaction
  _spi->beginTransaction(SPISettings(_frequency, MSBFIRST, _mode));
  
  // Select slave
  digitalWrite(_csPin, LOW);

  delayMicroseconds(5);
  
  // Transfer data
  _spi->transferBytes((uint8_t*)txData, _rxBuffer, length);

  delayMicroseconds(5);
  
  // Deselect slave
  digitalWrite(_csPin, HIGH);
  
  // End transaction
  _spi->endTransaction();
  
  // Queue the received data for processing
  SPIDataPacket* packet = new SPIDataPacket(_rxBuffer, SPI_BUFFER_SIZE);
  _receiveQueue.push(packet);
  
  _logger->debug("SPIHandler: Send complete, queued response for processing");
	delay(1);
  return true;
}

bool SPIHandler::sendCommand(Communication::SPICommand cmd) {
	uint8_t command = (uint8_t) cmd;
	return send(&command);
}

bool SPIHandler::receive(uint8_t* buffer, size_t length) {
  if (!_initialized || !_spi) {
    _logger->error("SPIHandler: Not initialized");
    return false;
  }
  
  if (!buffer || length == 0 || length > _bufferSize) {
    _logger->error("SPIHandler: Invalid buffer or length");
    return false;
  }
  
  _logger->debug("SPIHandler: Receiving %d bytes", length);
  
  // Begin transaction
  _spi->beginTransaction(SPISettings(_frequency, MSBFIRST, _mode));
  
  // Select slave
  digitalWrite(_csPin, LOW);
  
  // Send dummy bytes to read data
  memset(_txBuffer, 0, length);
  _spi->transferBytes(_txBuffer, buffer, length);
  
  // Deselect slave
  digitalWrite(_csPin, HIGH);
  
  // End transaction
  _spi->endTransaction();
  
  _logger->debug("SPIHandler: Receive complete");
  return true;
}

bool SPIHandler::processNextReceive() {
  if (_receiveQueue.empty()) {
    return false;
  }
  
  SPIDataPacket* packet = _receiveQueue.front();
  _receiveQueue.pop();
  
  // Process the received data
  handleReceivedData(packet->data, packet->length);
  
  // Clean up
  delete packet;
  return true;
}

void SPIHandler::handleReceivedData(const uint8_t* data, size_t length) {
  _logger->debug("SPIHandler: Processing received data, %d bytes", length);
  
  // If a callback is registered, call it
  if (_receiveCallback) {
    _receiveCallback(data, length);
  } else {
    // Basic handling when no callback is registered
    // Just log the first few bytes for debugging
    if (length > 0) {
      _logger->debug("SPIHandler: First byte: 0x%02X", data[0]);
      
      // If it's a command byte, log it
      if (length > 1 && data[0] < 0xFF) {
        SPICommand cmd = static_cast<SPICommand>(data[0]);
        switch (cmd) {
          case SPICommand::PONG:
            _logger->debug("SPIHandler: Received PONG");
            break;
          case SPICommand::ACK:
            _logger->debug("SPIHandler: Received ACK");
            break;
          case SPICommand::NACK:
            _logger->debug("SPIHandler: Received NACK");
            break;
          default:
            _logger->debug("SPIHandler: Received command 0x%02X", static_cast<uint8_t>(cmd));
            break;
        }
      }
    }
  }
}

void SPIHandler::setReceiveCallback(ReceiveCallback callback) {
  _receiveCallback = callback;
}

size_t SPIHandler::pendingReceiveCount() {
  return _receiveQueue.size();
}

} // namespace Communication
