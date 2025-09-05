#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "I2CManager.h"
#include "./components/Face/Face.h"
#include "./components/Bar/Bar.h"
#include "./components/Mochi/Mochi.h"

namespace Display {

class Display {
public:
    Display();
    ~Display();

    /**
     * Initialize display
     * @param sda The SDA pin for I2C communication
     * @param scl The SCL pin for I2C communication
     * @return true if initialization was successful, false otherwise
     */
    bool init(int sda = SDA, int scl = SCL, int width = 128, int height = 64);

    /**
     * Clear the display
     */
    void clear();

    /**
     * Draw text at a specific position
     * @param x The x-coordinate
     * @param y The y-coordinate
     * @param text The text to draw
     * @param font The font to use (optional)
     */
    void drawText(int x, int y, const String& text, const uint8_t* font = nullptr);

    /**
     * Draw a centered text
     * @param y The y-coordinate
     * @param text The text to draw
     * @param font The font to use (optional)
     */
    void drawCenteredText(int y, const String& text, const uint8_t* font = nullptr);

    /**
     * Draw a line
     * @param x1 The starting x-coordinate
     * @param y1 The starting y-coordinate
     * @param x2 The ending x-coordinate
     * @param y2 The ending y-coordinate
     */
    void drawLine(int x1, int y1, int x2, int y2);

    /**
     * Draw a rectangle
     * @param x The x-coordinate of the top-left corner
     * @param y The y-coordinate of the top-left corner
     * @param width The width of the rectangle
     * @param height The height of the rectangle
     * @param fill Whether to fill the rectangle
     */
    void drawRect(int x, int y, int width, int height, bool fill = false);

    /**
     * Draw a circle
     * @param x The x-coordinate of the center
     * @param y The y-coordinate of the center
     * @param radius The radius of the circle
     * @param fill Whether to fill the circle
     */
    void drawCircle(int x, int y, int radius, bool fill = false);

    void setMicLevel(int level = 0);

    /**
     * Update the display (call this after drawing operations)
     */
    void update();
    void enableMutex(bool enable = true) { _useMutex = enable; }

    /**
     * Set the font
     * @param font The font to use
     */
    void setFont(const uint8_t* font);

    /**
     * Get the width of the display
     * @return The width in pixels
     */
    int getWidth() const;

    /**
     * Get the height of the display
     * @return The height in pixels
     */
    int getHeight() const;

    Face* getFace();
    void autoFace(bool exp = true);

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* _u8g2;
    bool _initialized;
    SemaphoreHandle_t _mux;
    MicBar *_micBar;
    int _micLevel;
    int _width;
    int _height;
    bool _holdFace;
    long _holdTimer;
    bool _useMutex;

    void faceInit();
    Face *_face;

    bool _lock();
    void _unlock();
};

} // namespace Display

#endif