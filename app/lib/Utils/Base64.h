#pragma once

#include <Arduino.h>

/**
 * Base64 encoding and decoding utilities
 */
class Base64 {
public:
    /**
     * Encode binary data to base64 string
     * @param output Buffer to store the encoded string
     * @param input Binary input data
     * @param inputLength Length of the input data
     * @return Length of the encoded data
     */
    static size_t encode(uint8_t* output, const uint8_t* input, size_t inputLength);

    /**
     * Calculate length of base64 encoded string
     * @param input Binary input data
     * @param inputLength Length of the input data
     * @return Length of the encoded data including null terminator
     */
    static size_t encodedLength(size_t inputLength);

    /**
     * Decode base64 string to binary data
     * @param output Buffer to store the decoded data
     * @param input Base64 encoded input string
     * @param inputLength Length of the input string
     * @return Length of the decoded data
     */
    static size_t decode(char* output, const char* input, size_t inputLength);

    /**
     * Calculate length of decoded base64 data
     * @param input Base64 encoded input string
     * @param inputLength Length of the input string
     * @return Length of the decoded data
     */
    static size_t decodedLength(const char* input, size_t inputLength);
};
