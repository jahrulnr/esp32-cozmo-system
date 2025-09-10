#include "Weather.h"

namespace Display {

Weather::Weather(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display, int width, int height) 
    : _display(display), _hasData(false), _lastUpdate(0), _width(width), _height(height) {
    // Enable UTF8 support for better text rendering (degree symbol, etc.)
    if (_display) {
        _display->enableUTF8Print();
    }
}

Weather::~Weather() {
    // Destructor
}

void Weather::updateWeatherData(const Services::WeatherService::WeatherData& weatherData) {
    _currentWeather = weatherData;
    _hasData = weatherData.isValid;
    _lastUpdate = millis();
}

void Weather::draw() {
    if (!_hasData || !_display) {
        return;
    }

    // Clear the display area
    _display->clearBuffer();

    // Draw all weather information on a single page
    drawAllWeatherInfo();

    _display->sendBuffer();
}

void Weather::clearData() {
    _hasData = false;
    _currentWeather = Services::WeatherService::WeatherData();
}

void Weather::drawAllWeatherInfo() {
    // Calculate compact layout for 128x64 display
    int iconX = 2;
    int iconY = 16;
    int tempX = iconX + ICON_SIZE + 4;
    int tempY = 14;
    
    // Draw weather icon (small, top-left)
    drawWeatherIcon(iconX, iconY, _currentWeather.condition);
    
    // Draw temperature next to icon
    _display->setFont(u8g2_font_ncenB12_tr);
    String tempStr = String(_currentWeather.temperature) + "°C";
    
    // Check if temperature fits, use smaller font if needed
    int tempWidth = _display->getStrWidth(tempStr.c_str());
    if (tempX + tempWidth > _width - 2) {
        _display->setFont(u8g2_font_6x10_tf);
        tempWidth = _display->getStrWidth(tempStr.c_str());
    }
    _display->drawStr(tempX, tempY, tempStr.c_str());
    
    // Draw humidity on the right side, top row
    _display->setFont(u8g2_font_6x10_tf);
    String humidityStr = String(_currentWeather.humidity) + "%";
    int humidityWidth = _display->getStrWidth(humidityStr.c_str());
    _display->drawStr(_width - humidityWidth - 2, 10, humidityStr.c_str());
    
    // Draw description on second line
    int descY = 26;
    String description = truncateText(_currentWeather.description.toString(), _width - 4);
    _display->setFont(u8g2_font_6x10_tf);
    _display->drawStr(2, descY, description.c_str());
    
    // Draw location on third line
    int locationY = 38;
    String location = _currentWeather.location.toString();
    // Simplify location if too long
    if (_display->getStrWidth(location.c_str()) > _width - 4) {
        int commaPos = location.indexOf(',');
        if (commaPos > 0) {
            location = location.substring(0, commaPos);
        }
        location = truncateText(location, _width - 4);
    }
    _display->drawStr(2, locationY, location.c_str());
    
    // Draw wind info on bottom line if space available
    if (_currentWeather.windSpeed > 0 && _height >= 50) {
        int windY = 50;
        String windStr = "Wind: " + String(_currentWeather.windSpeed) + "km/h";
        if (!_currentWeather.windDirection.isEmpty()) {
            String fullWind = windStr + " " + _currentWeather.windDirection.toString();
            if (_display->getStrWidth(fullWind.c_str()) <= _width - 4) {
                windStr = fullWind;
            }
        }
        windStr = truncateText(windStr, _width - 4);
        _display->setFont(u8g2_font_5x7_tf);
        _display->drawStr(2, windY, windStr.c_str());
    }
}

void Weather::drawWeatherIcon(int x, int y, Services::WeatherService::WeatherCondition condition) {
    // Use smaller, simpler icons that fit the compact layout
    _display->setFont(u8g2_font_unifont_t_symbols);
    
    uint16_t glyph = getWeatherIconGlyph(condition);
    if (glyph != 0) {
        _display->drawGlyph(x, y, glyph);
    }
}

uint16_t Weather::getWeatherIconGlyph(Services::WeatherService::WeatherCondition condition) {
    // Use Unicode weather symbols that fit better in compact space
    switch (condition) {
        case Services::WeatherService::WeatherCondition::CLEAR:
            return 0x2600; // Sun symbol ☀
        case Services::WeatherService::WeatherCondition::PARTLY_CLOUDY:
            return 0x26C5; // Partly cloudy ⛅
        case Services::WeatherService::WeatherCondition::CLOUDY:
        case Services::WeatherService::WeatherCondition::OVERCAST:
            return 0x2601; // Cloud ☁
        case Services::WeatherService::WeatherCondition::LIGHT_RAIN:
        case Services::WeatherService::WeatherCondition::MODERATE_RAIN:
        case Services::WeatherService::WeatherCondition::HEAVY_RAIN:
            return 0x2614; // Rain ☔
        case Services::WeatherService::WeatherCondition::THUNDERSTORM:
            return 0x26C8; // Thunder cloud ⛈
        case Services::WeatherService::WeatherCondition::FOG:
        case Services::WeatherService::WeatherCondition::MIST:
            return 0x1F32B; // Fog 🌫 (or fallback to cloud)
        default:
            return 0x2753; // Question mark ❓
    }
}

void Weather::drawCenteredText(int y, const String& text, const uint8_t* font) {
    if (font != nullptr) {
        _display->setFont(font);
    }
    
    int textWidth = _display->getStrWidth(text.c_str());
    int x = (_width - textWidth) / 2;
    _display->drawStr(x, y, text.c_str());
}

String Weather::truncateText(const String& text, int maxWidth, const uint8_t* font) {
    if (font != nullptr) {
        _display->setFont(font);
    }
    
    String result = text;
    int textWidth = _display->getStrWidth(result.c_str());
    
    // If text fits, return as is
    if (textWidth <= maxWidth) {
        return result;
    }
    
    // Try to truncate with ellipsis
    String ellipsis = "...";
    int ellipsisWidth = _display->getStrWidth(ellipsis.c_str());
    int availableWidth = maxWidth - ellipsisWidth;
    
    // Binary search for best fit
    int left = 0;
    int right = result.length();
    int bestFit = 0;
    
    while (left <= right) {
        int mid = (left + right) / 2;
        String candidate = result.substring(0, mid);
        int candidateWidth = _display->getStrWidth(candidate.c_str());
        
        if (candidateWidth <= availableWidth) {
            bestFit = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (bestFit == 0) {
        // Even single character doesn't fit, return empty
        return "";
    }
    
    return result.substring(0, bestFit) + ellipsis;
}

void Weather::drawScrollingText(int y, const String& text, int maxWidth) {
    // Simplified: just draw text that fits, truncate if needed
    _display->setFont(u8g2_font_6x10_tf);
    String displayText = truncateText(text, maxWidth);
    _display->drawStr(2, y, displayText.c_str());
}

void Weather::drawScrollString(int16_t offset, const String& text, int y, int maxWidth) {
    // Clear the scrolling area
    _display->setDrawColor(0);
    _display->drawBox(0, y - 13, maxWidth, 13);
    _display->setDrawColor(1);
    
    _display->setFont(u8g2_font_8x13_mf);
    
    const char* s = text.c_str();
    size_t len = strlen(s);
    static char buf[64]; // Buffer for visible text portion
    size_t char_offset = 0;
    int dx = 0;
    size_t visible = 0;
    
    if (offset < 0) {
        char_offset = (-offset) / 8;
        dx = offset + char_offset * 8;
        if (char_offset >= (size_t)(maxWidth / 8)) {
            return;
        }
        visible = maxWidth / 8 - char_offset + 1;
        if (visible > len) visible = len;
        strncpy(buf, s, visible);
        buf[visible] = '\0';
        _display->drawStr(char_offset * 8 - dx, y, buf);
    } else {
        char_offset = offset / 8;
        if (char_offset >= len) {
            return; // Nothing visible
        }
        dx = offset - char_offset * 8;
        visible = len - char_offset;
        if (visible > (size_t)(maxWidth / 8 + 1)) {
            visible = maxWidth / 8 + 1;
        }
        strncpy(buf, s + char_offset, visible);
        buf[visible] = '\0';
        _display->drawStr(-dx, y, buf);
    }
}

int Weather::getTextWidth(const String& text, const uint8_t* font) {
    if (font != nullptr) {
        _display->setFont(font);
    }
    
    return _display->getStrWidth(text.c_str());
}

} // namespace Display
