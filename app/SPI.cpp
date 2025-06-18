#include "app.h"
#include <mbedtls/base64.h>
#include <ArduinoJson.h>

// External reference to slave camera data structure defined in app.ino
extern struct SlaveCameraData slaveCameraData;

/**
 * Callback function to process received SPI messages
 * This will be called when SPI data is received from the slave
 */
void onSPIMessageReceived(const uint8_t* data, size_t length) {
    Utils::Logger& logger = Utils::Logger::getInstance();
    
    if (length == 0) return;
    
    logger.debug("SPI message received: %d bytes", length);
    
    // Print the first few bytes for debugging
    String dataStr = "";
    for (size_t i = 0; i < min(length, (size_t)8); i++) {
        char buf[8];
        snprintf(buf, sizeof(buf), "0x%02X ", data[i]);
        dataStr += buf;
    }
    logger.debug("SPI data: %s", dataStr.c_str());
    
    // Process based on command type
    if (length > 0) {
        Communication::SPICommand cmd = static_cast<Communication::SPICommand>(data[0]);
        
        switch (cmd) {
            case Communication::SPICommand::PONG:
                logger.info("Received PONG from slave");
                break;
                
            case Communication::SPICommand::CAMERA_DATA_RESPONSE:
                logger.info("Received camera data response with %d bytes", length);
                // Process camera metadata from slave
                if (length >= 16) { // We need at least 16 bytes for the metadata
                    // Clean up previous data if any
                    if (slaveCameraData.imageData) {
                        heap_caps_free(slaveCameraData.imageData);
                        slaveCameraData.imageData = nullptr;
                    }
                    if (slaveCameraData.blockReceived) {
                        heap_caps_free(slaveCameraData.blockReceived);
                        slaveCameraData.blockReceived = nullptr;
                    }
                    
                    // Extract camera metadata
                    slaveCameraData.dataVersion = data[1];
                    slaveCameraData.width = (data[2] << 8) | data[3];
                    slaveCameraData.height = (data[4] << 8) | data[5];
                    slaveCameraData.totalBlocks = (data[6] << 8) | data[7];
                    slaveCameraData.blockSize = (data[8] << 8) | data[9];
                    slaveCameraData.totalSize = ((uint32_t)data[10] << 24) | 
                                             ((uint32_t)data[11] << 16) | 
                                             ((uint32_t)data[12] << 8) | 
                                              data[13];
                    
                    // Allocate memory for the image data and block tracking
                    slaveCameraData.imageData = (uint8_t*)heap_caps_malloc(slaveCameraData.totalSize, MALLOC_CAP_DMA | MALLOC_CAP_DEFAULT);
                    slaveCameraData.blockReceived = (bool*)heap_caps_calloc(slaveCameraData.totalBlocks, sizeof(bool), MALLOC_CAP_DMA | MALLOC_CAP_DEFAULT);
                    
                    if (!slaveCameraData.imageData || !slaveCameraData.blockReceived) {
                        logger.error("Failed to allocate memory for camera data! data size: %d", slaveCameraData.totalSize);
                        if (slaveCameraData.imageData) {
                            heap_caps_free(slaveCameraData.imageData);
                            slaveCameraData.imageData = nullptr;
                        }
                        if (slaveCameraData.blockReceived) {
                            heap_caps_free(slaveCameraData.blockReceived);
                            slaveCameraData.blockReceived = nullptr;
                        }
                        slaveCameraData.dataAvailable = false;
                    } else {
                        // Reset counters and flags
                        slaveCameraData.receivedBlocks = 0;
                        slaveCameraData.frameComplete = false;
                        slaveCameraData.dataAvailable = true;
                        
                        logger.info("Camera data metadata received: %dx%d, %d bytes, %d blocks",
                                  slaveCameraData.width, slaveCameraData.height, 
                                  slaveCameraData.totalSize, slaveCameraData.totalBlocks);
                        
                        // Start requesting blocks
                        requestCameraDataBlockFromSlave(0);
                    }
                }
                break;
                
            case Communication::SPICommand::CAMERA_DATA_BLOCK_RESPONSE:
                // Process camera data block from slave
                if (length >= 6) { // We need at least 6 bytes for header plus some data
                    // Extract block index and data length
                    uint16_t blockIndex = (data[1] << 8) | data[2];
                    uint16_t dataLength = (data[3] << 8) | data[4];
                    
                    logger.info("Received camera data block %d, %d bytes", blockIndex, dataLength);
                    
                    // Validate the data
                    if (!slaveCameraData.dataAvailable || !slaveCameraData.imageData || !slaveCameraData.blockReceived) {
                        logger.error("Camera data not initialized");
                        break;
                    }
                    
                    if (blockIndex >= slaveCameraData.totalBlocks) {
                        logger.error("Invalid block index: %d >= %d", blockIndex, slaveCameraData.totalBlocks);
                        break;
                    }
                    
                    if (slaveCameraData.blockReceived[blockIndex]) {
                        logger.warning("Block %d already received, ignoring", blockIndex);
                        break;
                    }
                    
                    if (length < 5 + dataLength) {
                        logger.error("Incomplete block data received");
                        break;
                    }
                    
                    // Calculate offset into the image buffer
                    size_t offset = blockIndex * slaveCameraData.blockSize;
                    if (offset + dataLength > slaveCameraData.totalSize) {
                        logger.error("Block data exceeds buffer size");
                        break;
                    }
                    
                    // Copy the data to the image buffer
                    memcpy(slaveCameraData.imageData + offset, &data[5], dataLength);
                    
                    // Mark this block as received
                    slaveCameraData.blockReceived[blockIndex] = true;
                    slaveCameraData.receivedBlocks++;
                    
                    // Check if we've received all blocks
                    if (slaveCameraData.receivedBlocks == slaveCameraData.totalBlocks) {
                        logger.info("All camera data blocks received, frame complete");
                        slaveCameraData.frameComplete = true;
                        
                        // Signal that the frame is ready to be processed
                        // This could trigger a callback or set a flag for the main loop
                    }
                    // else {
                    //     // Request the next block
                    //     // Find the next unreceived block
                    //     for (uint16_t i = 0; i < slaveCameraData.totalBlocks; i++) {
                    //         if (!slaveCameraData.blockReceived[i]) {
                    //             requestCameraDataBlockFromSlave(i);
                    //             break;
                    //         }
                    //     }
                    // }
                }
                break;
            
            case Communication::SPICommand::ACK:
                logger.debug("Received ACK from slave");
                break;
                
            case Communication::SPICommand::NACK:
                logger.warning("Received NACK from slave");
                
                // Check if the NACK includes an error code
                if (length > 1) {
                    uint8_t errorCode = data[1];
                    logger.warning("NACK error code: 0x%02X", errorCode);
                    
                    // Handle specific error codes
                    switch (errorCode) {
                        case 0x01:
                            logger.warning("Camera not available on slave");
                            break;
                        case 0x02:
                            logger.warning("Failed to capture camera frame");
                            break;
                        case 0x03:
                            logger.warning("Failed to allocate memory for camera frame");
                            break;
                        // Add more error codes as needed
                    }
                }
                break;
                
            default:
                logger.debug("Received unknown command: 0x%02X", (uint8_t)cmd);
        }
    }
}

