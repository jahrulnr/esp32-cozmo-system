#include "Screen.h"
#include "../Utils/I2CManager.h"

namespace Screen {

Screen::Screen() : _u8g2(nullptr), _initialized(false), _mux(xSemaphoreCreateMutex()) {
}

Screen::~Screen() {
    if (_u8g2) {
        delete _u8g2;
    }

    vSemaphoreDelete(_mux);
}

bool Screen::init(int sda, int scl) {
    _u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
    Utils::I2CManager::getInstance().initBus("base", sda, scl);
    
    _u8g2->begin();
    _u8g2->setFont(u8g2_font_6x10_tf);
    _u8g2->setDrawColor(1);
    _u8g2->setFontRefHeightExtendedText();
    _u8g2->setFontPosTop();
    _u8g2->setFontDirection(0);

    _face = new Face(_u8g2, SCREEN_WIDTH, SCREEN_HEIGHT, 40);
    _face->Expression.GoTo_Normal();

    // Assign a weight to each emotion
    _face->Behavior.SetEmotion(eEmotions::Normal, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Angry, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Sad, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Glee, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Happy, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Worried, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Focused, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Annoyed, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Surprised, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Skeptic, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Frustrated, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Unimpressed, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Sleepy, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Suspicious, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Squint, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Furious, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Scared, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Awe, 1.0);
    _face->Behavior.Timer.SetIntervalMillis(10000);

    _face->Blink.Timer.SetIntervalMillis(5000);
    _face->Look.Timer.SetIntervalMillis(1000);
    
    clear();
    autoFace();
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
    
    _holdFace = true;
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
    
    _holdFace = true;
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

    if (_holdFace) {
        _u8g2->sendBuffer();
        vTaskDelay(pdMS_TO_TICKS(3000));
    } else {
        updateFace();
    }

    _holdFace = false;
}

void Screen::mutexUpdate() {
    if (_lock()) {
        update();
        _unlock();
    }
}

void Screen::updateFace() {
    if (!_initialized || !_u8g2) {
        return;
    }

    if (!_holdFace) {
        _face->Update();
    }
}

void Screen::mutexUpdateFace() {
    if (_lock()) {
        updateFace();
        _unlock();
    }
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

Face *Screen::getFace() {
    return _face;
}

void Screen::autoFace(bool exp) {
  _face->RandomBehavior = 
  _face->RandomBlink = 
  _face->RandomLook = 
    exp;
}

bool Screen::_lock() {
    if (!_initialized || !_u8g2) {
        return false;
    }

    return xSemaphoreTake(_mux, pdMS_TO_TICKS(3000));
}

void Screen::_unlock() {
    xSemaphoreGive(_mux);
}

} // namespace Utils
