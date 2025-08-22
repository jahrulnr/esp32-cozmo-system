#include <Arduino.h>
#include <Wire.h>
#include <vector>

namespace Utils {

/**
 * @brief A comprehensive utility for I2C bus scanning, device detection and diagnostics
 * 
 * The I2CScanner provides tools for working with I2C devices including:
 * - Bus scanning to find connected devices
 * - Device identification by address and register signatures 
 * - Connection testing and quality assessment
 * - Diagnostic tools for troubleshooting I2C issues
 * - Non-blocking scanning for runtime diagnostics
 * - Multi-bus support for complex systems
 * 
 * All methods are static and can be called without instantiation:
 * `Utils::I2CScanner::scan();`
 * 
 * @note This class is designed to work with ESP32 and other Arduino-compatible platforms
 * that use the TwoWire library for I2C communication.
 */
class I2CScanner {
public:
    /**
     * @brief Structure to store device signature information for identification
     */
    struct DeviceSignature {
        uint8_t address;      ///< I2C address of device
        uint8_t idRegister;   ///< Register address containing identification value
        uint8_t idValue;      ///< Expected value from ID register
        uint8_t idMask;       ///< Mask to apply to returned value (for partial matching)
        const char* name;     ///< Human-readable device name
    };
    
    /**
     * @brief Scan an I2C bus for connected devices
     * 
     * Performs a complete scan of the I2C address space and reports all devices that respond.
     * This is useful for initial setup and debugging I2C connections.
     * 
     * @param wire The TwoWire instance to use (default = Wire)
     * @param startAddress Start address for scan, 1-127 (default = 1)
     * @param endAddress End address for scan, 1-127 (default = 127)
     * @param printOutput Whether to print scan results to Serial (default = true)
     * @return int Number of devices found
     */
    static int scan(TwoWire& wire = Wire, uint8_t startAddress = 1, 
                    uint8_t endAddress = 127, bool printOutput = true);

    /**
     * @brief Initialize an I2C bus with custom pins and scan for devices
     * 
     * Initializes a TwoWire instance with specified pins and frequency, then
     * performs a scan to detect connected devices. Useful for custom I2C bus setups.
     * 
     * @param sda SDA pin number
     * @param scl SCL pin number
     * @param frequency Bus frequency in Hz (default: 100000)
     * @param wire The TwoWire instance to use (default = Wire)
     * @return int Number of devices found
     */
    static int initAndScan(int sda, int scl, uint32_t frequency = 100000, TwoWire& wire = Wire);

    /**
     * @brief Check if a specific I2C device is present at an address
     * 
     * Quick check to see if a device responds at a specific address.
     * 
     * @param address Device address (0-127)
     * @param wire The TwoWire instance to use (default = Wire)
     * @return true Device is present
     * @return false Device is not present
     */
    static bool devicePresent(uint8_t address, TwoWire& wire = Wire);
    
    /**
     * @brief Attempt to identify a device by its address and signature registers
     * 
     * Tries to identify common I2C devices by reading identification registers.
     * Returns a human-readable string with the device name or "Unknown device" if 
     * it cannot be identified.
     * 
     * @param address I2C address of the device
     * @param wire The TwoWire instance to use (default = Wire)
     * @return String Device name or "Unknown device"
     */
    static String identifyDevice(uint8_t address, TwoWire& wire = Wire);
    
    /**
     * @brief Perform a detailed scan with device identification and connection quality
     * 
     * Enhanced version of scan() that includes device identification and connection
     * quality metrics. Outputs a formatted table with detailed device information.
     * 
     * @param wire The TwoWire instance to use (default = Wire)
     */
    static void advancedScan(TwoWire& wire = Wire);
    
    /**
     * @brief Start a non-blocking background scan of the I2C bus
     * 
     * Initiates an asynchronous scan that can be checked later. Useful for 
     * runtime diagnostics without blocking the main execution flow.
     * 
     * @param wire The TwoWire instance to use (default = Wire)
     */
    static void beginAsyncScan(TwoWire& wire = Wire);
    
    /**
     * @brief Check if the asynchronous scan has completed
     * 
     * @return true Scan is complete, results are available
     * @return false Scan is still in progress
     */
    static bool isAsyncScanComplete();
    
    /**
     * @brief Get the results from an asynchronous scan
     * 
     * @return std::vector<uint8_t> Vector of device addresses found
     * @note Call only after isAsyncScanComplete() returns true
     */
    static std::vector<uint8_t> getAsyncScanResults();
    
    /**
     * @brief Test the quality of an I2C device connection
     * 
     * Performs detailed connection tests and reports latency, reliability,
     * and potential issues with the connection to a specific device.
     * 
     * @param address I2C address of the device to test
     * @param wire The TwoWire instance to use (default = Wire)
     * @param printOutput Whether to print results to Serial (default = true)
     * @return true Connection is stable
     * @return false Connection has issues
     */
    static bool testDeviceConnection(uint8_t address, TwoWire& wire = Wire,
                                   bool printOutput = true);
    
    /**
     * @brief Provide comprehensive diagnostics for I2C connection issues
     * 
     * Deep analysis of connection issues with suggestions for 
     * resolution. Checks bus health, device response, and common problems.
     * 
     * @param address I2C address of the problematic device
     * @param wire The TwoWire instance to use (default = Wire)
     */
    static void diagnoseConnectionIssues(uint8_t address, TwoWire& wire = Wire);

private:
    // Database of known device signatures
    static const DeviceSignature deviceDb[];
    
    // For async scanning
    static std::vector<uint8_t> _asyncResults;
    static bool _asyncScanRunning;
    static TwoWire* _asyncWire;
};

} // namespace Utils
