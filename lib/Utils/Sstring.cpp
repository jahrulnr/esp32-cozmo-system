#include "Sstring.h"
#include <string.h>
#include <stdlib.h>

namespace Utils {

Sstring::Sstring() : buffer(nullptr), capacity(0), len(0) {
}

Sstring::~Sstring() {
    if (buffer) {
        heap_caps_free(buffer);
    }
}

void Sstring::setBuffer(char* buf, size_t cap) {
    if (buffer) {
        heap_caps_free(buffer);
    }
    buffer = buf;
    capacity = cap;
}

bool Sstring::ensureCapacity(size_t minCap) {
    // If we already have enough capacity, just return true
    if (minCap <= capacity) {
        return true;
    }
    
    // Calculate new capacity (with growth factor of about 1.5)
    size_t newCap = capacity + (capacity / 2);
    if (newCap < minCap) {
        newCap = minCap;
    }
    
    // Allocate new buffer
    char* newBuf = static_cast<char*>(heap_caps_malloc(newCap + 1, MALLOC_CAP_SPIRAM));
    if (!newBuf) {
        newBuf = static_cast<char*>(heap_caps_malloc(newCap + 1, MALLOC_CAP_INTERNAL));
    }
    if (!newBuf) {
        newBuf = static_cast<char*>(heap_caps_malloc(newCap + 1, MALLOC_CAP_DEFAULT));
    }
    
    // Copy existing content if any
    if (buffer && len > 0) {
        memcpy(newBuf, buffer, len);
    }
    
    // Add null terminator
    newBuf[len] = '\0';
    
    // Set new buffer and capacity
    setBuffer(newBuf, newCap);
    return true;
}

Sstring::Sstring(const Sstring& other) : buffer(nullptr), capacity(0), len(0) {
    if (other.len > 0) {
        if (ensureCapacity(other.len)) {
            memcpy(buffer, other.buffer, other.len);
            len = other.len;
            buffer[len] = '\0';
        }
    }
}

Sstring& Sstring::operator=(const Sstring& other) {
    if (this != &other) {
        clear();
        if (other.len > 0) {
            if (ensureCapacity(other.len)) {
                memcpy(buffer, other.buffer, other.len);
                len = other.len;
                buffer[len] = '\0';
            }
        }
    }
    return *this;
}

Sstring::Sstring(Sstring&& other) noexcept : buffer(other.buffer), capacity(other.capacity), len(other.len) {
    other.buffer = nullptr;
    other.capacity = 0;
    other.len = 0;
}

Sstring& Sstring::operator=(Sstring&& other) noexcept {
    if (this != &other) {
        setBuffer(other.buffer, other.capacity);
        len = other.len;
        other.buffer = nullptr;
        other.capacity = 0;
        other.len = 0;
    }
    return *this;
}

Sstring::Sstring(char value) : buffer(nullptr), capacity(0), len(0) {
    if (ensureCapacity(1)) {
        buffer[0] = value;
        buffer[1] = '\0';
        len = 1;
    }
}

Sstring::Sstring(char* value) : buffer(nullptr), capacity(0), len(0) {
    if (value) {
        size_t valueLen = strlen(value);
        if (valueLen > 0 && ensureCapacity(valueLen)) {
            memcpy(buffer, value, valueLen);
            len = valueLen;
            buffer[len] = '\0';
        }
    }
}

Sstring::Sstring(const char* value) : buffer(nullptr), capacity(0), len(0) {
    if (value) {
        size_t valueLen = strlen(value);
        if (valueLen > 0 && ensureCapacity(valueLen)) {
            memcpy(buffer, value, valueLen);
            len = valueLen;
            buffer[len] = '\0';
        }
    }
}

Sstring::Sstring(String& value) : buffer(nullptr), capacity(0), len(0) {
    size_t valueLen = value.length();
    if (valueLen > 0 && ensureCapacity(valueLen)) {
        memcpy(buffer, value.c_str(), valueLen);
        len = valueLen;
        buffer[len] = '\0';
    }
}

Sstring::Sstring(const String& value) : buffer(nullptr), capacity(0), len(0) {
    size_t valueLen = value.length();
    if (valueLen > 0 && ensureCapacity(valueLen)) {
        memcpy(buffer, value.c_str(), valueLen);
        len = valueLen;
        buffer[len] = '\0';
    }
}

Sstring::Sstring(int value, unsigned char base) : buffer(nullptr), capacity(0), len(0) {
    char buf[34]; // Max 33 chars for base 2 + null terminator
    ltoa(value, buf, base);
    *this = Sstring(buf);
}

Sstring::Sstring(unsigned int value, unsigned char base) : buffer(nullptr), capacity(0), len(0) {
    char buf[34]; // Max 33 chars for base 2 + null terminator
    ultoa(value, buf, base);
    *this = Sstring(buf);
}

Sstring::Sstring(long value, unsigned char base) : buffer(nullptr), capacity(0), len(0) {
    char buf[34]; // Max 33 chars for base 2 + null terminator
    ltoa(value, buf, base);
    *this = Sstring(buf);
}

Sstring::Sstring(unsigned long value, unsigned char base) : buffer(nullptr), capacity(0), len(0) {
    char buf[34]; // Max 33 chars for base 2 + null terminator
    ultoa(value, buf, base);
    *this = Sstring(buf);
}

Sstring::Sstring(float value, unsigned char decimals) : buffer(nullptr), capacity(0), len(0) {
    char buf[33];
    dtostrf(value, (decimals + 2), decimals, buf);
    *this = Sstring(buf);
}

Sstring::Sstring(double value, unsigned char decimals) : buffer(nullptr), capacity(0), len(0) {
    char buf[33];
    dtostrf(value, (decimals + 2), decimals, buf);
    *this = Sstring(buf);
}

void Sstring::clear() {
    if (buffer) {
        buffer[0] = '\0';
    }
    len = 0;
}

bool Sstring::append(const char* str) {
    if (!str) return false;
    
    size_t strLen = strlen(str);
    if (strLen == 0) return true;
    
    size_t newLen = len + strLen;
    if (!ensureCapacity(newLen)) {
        return false;
    }
    
    memcpy(buffer + len, str, strLen);
    len = newLen;
    buffer[len] = '\0';
    return true;
}

bool Sstring::append(char c) {
    if (!ensureCapacity(len + 1)) {
        return false;
    }
    
    buffer[len] = c;
    len++;
    buffer[len] = '\0';
    return true;
}

void Sstring::reserve(size_t minCap) {
    ensureCapacity(minCap);
}

const char* Sstring::c_str() const {
    return buffer ? buffer : "";
}

String Sstring::toString() const {
    return String(c_str());
}

const char* Sstring::toChar() const {
    return c_str();
}

int Sstring::toInt() {
    return buffer ? atoi(buffer) : 0;
}

size_t Sstring::size() const {
    return len;
}

size_t Sstring::length() const {
    return len;
}

bool Sstring::isEmpty() {
    return (len == 0);
}

Sstring Sstring::operator+(const Sstring& rhs) const {
    Sstring result(*this);
    result.append(rhs.c_str());
    return result;
}

Sstring Sstring::operator+(const String& rhs) const {
    Sstring result(*this);
    result.append(rhs.c_str());
    return result;
}

Sstring Sstring::operator+(const char* rhs) const {
    Sstring result(*this);
    result.append(rhs);
    return result;
}

Sstring Sstring::operator+(char rhs) const {
    Sstring result(*this);
    result.append(rhs);
    return result;
}

Sstring& Sstring::operator+=(const Sstring& rhs) {
    append(rhs.c_str());
    return *this;
}

Sstring& Sstring::operator+=(const String& rhs) {
    append(rhs.c_str());
    return *this;
}

Sstring& Sstring::operator+=(const char* rhs) {
    append(rhs);
    return *this;
}

Sstring& Sstring::operator+=(char rhs) {
    append(rhs);
    return *this;
}

bool Sstring::operator==(const Sstring& rhs) const {
    if (len != rhs.len) return false;
    if (len == 0) return true;
    return (strcmp(c_str(), rhs.c_str()) == 0);
}

bool Sstring::operator==(const char* rhs) const {
    if (!rhs) return (len == 0);
    return (strcmp(c_str(), rhs) == 0);
}

bool Sstring::contains(const char* substr) const {
    if (!buffer || !substr) return false;
    return (strstr(buffer, substr) != nullptr);
}

bool Sstring::contains(const Sstring& substr) const {
    return contains(substr.c_str());
}

bool Sstring::equals(const char* other) const {
    return operator==(other);
}

bool Sstring::equals(const Sstring& other) const {
    return operator==(other);
}

bool Sstring::startsWith(const char* prefix) const {
    if (!buffer || !prefix) return false;
    size_t prefixLen = strlen(prefix);
    if (len < prefixLen) return false;
    return (strncmp(buffer, prefix, prefixLen) == 0);
}

bool Sstring::startsWith(const Sstring& prefix) const {
    return startsWith(prefix.c_str());
}

int Sstring::indexOf(const char* substr, size_t startPos) const {
    if (!buffer || !substr || startPos >= len) return -1;
    const char* found = strstr(buffer + startPos, substr);
    if (!found) return -1;
    return (found - buffer);
}

int Sstring::indexOf(const Sstring& substr, size_t startPos) const {
    return indexOf(substr.c_str(), startPos);
}

int Sstring::indexOf(char ch, size_t startPos) const {
    if (!buffer || startPos >= len) return -1;
    const char* found = strchr(buffer + startPos, ch);
    if (!found) return -1;
    return (found - buffer);
}

void Sstring::replace(Sstring src, Sstring dest) {
    if (len == 0 || src.len == 0) return;

    const char* srcStr = src.c_str();
    const char* destStr = dest.c_str();
    size_t srcLen = src.len;
    size_t destLen = dest.len;

    // Find the first occurrence
    const char* found = strstr(buffer, srcStr);
    if (!found) return;

    // Create a new buffer for the result
    size_t resultLen = len + 100; // Start with some extra space
    char* result = static_cast<char*>(heap_caps_malloc(resultLen + 1, MALLOC_CAP_SPIRAM));
    if (!result) return;

    size_t pos = 0;
    size_t copiedPos = 0;

    while (found) {
        size_t foundPos = found - buffer;
        size_t needLen = copiedPos + (foundPos - pos) + destLen;

        // Ensure we have enough space
        if (needLen > resultLen) {
            resultLen = needLen + 100; // Add some extra space
            char* newResult = static_cast<char*>(heap_caps_realloc(result, resultLen + 1, MALLOC_CAP_SPIRAM));
            if (!newResult) {
                heap_caps_free(result);
                return;
            }
            result = newResult;
        }

        // Copy part before the found substring
        if (foundPos > pos) {
            memcpy(result + copiedPos, buffer + pos, foundPos - pos);
            copiedPos += (foundPos - pos);
        }

        // Copy the replacement
        memcpy(result + copiedPos, destStr, destLen);
        copiedPos += destLen;

        // Move past this occurrence
        pos = foundPos + srcLen;

        // Look for the next occurrence
        found = strstr(buffer + pos, srcStr);
    }

    // Copy the rest of the string
    if (pos < len) {
        size_t restLen = len - pos;
        size_t needLen = copiedPos + restLen;

        // Ensure we have enough space for the rest
        if (needLen > resultLen) {
            resultLen = needLen;
            char* newResult = static_cast<char*>(heap_caps_realloc(result, resultLen + 1, MALLOC_CAP_SPIRAM));
            if (!newResult) {
                heap_caps_free(result);
                return;
            }
            result = newResult;
        }

        memcpy(result + copiedPos, buffer + pos, restLen);
        copiedPos += restLen;
    }

    // Null terminate the result
    result[copiedPos] = '\0';

    // Replace our buffer with the new one
    setBuffer(result, resultLen);
    len = copiedPos;
}

Sstring Sstring::substring(size_t start, size_t count) const {
    if (!buffer || start >= len) {
        return Sstring();
    }

    // Adjust count if necessary
    if (count > len - start || count == SIZE_MAX) {
        count = len - start;
    }

    // Create a new buffer for the substring
    char* newBuf = static_cast<char*>(heap_caps_malloc(count + 1, MALLOC_CAP_SPIRAM));
    if (!newBuf) {
        return Sstring();
    }

    // Copy the substring
    memcpy(newBuf, buffer + start, count);
    newBuf[count] = '\0';

    // Create a new Sstring with the substring
    Sstring result;
    result.setBuffer(newBuf, count);
    result.len = count;
    return result;
}

Sstring Sstring::trim() const {
    if (!buffer || len == 0) {
        return Sstring();
    }

    // Find the first non-whitespace character
    size_t start = 0;
    while (start < len && isspace(buffer[start])) {
        start++;
    }

    // If the string is all whitespace, return an empty string
    if (start == len) {
        return Sstring();
    }

    // Find the last non-whitespace character
    size_t end = len - 1;
    while (end > start && isspace(buffer[end])) {
        end--;
    }

    // Return the trimmed substring
    return substring(start, end - start + 1);
}

float Sstring::toFloat() const {
    if (!buffer) return 0.0f;
    return atof(buffer);
}

} // namespace Utils
