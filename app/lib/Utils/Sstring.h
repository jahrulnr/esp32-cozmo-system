#pragma once

#include <Arduino.h>
#include <esp_heap_caps.h>

namespace Utils {

/**
 * @brief A string class that uses ESP32's external SPI RAM
 * 
 * This class provides string functionality similar to Arduino's String class
 * but allocates memory from external SPI RAM, preserving internal memory.
 */
class Sstring {
private:
    char* buffer;
    size_t capacity;
    size_t len;

    /**
     * @brief Set buffer pointer and capacity
     * @param buf Buffer pointer
     * @param cap Capacity
     */
    void setBuffer(char* buf, size_t cap);

    /**
     * @brief Ensure buffer has sufficient capacity
     * @param minCap Minimum required capacity
     * @return true if sufficient capacity available or allocated
     */
    bool ensureCapacity(size_t minCap);

public:
    /**
     * @brief Construct an empty string
     */
    Sstring();

    /**
     * @brief Copy constructor
     * @param other String to copy
     */
    Sstring(const Sstring& other);

    /**
     * @brief Assignment operator
     * @param other String to assign
     * @return Reference to this string
     */
    Sstring& operator=(const Sstring& other);

    /**
     * @brief Move constructor
     * @param other String to move
     */
    Sstring(Sstring&& other) noexcept;