void setupSPI() {
    Utils::Logger& logger = Utils::Logger::getInstance();
    
    // Initialize the SPI handler singleton
    spiHandler = &Communication::SPIHandler::getInstance();
    
    // Explicitly configure SPI for HSPI to avoid conflicts with PSRAM
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2)
    spiHandler->setSPIHost(SPI2_HOST);  // Use SPI2_HOST for S2/S3
#else
    spiHandler->setSPIHost(HSPI);       // Use HSPI for ESP32
#endif
    
    // Lower SPI frequency for better stability (100kHz)
    // Use SPI_MODE1 which may work better with certain SPI slaves
    if (!spiHandler->init(10 * 1000000, SPI_MODE1, SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_ESP32_SS)) {
        logger.error("Failed to initialize SPI");
        return;
    } else {
        logger.info("SPI initialized with SCK=%d, MISO=%d, MOSI=%d, CS=%d", 
                  SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_ESP32_SS);
    }
    
    // Register callback for received SPI data
    spiHandler->setReceiveCallback(onSPIMessageReceived);
    logger.info("SPI message handler registered");
    
    // Send an initial ping to check if slave is responsive
    if (sendPingToSlave()) {
        logger.info("Initial ping sent to slave device");
    }
}

bool sendPingToSlave() {
    if (!spiHandler) {
        Utils::Logger::getInstance().error("SPI handler not initialized");
        return false;
    }

    bool result = spiHandler->sendCommand(Communication::SPICommand::PING);
    if (result) {
        Utils::Logger::getInstance().debug("Ping sent to slave device");
    } else {
        Utils::Logger::getInstance().error("Failed to send ping to slave device");
    }
    return result;
}

