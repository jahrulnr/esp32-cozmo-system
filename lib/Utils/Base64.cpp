#include "Base64.h"

// Base64 encoding table
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Base64 decoding table
static const unsigned char base64_decoding_table[] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

size_t Base64::encode(uint8_t* output, const uint8_t* input, size_t inputLength) {
    size_t i = 0, j = 0;
    size_t encLen = 0;
    unsigned char a3[3];
    unsigned char a4[4];

    for (i = 0; i < inputLength; i++) {
        a3[i % 3] = input[i];
        if ((i % 3) == 2) {
            a4[0] = (a3[0] & 0xfc) >> 2;
            a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
            a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
            a4[3] = a3[2] & 0x3f;

            for (j = 0; j < 4; j++) {
                output[encLen++] = base64_chars[a4[j]];
            }
            i = 0;
        }
    }

    if (i != 0) {
        for (j = i; j < 3; j++) {
            a3[j] = '\0';
        }

        a4[0] = (a3[0] & 0xfc) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
        a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
        a4[3] = a3[2] & 0x3f;

        for (j = 0; j < i + 1; j++) {
            output[encLen++] = base64_chars[a4[j]];
        }

        while ((i++ < 3)) {
            output[encLen++] = '=';
        }
    }

    return encLen;
}

size_t Base64::encodedLength(size_t inputLength) {
    return ((inputLength + 2) / 3) * 4;
}

size_t Base64::decode(char* output, const char* input, size_t inputLength) {
    size_t i = 0, j = 0;
    size_t decLen = 0;
    unsigned char a3[3];
    unsigned char a4[4];

    while (inputLength--) {
        if (*input == '=') {
            break;
        }
        unsigned char b = base64_decoding_table[(unsigned char)*input++];
        if (b == 64) {
            continue; // Invalid character, skip it
        }
        
        a4[i++] = b;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                a4[i] = base64_decoding_table[(unsigned char)a4[i]];
            }

            a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
            a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
            a3[2] = ((a4[2] & 0x3) << 6) + a4[3];

            for (i = 0; i < 3; i++) {
                output[decLen++] = a3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            a4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            a4[j] = base64_decoding_table[(unsigned char)a4[j]];
        }

        a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
        a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
        a3[2] = ((a4[2] & 0x3) << 6) + a4[3];

        for (j = 0; j < i - 1; j++) {
            output[decLen++] = a3[j];
        }
    }

    return decLen;
}

size_t Base64::decodedLength(const char* input, size_t inputLength) {
    size_t padding = 0;
    
    if (inputLength == 0) {
        return 0;
    }
    
    // Check for trailing '='
    if (input[inputLength - 1] == '=' && inputLength > 0) padding++;
    if (input[inputLength - 2] == '=' && inputLength > 1) padding++;
    
    return ((inputLength * 3) / 4) - padding;
}