    /**
     * @brief Move assignment operator
     * @param other String to move-assign
     * @return Reference to this string
     */
    Sstring& operator=(Sstring&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~Sstring();

    // Constructors for various types
    
    /**
     * @brief Construct from a character
     * @param value Character value
     */
    Sstring(char value);

    /**
     * @brief Construct from a non-const char pointer
     * @param value Char pointer
     */
    Sstring(char* value);

    /**
     * @brief Construct from a const char pointer
     * @param value Const char pointer
     */
    Sstring(const char* value);

    /**
     * @brief Construct from an Arduino String reference
     * @param value Arduino String
     */
    Sstring(String& value);

    /**
     * @brief Construct from a const Arduino String reference
     * @param value Const Arduino String
     */
    Sstring(const String& value);

    /**
     * @brief Construct from an integer
     * @param value Integer value
     * @param base Number base (default: 10)
     */
    explicit Sstring(int value, unsigned char base = 10);

    /**
     * @brief Construct from an unsigned integer
     * @param value Unsigned integer value
     * @param base Number base (default: 10)
     */
    explicit Sstring(unsigned int value, unsigned char base = 10);

    /**
     * @brief Construct from a long integer
     * @param value Long integer value
     * @param base Number base (default: 10)
     */
    explicit Sstring(long value, unsigned char base = 10);

    /**
     * @brief Construct from an unsigned long integer
     * @param value Unsigned long integer value
     * @param base Number base (default: 10)
     */
    explicit Sstring(unsigned long value, unsigned char base = 10);

    /**
     * @brief Construct from a float
     * @param value Float value
     * @param decimals Number of decimal places (default: 2)
     */
    explicit Sstring(float value, unsigned char decimals = 2);

    /**
     * @brief Construct from a double
     * @param value Double value
     * @param decimals Number of decimal places (default: 2)
     */
    explicit Sstring(double value, unsigned char decimals = 2);

    /**
     * @brief Clear the string
     */
    void clear();

    /**
     * @brief Append a C-string
     * @param str String to append
     * @return true if successful
     */
    bool append(const char* str);

    /**
     * @brief Append a character
     * @param c Character to append
     * @return true if successful
     */
    bool append(char c);

    /**
     * @brief Reserve memory for string
     * @param minCap Minimum capacity to reserve
     */
    void reserve(size_t minCap);

    /**
     * @brief Get C-string representation
     * @return Const pointer to null-terminated string
     */
    const char* c_str() const;

    /**
     * @brief Convert to Arduino String
     * @return Arduino String
     */
    String toString() const;

    /**
     * @brief Alias for c_str()
     * @return Const pointer to null-terminated string
     */
    const char* toChar() const;

    /**
     * @brief Convert to integer
     * @return Integer value
     */
    int toInt();

    /**
     * @brief Get string length
     * @return String length
     */
    size_t size() const;

    /**
     * @brief Alias for size()
     * @return String length
     */
    size_t length() const;

    /**
     * @brief Check if string is empty
     * @return true if empty
     */
    bool isEmpty();

    // Operator overloads for concatenation
    
    /**
     * @brief Concatenate with another Sstring
     * @param rhs String to add
     * @return New concatenated string
     */
    Sstring operator+(const Sstring& rhs) const;

    /**
     * @brief Concatenate with Arduino String
     * @param rhs String to add
     * @return New concatenated string
     */
    Sstring operator+(const String& rhs) const;

    /**
     * @brief Concatenate with C-string
     * @param rhs String to add
     * @return New concatenated string
     */
    Sstring operator+(const char* rhs) const;

    /**
     * @brief Concatenate with character
     * @param rhs Character to add
     * @return New concatenated string
     */
    Sstring operator+(char rhs) const;

    /**
     * @brief Append another Sstring
     * @param rhs String to append
     * @return Reference to this string
     */
    Sstring& operator+=(const Sstring& rhs);

    /**
     * @brief Append Arduino String
     * @param rhs String to append
     * @return Reference to this string
     */
    Sstring& operator+=(const String& rhs);

    /**
     * @brief Append C-string
     * @param rhs String to append
     * @return Reference to this string
     */
    Sstring& operator+=(const char* rhs);

    /**
     * @brief Append character
     * @param rhs Character to append
     * @return Reference to this string
     */
    Sstring& operator+=(char rhs);

    // Comparison operators
    
    /**
     * @brief Compare with another Sstring
     * @param rhs String to compare
     * @return true if equal
     */
    bool operator==(const Sstring& rhs) const;

    /**
     * @brief Compare with C-string
     * @param rhs String to compare
     * @return true if equal
     */
    bool operator==(const char* rhs) const;

    /**
     * @brief Check if string contains substring
     * @param substr Substring to check
     * @return true if contains
     */
    bool contains(const char* substr) const;

    /**
     * @brief Check if string contains substring
     * @param substr Substring to check
     * @return true if contains
     */
    bool contains(const Sstring& substr) const;

    /**
     * @brief Compare with another string
     * @param other String to compare
     * @return true if equal
     */
    bool equals(const char* other) const;

    /**
     * @brief Compare with another string
     * @param other String to compare
     * @return true if equal
     */
    bool equals(const Sstring& other) const;

    /**
     * @brief Check if string starts with prefix
     * @param prefix Prefix to check
     * @return true if starts with prefix
     */
    bool startsWith(const char* prefix) const;

    /**
     * @brief Check if string starts with prefix
     * @param prefix Prefix to check
     * @return true if starts with prefix
     */
    bool startsWith(const Sstring& prefix) const;

    /**
     * @brief Find position of substring
     * @param substr Substring to find
     * @param startPos Start position for search
     * @return Position or -1 if not found
     */
    int indexOf(const char* substr, size_t startPos = 0) const;

    /**
     * @brief Find position of substring
     * @param substr Substring to find
     * @param startPos Start position for search
     * @return Position or -1 if not found
     */
    int indexOf(const Sstring& substr, size_t startPos = 0) const;

    /**
     * @brief Find position of character
     * @param ch Character to find
     * @param startPos Start position for search
     * @return Position or -1 if not found
     */
    int indexOf(char ch, size_t startPos = 0) const;

    /**
     * @brief Replace occurrences of src with dest
     * @param src String to replace
     * @param dest Replacement string
     */
    void replace(Sstring src, Sstring dest);

    /**
     * @brief Extract a substring
     * @param start Start position
     * @param count Length of substring (default: remainder)
     * @return New string containing substring
     */
    Sstring substring(size_t start, size_t count = SIZE_MAX) const;

    /**
     * @brief Trim whitespace from both ends
     * @return New trimmed string
     */
    Sstring trim() const;

    /**
     * @brief Convert to float
     * @return Float value
     */
    float toFloat() const;
};

} // namespace Utils