/**
 * Request camera data from the slave device
 * @return true if request was sent successfully, false otherwise
 */
bool requestCameraDataFromSlave() {
    if (!spiHandler) {
        Utils::Logger::getInstance().error("SPI handler not initialized");
        return false;
    }
    
    Utils::Logger::getInstance().info("Requesting camera data from slave");
    return spiHandler->sendCommand(Communication::SPICommand::CAMERA_DATA_REQUEST);
}

/**
 * Request a specific block of camera data from the slave device
 * @param blockNumber The block number to request
 * @return true if request was sent successfully, false otherwise
 */
bool requestCameraDataBlockFromSlave(uint16_t blockNumber) {
    if (!spiHandler) {
        Utils::Logger::getInstance().error("SPI handler not initialized");
        return false;
    }
    
    uint8_t cmdData[3] = {
        static_cast<uint8_t>(Communication::SPICommand::CAMERA_DATA_BLOCK_REQUEST),
        static_cast<uint8_t>((blockNumber >> 8) & 0xFF),  // High byte
        static_cast<uint8_t>(blockNumber & 0xFF)          // Low byte
    };
    
    Utils::Logger::getInstance().info("Requesting camera data block %d from slave", blockNumber);
    return spiHandler->send(cmdData);
}

/**
 * Reset slave camera data and free any allocated memory
 */
void resetSlaveCameraData() {
    if (slaveCameraData.imageData) {
        heap_caps_free(slaveCameraData.imageData);
        slaveCameraData.imageData = nullptr;
    }
    
    if (slaveCameraData.blockReceived) {
        heap_caps_free(slaveCameraData.blockReceived);
        slaveCameraData.blockReceived = nullptr;
    }
    
    slaveCameraData.dataAvailable = false;
    slaveCameraData.width = 0;
    slaveCameraData.height = 0;
    slaveCameraData.totalSize = 0;
    slaveCameraData.totalBlocks = 0;
    slaveCameraData.blockSize = 0;
    slaveCameraData.receivedBlocks = 0;
    slaveCameraData.frameComplete = false;
    slaveCameraData.dataVersion = 0;
}

/**
 * Check if a complete camera frame is available from the slave
 * @return true if a complete frame is available
 */
bool isSlaveCameraFrameComplete() {
    return slaveCameraData.dataAvailable && slaveCameraData.frameComplete;
}

/**
 * Get a pointer to the slave camera image data
 * This will not transfer ownership of the data
 * @return Pointer to the image data or nullptr if no data is available
 */
uint8_t* getSlaveCameraImageData() {
    return (slaveCameraData.dataAvailable && slaveCameraData.frameComplete) ? 
           slaveCameraData.imageData : nullptr;
}

/**
 * Get the size of the slave camera image data
 * @return Size of the image data in bytes or 0 if no data is available
 */
uint32_t getSlaveCameraImageSize() {
    return (slaveCameraData.dataAvailable && slaveCameraData.frameComplete) ? 
           slaveCameraData.totalSize : 0;
}

/**
 * Get the dimensions of the slave camera image
 * @param width Pointer to store the width (will be set to 0 if no data is available)
 * @param height Pointer to store the height (will be set to 0 if no data is available)
 */
void getSlaveCameraImageDimensions(uint16_t* width, uint16_t* height) {
    if (width) *width = slaveCameraData.dataAvailable ? slaveCameraData.width : 0;
    if (height) *height = slaveCameraData.dataAvailable ? slaveCameraData.height : 0;
}

/**
 * Determine if the slave camera data is already in a displayable format like JPEG
 * This assumes the slave is sending JPEG data from the ESP32-CAM
 * @return true if the data appears to be in JPEG format
 */
