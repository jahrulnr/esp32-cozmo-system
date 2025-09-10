#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "services/WeatherService.h"

namespace Display {

class Weather {
private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* _display;
    Services::WeatherService::WeatherData _currentWeather;
    bool _hasData;
    unsigned long _lastUpdate;
    int _width;
    int _height;
    
    // Display positioning constants
    static const int ICON_SIZE = 16;  // Compact size to fit display
    static const int SCROLL_SPEED = 2;
    static const int SCROLL_DELAY = 20;

public:
    Weather(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display, int width = 128, int height = 64);
    ~Weather();

    /**
     * Update weather data to display
     * @param weatherData The weather data to display
     */
    void updateWeatherData(const Services::WeatherService::WeatherData& weatherData);

    /**
     * Draw the weather display (called during display update cycle)
     */
    void draw();

    /**
     * Check if weather data is available
     * @return true if weather data is available
     */
    bool hasData() const { return _hasData; }

    /**
     * Clear weather data
     */
    void clearData();

private:
    /**
     * Draw weather icon based on condition using better icon font
     * @param x X position
     * @param y Y position
     * @param condition Weather condition
     */
    void drawWeatherIcon(int x, int y, Services::WeatherService::WeatherCondition condition);

    /**
     * Draw all weather information on single page
     */
    void drawAllWeatherInfo();

    /**
     * Get weather icon glyph code for open-iconic font
     * @param condition Weather condition
     * @return Icon glyph code
     */
    uint16_t getWeatherIconGlyph(Services::WeatherService::WeatherCondition condition);

    /**
     * Draw scrolling text for long strings
     * @param y Y position for text
     * @param text Text to scroll
     * @param maxWidth Maximum width available
     */
    void drawScrollingText(int y, const String& text, int maxWidth);

    /**
     * Draw a portion of scrolling text at given offset
     * @param offset Pixel offset for scrolling
     * @param text Text to draw
     * @param y Y position
     * @param maxWidth Maximum width
     */
    void drawScrollString(int16_t offset, const String& text, int y, int maxWidth);

    /**
     * Draw centered text with proper sizing
     * @param y Y position
     * @param text Text to draw
     * @param font Font to use (optional)
     */
    void drawCenteredText(int y, const String& text, const uint8_t* font = nullptr);
    
    /**
     * Truncate text to fit within specified width
     * @param text Original text
     * @param maxWidth Maximum width in pixels
     * @param font Font to use for measurement
     * @return Truncated text
     */
    String truncateText(const String& text, int maxWidth, const uint8_t* font = nullptr);
    
    /**
     * Get text width for given font
     * @param text Text to measure
     * @param font Font to use (optional)
     * @return Text width in pixels
     */
    int getTextWidth(const String& text, const uint8_t* font = nullptr);
};

} // namespace Display

#endif // WEATHER_H
