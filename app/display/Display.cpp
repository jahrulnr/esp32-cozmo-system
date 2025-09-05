#include "display/Display.h"

namespace Display {

Display::Display() : _u8g2(nullptr), _initialized(false), 
    _holdFace(false), _holdTimer(0),
    _micLevel(0), _width(128), _height(64),
    _mux(nullptr), _face(nullptr), _useMutex(false) {
}

Display::~Display() {
    if (_u8g2) {
        delete _u8g2;
    }

    vSemaphoreDelete(_mux);
}

bool Display::init(int sda, int scl, int width, int height) {
    _mux = xSemaphoreCreateMutex();
    _u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
    Utils::I2CManager::getInstance().initBus("base", sda, scl);
    
    _u8g2->begin();
    _u8g2->setFont(u8g2_font_6x10_tf);
    _u8g2->setDrawColor(1);
    _u8g2->setFontRefHeightExtendedText();
    _u8g2->setFontPosTop();
    _u8g2->setFontDirection(0);

    _micBar = new MicBar(_u8g2);
    _width = width;
    _height = height;

    faceInit();
    update();
    
    _initialized = true;
    return true;
}

void Display::clear() {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    _u8g2->clearBuffer();
    _u8g2->sendBuffer();
}

void Display::update() {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }

    if (_useMutex && _lock() == pdFAIL) return;

    if (_holdFace) {
        if (_holdTimer == 0) {
            _holdTimer = millis() + 3000;
        }

        _micBar->drawBar(_micLevel);
        _u8g2->sendBuffer();
    } else {
        _u8g2->clearBuffer();

        _micBar->drawBar(_micLevel);
        _face->Update();
    }

    if (_holdFace && millis() > _holdTimer){
        _holdFace = false;
        _holdTimer = 0;
    }

    if (_useMutex) _unlock();
}

// level range 0-4096
void Display::setMicLevel(int level) {
    _micLevel = level;
}

int Display::getWidth() const {
    if (_initialized == false || _u8g2 == nullptr) {
        return 0;
    }
    
    return _u8g2->getWidth();
}

int Display::getHeight() const {
    if (_initialized == false || _u8g2 == nullptr) {
        return 0;
    }
    
    return _u8g2->getHeight();
}

bool Display::_lock() {
    if (_initialized == false || _u8g2 == nullptr) {
        return false;
    }

    return xSemaphoreTake(_mux, pdMS_TO_TICKS(3000));
}

void Display::_unlock() {
    xSemaphoreGive(_mux);
}

} // namespace Display