bool isSlaveCameraDataJPEG() {
    // Check if we have data and that it's complete
    if (!slaveCameraData.dataAvailable || !slaveCameraData.frameComplete || !slaveCameraData.imageData) {
        return false;
    }
    
    // Check for JPEG magic bytes (SOI marker - Start Of Image)
    // A valid JPEG starts with FF D8 (SOI) and typically has an FF Ex segment right after
    if (slaveCameraData.totalSize >= 3) {
        bool hasJpegHeader = (slaveCameraData.imageData[0] == 0xFF && 
                             slaveCameraData.imageData[1] == 0xD8);
        
        if (hasJpegHeader) {
            // Also check for the third byte which is typically 0xFF in a JPEG
            // followed by a segment marker like APP0 (0xE0) or similar
            bool hasSegmentMarker = (slaveCameraData.totalSize >= 3 && slaveCameraData.imageData[2] == 0xFF);
            
            Utils::Logger::getInstance().debug("JPEG validation: Header %s, Segment marker %s", 
                hasJpegHeader ? "valid" : "invalid",
                hasSegmentMarker ? "found" : "not found");
                
            // Even if the third byte is not 0xFF, we'll still accept it if the first two bytes are valid
            return true;
        }
    }
    
    return false;
}

/**
 * Process a complete slave camera frame
 * This should be called when a complete frame is available
 * @return true if the frame was processed successfully
 */
bool processSlaveCameraFrame() {
    if (!isSlaveCameraFrameComplete()) {
        Utils::Logger::getInstance().error("No complete camera frame available");
        return false;
    }
    
    Utils::Logger& logger = Utils::Logger::getInstance();
    logger.info("Processing slave camera frame: %dx%d, %d bytes", 
               slaveCameraData.width, slaveCameraData.height, slaveCameraData.totalSize);
    
    // For now, just check if it's a JPEG and log that information
    if (isSlaveCameraDataJPEG()) {
        logger.info("Slave camera data is in JPEG format");
    } else {
        logger.info("Slave camera data is in a raw format");
    }
    
    // Here you would typically:
    // 1. Process the image data as needed
    // 2. Send it to any consumers (e.g., a web interface)
    // 3. Save it to storage if needed
    
    // For example, if there's a websocket connection, you could send the image:
    if (webSocket) {
        if (isSlaveCameraDataJPEG() && slaveCameraData.totalSize > 0) {
            // Log the last few bytes for debugging
            char lastBytesStr[32] = {0};
            for (int i = max(0, (int)slaveCameraData.totalSize - 10); i < slaveCameraData.totalSize; i++) {
                char byteStr[8];
                snprintf(byteStr, sizeof(byteStr), "%02X ", slaveCameraData.imageData[i]);
                strcat(lastBytesStr, byteStr);
            }
            logger.debug("Last bytes of camera data: %s", lastBytesStr);
            
            // JPEG data starts with specific marker (FF D8 FF) and ends with (FF D9)
            // Check for proper end marker to ensure valid JPEG data
            bool validJpegEnd = (slaveCameraData.totalSize >= 2 && 
                               slaveCameraData.imageData[slaveCameraData.totalSize - 2] == 0xFF && 
                               slaveCameraData.imageData[slaveCameraData.totalSize - 1] == 0xD9);
            
            // If the JPEG end marker is missing, try to fix it
            if (!validJpegEnd) {
                logger.warning("Invalid JPEG data - missing end marker, attempting to fix");
                
                // Ensure we have room for the marker
                if (slaveCameraData.totalSize + 2 <= slaveCameraData.blockSize * slaveCameraData.totalBlocks) {
                    // Add the JPEG end marker
                    slaveCameraData.imageData[slaveCameraData.totalSize] = 0xFF;
                    slaveCameraData.imageData[slaveCameraData.totalSize + 1] = 0xD9;
                    slaveCameraData.totalSize += 2;
                    logger.info("Added JPEG end marker, new size: %d bytes", slaveCameraData.totalSize);
                    validJpegEnd = true;
                } else {
                    logger.error("Cannot fix JPEG data - no room for end marker");
                    return false;
                }
            }
            
            // JSON header to tell client this is a JPEG image
            Utils::SpiJsonDocument headerDoc;
            headerDoc["type"] = "camera_frame";
            headerDoc["data"]["format"] = "jpeg";
            headerDoc["data"]["width"] = slaveCameraData.width;
            headerDoc["data"]["height"] = slaveCameraData.height;
            headerDoc["data"]["size"] = slaveCameraData.totalSize;
            
            String headerJson;
            serializeJson(headerDoc, headerJson);
            webSocket->sendText(-1, headerJson);
            delayMicroseconds(50);
            
            // Send the actual binary data
            webSocket->sendBinary(-1, slaveCameraData.imageData, slaveCameraData.totalSize);
            logger.info("Sent slave camera data to web clients");
        } else {
            logger.warning("Cannot send slave camera data - invalid format");
            return false;
        }
    } else {
        logger.warning("Failed to send slave camera data to web clients");
    }
    
    return true;
}