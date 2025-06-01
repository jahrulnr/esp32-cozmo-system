#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

namespace Utils {

/**
 * @brief Custom allocator for ArduinoJson that uses ESP32's external SPI RAM
 * 
 * This allocator allows JsonDocument to be stored in external SPI RAM instead of
 * the limited internal RAM. Useful for large JSON documents.
 * 
 * Usage example:
 * SpiJsonDocument<size> doc;  // Uses external SPI RAM
 * doc["key"] = "value";
 */
struct SpiRamAllocator : ArduinoJson::Allocator {
    /**
     * @brief Allocate memory from SPI RAM
     * @param size Size of memory to allocate
     * @return Pointer to allocated memory
     */
    void* allocate(size_t size) override {
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }

    /**
     * @brief Deallocate memory previously allocated
     * @param pointer Pointer to memory to free
     */
    void deallocate(void* pointer) override {
        heap_caps_free(pointer);
    }

    /**
     * @brief Reallocate memory block to different size
     * @param ptr Pointer to memory to reallocate
     * @param new_size New size for memory block
     * @return Pointer to reallocated memory
     */
    void* reallocate(void* ptr, size_t new_size) override {
        return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
    }

    /**
     * @brief Get singleton instance of the allocator
     * @return SpiRamAllocator* Pointer to the singleton allocator
     */
    static SpiRamAllocator* instance() {
        static SpiRamAllocator instance;
        return &instance;
    }
};

/**
 * @brief A JsonDocument that uses SPI RAM for storage
 * 
 * Template class that creates a JsonDocument using the SpiRamAllocator
 * @tparam desiredCapacity The capacity of the JsonDocument in bytes
 */
class SpiJsonDocument : public ArduinoJson::JsonDocument {
public:
    /**
     * @brief Construct a new SpiJsonDocument
     */
    SpiJsonDocument() : ArduinoJson::JsonDocument(SpiRamAllocator::instance()) {
        // In ArduinoJson 7, the capacity is managed dynamically
    }

    /**
     * @brief Construct from a JsonVariant
     * @param src The JsonVariant to copy
     */
    SpiJsonDocument(const ArduinoJson::JsonVariant& src) 
        : ArduinoJson::JsonDocument(SpiRamAllocator::instance()) {
        this->set(src);
    }
    
    /**
     * @brief Get the capacity of the document (for backward compatibility)
     * @return The capacity in bytes
     */
    size_t capacity() const {
        return this->size();
    }
};

} // namespace Utils
