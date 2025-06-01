#include "Screen.h"

namespace Screen {

Screen::Screen() : _u8g2(nullptr), _initialized(false) {
}

Screen::~Screen() {
    if (_u8g2) {
        delete _u8g2;
    }
}

bool Screen::init(int sda, int scl) {
    _u8g2 = new U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R0, scl, sda, U8X8_PIN_NONE);
    _u8g2->begin();
    _u8g2->setFont(u8g2_font_6x10_tf);
    _u8g2->setDrawColor(1);
    _u8g2->setFontRefHeightExtendedText();
    _u8g2->setFontPosTop();
    _u8g2->setFontDirection(0);
    
    clear();
    update();
    
    _initialized = true;
    return true;
}

void Screen::clear() {
    if (!_initialized || !_u8g2) {
        return;
    }
    
    _u8g2->clearBuffer();
}

void Screen::drawText(int x, int y, const String& text, const uint8_t* font) {
    if (!_initialized || !_u8g2) {
        return;
    }
    
    if (font) {
        _u8g2->setFont(font);
    }
    
    _u8g2->drawStr(x, y, text.c_str());
}

void Screen::drawCenteredText(int y, const String& text, const uint8_t* font) {
    if (!_initialized || !_u8g2) {
        return;
    }
    
    if (font) {
        _u8g2->setFont(font);
    }
    
    int width = _u8g2->getStrWidth(text.c_str());
    int x = (getWidth() - width) / 2;
    
    _u8g2->drawStr(x, y, text.c_str());
}

void Screen::drawLine(int x1, int y1, int x2, int y2) {
    if (!_initialized || !_u8g2) {
        return;
    }
    
    _u8g2->drawLine(x1, y1, x2, y2);
}

void Screen::drawRect(int x, int y, int width, int height, bool fill) {
    if (!_initialized || !_u8g2) {
        return;
    }
    
    if (fill) {
        _u8g2->drawBox(x, y, width, height);
    } else {
        _u8g2->drawFrame(x, y, width, height);
    }
}

void Screen::drawCircle(int x, int y, int radius, bool fill) {
    if (!_initialized || !_u8g2) {
        return;
    }
    
    if (fill) {
        _u8g2->drawDisc(x, y, radius);
    } else {
        _u8g2->drawCircle(x, y, radius);
    }
}

void Screen::update() {
    if (!_initialized || !_u8g2) {
        return;
    }
    
    _u8g2->sendBuffer();
}

void Screen::setFont(const uint8_t* font) {
    if (!_initialized || !_u8g2) {
        return;
    }
    
    _u8g2->setFont(font);
}

int Screen::getWidth() const {
    if (!_initialized || !_u8g2) {
        return 0;
    }
    
    return _u8g2->getWidth();
}

int Screen::getHeight() const {
    if (!_initialized || !_u8g2) {
        return 0;
    }
    
    return _u8g2->getHeight();
}

} // namespace Utils
