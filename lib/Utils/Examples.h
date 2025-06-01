#pragma once

#include <Arduino.h>

namespace Utils {

/**
 * @file Examples.h
 * @brief Example usage of SpiAllocator, I2CScanner, and Sstring
 * 
 * This file contains example functions showing how to use the SpiAllocator,
 * I2CScanner, and Sstring classes.
 */

/**
 * @brief Example usage of SpiJsonDocument
 * 
 * This function demonstrates how to use the SpiJsonDocument class to create
 * and manipulate JSON documents in external SPI RAM.
 */
void spiJsonExample();

/**
 * @brief Example usage of I2CScanner
 * 
 * This function demonstrates how to use the I2CScanner class to scan
 * for I2C devices on a bus.
 */
void i2cScannerExample(int sdaPin, int sclPin);

/**
 * @brief Example usage of Sstring
 * 
 * This function demonstrates how to use the Sstring class to create
 * and manipulate strings in external SPI RAM.
 */
void sstringExample();

} // namespace Utils
