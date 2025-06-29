/*
 * SD_MMC Pin Configuration Example for ESP32S3
 * 
 * Add these pin definitions to your main configuration or pins.h file
 * based on your hardware setup.
 */

#pragma once

#ifdef CONFIG_IDF_TARGET_ESP32S3

// Example SD_MMC pin definitions for ESP32S3
// Adjust these based on your actual hardware connections

// Required pins for all modes
#define SD_MMC_CLK    14  // Clock pin
#define SD_MMC_CMD    15  // Command pin
#define SD_MMC_D0     2   // Data 0 pin (required for 1-bit and 4-bit mode)

// Additional pins for 4-bit mode (optional)
// Comment out these lines if you only want 1-bit mode
#define SD_MMC_D1     4   // Data 1 pin (4-bit mode)
#define SD_MMC_D2     12  // Data 2 pin (4-bit mode)
#define SD_MMC_D3     13  // Data 3 pin (4-bit mode)

/*
 * Common ESP32S3 SD_MMC pin configurations:
 * 
 * Configuration 1 (1-bit mode):
 * - CLK: GPIO14
 * - CMD: GPIO15
 * - D0:  GPIO2
 * 
 * Configuration 2 (4-bit mode):
 * - CLK: GPIO14
 * - CMD: GPIO15
 * - D0:  GPIO2
 * - D1:  GPIO4
 * - D2:  GPIO12
 * - D3:  GPIO13
 * 
 * Note: Make sure these pins don't conflict with other peripherals
 * in your design (SPI, I2C, UART, etc.)
 */

#endif // CONFIG_IDF_TARGET_ESP32S3
