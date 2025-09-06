#include "Weather.h"

namespace Display {

Weather::Weather(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display, int width, int height) 
    : _display(display), _hasData(false), _lastUpdate(0), _width(width), _height(height) {
}

Weather::~Weather() {
    // Destructor
}

void Weather::updateWeatherData(const Communication::WeatherService::WeatherData& weatherData) {
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
    _currentWeather = Communication::WeatherService::WeatherData();
}

void Weather::drawAllWeatherInfo() {
    // Calculate available space
    int iconWidth = ICON_SIZE + 5; // Icon + margin
    int rightSideX = iconWidth;
    int rightSideWidth = _width - rightSideX - 5; // Leave 5px right margin
    
    // Draw weather icon at top left
    drawWeatherIcon(2, 2, _currentWeather.condition);
    
    // Draw temperature prominently at top right
    _display->setFont(u8g2_font_ncenB10_tr);
    String tempStr = String(_currentWeather.temperature) + "C";
    int tempWidth = getTextWidth(tempStr);
    if (tempWidth > rightSideWidth) {
        _display->setFont(u8g2_font_6x10_tf);
        tempWidth = getTextWidth(tempStr);
    }
    _display->drawStr(_width - tempWidth - 2, 12, tempStr.c_str());
    
    // Draw humidity below temperature
    _display->setFont(u8g2_font_6x10_tf);
    String humidityStr = String(_currentWeather.humidity) + "%H";
    int humidityWidth = getTextWidth(humidityStr);
    _display->drawStr(_width - humidityWidth - 2, 23, humidityStr.c_str());
    
    // Calculate space for text content
    int contentStartY = 28;
    int lineHeight = 10;
    int contentWidth = _width - 4; // 2px margins on each side
    
    // Draw weather description
    _display->setFont(u8g2_font_6x10_tf);
    String description = truncateText(_currentWeather.description.toString(), contentWidth);
    drawCenteredText(contentStartY, description);
    
    // Draw location
    String location = _currentWeather.location.toString();
    // Try to show just city name if location is too long
    if (getTextWidth(location) > contentWidth) {
        int commaPos = location.indexOf(',');
        if (commaPos > 0) {
            location = location.substring(0, commaPos);
        }
        location = truncateText(location, contentWidth);
    }
    drawCenteredText(contentStartY + lineHeight, location);
    
    // Draw wind info at bottom
    String windStr = String(_currentWeather.windSpeed) + "km/h";
    if (!_currentWeather.windDirection.isEmpty()) {
        String fullWindStr = windStr + " " + _currentWeather.windDirection.toString();
        if (getTextWidth(fullWindStr) <= contentWidth) {
            windStr = fullWindStr;
        }
    }
    windStr = truncateText(windStr, contentWidth);
    drawCenteredText(contentStartY + lineHeight * 2, windStr);
}

void Weather::drawWeatherIcon(int x, int y, Communication::WeatherService::WeatherCondition condition) {
    _display->setFont(u8g2_font_unifont_t_symbols);
    
    char iconChar = getWeatherIconChar(condition);
    if (iconChar != 0) {
        _display->drawGlyph(x, y + ICON_SIZE, iconChar);
    }
}

char Weather::getWeatherIconChar(Communication::WeatherService::WeatherCondition condition) {
    switch (condition) {
        case Communication::WeatherService::WeatherCondition::CLEAR:
            return 0x2600; // Sun symbol
        case Communication::WeatherService::WeatherCondition::PARTLY_CLOUDY:
            return 0x26C5; // Partly cloudy symbol
        case Communication::WeatherService::WeatherCondition::CLOUDY:
        case Communication::WeatherService::WeatherCondition::OVERCAST:
            return 0x2601; // Cloud symbol
        case Communication::WeatherService::WeatherCondition::LIGHT_RAIN:
        case Communication::WeatherService::WeatherCondition::MODERATE_RAIN:
        case Communication::WeatherService::WeatherCondition::HEAVY_RAIN:
            return 0x2614; // Rain symbol
        case Communication::WeatherService::WeatherCondition::THUNDERSTORM:
            return 0x26C8; // Thunder cloud symbol
        case Communication::WeatherService::WeatherCondition::FOG:
        case Communication::WeatherService::WeatherCondition::MIST:
            return 0x2601; // Cloud symbol (fog)
        default:
            return 0x2753; // Question mark for unknown
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

int Weather::getTextWidth(const String& text, const uint8_t* font) {
    if (font != nullptr) {
        _display->setFont(font);
    }
    
    return _display->getStrWidth(text.c_str());
}

} // namespace Display
