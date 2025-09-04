#include "Screen.h"

namespace Screen {

Screen::Screen() : _u8g2(nullptr), _initialized(false), 
    _holdFace(false), _holdTimer(0),
    _micLevel(0), _width(128), _height(64),
    _mux(xSemaphoreCreateMutex()), _face(nullptr) {
}

Screen::~Screen() {
    if (_u8g2) {
        delete _u8g2;
    }

    vSemaphoreDelete(_mux);
}

bool Screen::init(int sda, int scl, int width, int height) {
    _u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
    Utils::I2CManager::getInstance().initBus("base", sda, scl);
    
    _u8g2->begin();
    _u8g2->setFont(u8g2_font_6x10_tf);
    _u8g2->setDrawColor(1);
    _u8g2->setFontRefHeightExtendedText();
    _u8g2->setFontPosTop();
    _u8g2->setFontDirection(0);

    _face = new Face(_u8g2, width, height, 40);
    _face->Expression.GoTo_Normal();

    // Assign a weight to each emotion
    // Normal emotions
    _face->Behavior.SetEmotion(eEmotions::Normal, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Unimpressed, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Focused, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Skeptic, 1.0);

    // Happy emotions
    _face->Behavior.SetEmotion(eEmotions::Happy, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Glee, 1.0);
    _face->Behavior.SetEmotion(eEmotions::Awe, 1.0);

    // Sad emotions
    _face->Behavior.SetEmotion(eEmotions::Sad, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Worried, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Sleepy, 0.2);

    // Other emotions
    _face->Behavior.SetEmotion(eEmotions::Angry, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Annoyed, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Surprised, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Frustrated, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Suspicious, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Squint, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Furious, 0.2);
    _face->Behavior.SetEmotion(eEmotions::Scared, 0.2);
    _face->Behavior.Timer.SetIntervalMillis(10000);

    _face->Blink.Timer.SetIntervalMillis(3000);
    _face->Look.Timer.SetIntervalMillis(1000);
    
    clear();
    autoFace(false);
    _face->RandomBlink = true;
    update();

    _micBar = new MicBar(_u8g2);
    _width = width;
    _height = height;
    _initialized = true;
    return true;
}

void Screen::clear() {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    _u8g2->clearBuffer();
    _u8g2->sendBuffer();
}

void Screen::mutexClear(){
    if (_lock()) {
        clear();
        _unlock();
    }
}

void Screen::drawText(int x, int y, const String& text, const uint8_t* font) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    if (font) {
        _u8g2->setFont(font);
    }
    
    _holdFace = true;
    _holdTimer = 0; // Reset timer to ensure it's initialized in update()
    _u8g2->drawStr(x, y, text.c_str());
}

void Screen::drawCenteredText(int y, const String& text, const uint8_t* font) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    const uint8_t* currentFont = font;
    bool usingCustomFont = (font != nullptr);
    
    int screenWidth = getWidth();
    int textWidth = _u8g2->getStrWidth(text.c_str());
    
    // Check if text fits on screen
    if (textWidth <= screenWidth) {
        // Simple case: text fits, center it
        int x = (screenWidth - textWidth) / 2;
        
    
        _holdFace = true;
        _holdTimer = 0; // Reset timer to ensure it's initialized in update()
        _u8g2->drawStr(x, y, text.c_str());
    } else {
        // Text is too long - handle wrapping
    
        _holdFace = true;
        
        // Try smaller font for long text if using default font
        bool usingDefaultFont = !font;
        if (usingDefaultFont && textWidth > screenWidth * 1.5) {
            // Switch to smaller font
            _u8g2->setFont(u8g2_font_4x6_tf);
        }
        
        // Get font height for line spacing
        int fontHeight = _u8g2->getMaxCharHeight();
        
        // Simple word wrap algorithm
        String remainingText = text;
        int currentY = y;
        int maxLines = 4;  // Prevent too many lines from going off screen
        int lineCount = 0;
        
        while (remainingText.length() > 0 && lineCount < maxLines) {
            int charsToFit = remainingText.length();
            String currentLine = remainingText;
            
            // Find how many characters can fit on one line
            while (_u8g2->getStrWidth(currentLine.c_str()) > screenWidth && charsToFit > 1) {
                charsToFit--;
                currentLine = remainingText.substring(0, charsToFit);
            }
            
            // If we're breaking mid-word, try to find a better break point
            if (charsToFit < remainingText.length() && charsToFit > 10) {
                int lastSpace = currentLine.lastIndexOf(' ');
                if (lastSpace > charsToFit / 2) {
                    charsToFit = lastSpace;
                    currentLine = remainingText.substring(0, charsToFit);
                }
            }
            
            // Draw this line centered
            int lineWidth = _u8g2->getStrWidth(currentLine.c_str());
            int x = (screenWidth - lineWidth) / 2;
            _u8g2->drawStr(x, currentY, currentLine.c_str());
            
            // Move to next line
            remainingText = remainingText.substring(charsToFit);
            if (remainingText.startsWith(" ")) {
                remainingText = remainingText.substring(1);  // Remove leading space
            }
            
            currentY += fontHeight + 2;  // Add some space between lines
            lineCount++;
        }
        
        // If we truncated text, add ellipsis to the last line
        if (remainingText.length() > 0 && lineCount >= maxLines) {
            _u8g2->drawStr((screenWidth - _u8g2->getStrWidth("...")) / 2, currentY, "...");
        }
        
        // Restore original font if we changed it
        if (usingDefaultFont && textWidth > screenWidth * 1.5) {
            _u8g2->setFont(u8g2_font_6x10_tf);  // Default font
        } else if (usingCustomFont) {
            _u8g2->setFont(currentFont);
        }
    }
}

void Screen::drawLine(int x1, int y1, int x2, int y2) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    _u8g2->drawLine(x1, y1, x2, y2);
}

void Screen::drawRect(int x, int y, int width, int height, bool fill) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    if (fill) {
        _u8g2->drawBox(x, y, width, height);
    } else {
        _u8g2->drawFrame(x, y, width, height);
    }
}

void Screen::drawCircle(int x, int y, int radius, bool fill) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    if (fill) {
        _u8g2->drawDisc(x, y, radius);
    } else {
        _u8g2->drawCircle(x, y, radius);
    }
}

void Screen::update() {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }

    if (_holdFace) {
        if (_holdTimer == 0) {
            _holdTimer = millis() + 3000;
        }

        _micBar->drawBar(_micLevel);
        _u8g2->sendBuffer();
    } else {
        updateFace();
    }

    if (_holdFace && millis() > _holdTimer){
        _holdFace = false;
        _holdTimer = 0;
    }
}

// level range 0-4096
void Screen::setMicLevel(int level) {
    _micLevel = level;
}

void Screen::mutexUpdate() {
    if (_lock()) {
        update();
        _unlock();
    }
}

void Screen::updateFace() {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }

    if (!_holdFace) {
        _u8g2->clearBuffer();

        _micBar->drawBar(_micLevel);
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
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    _u8g2->setFont(font);
}

int Screen::getWidth() const {
    if (_initialized == false || _u8g2 == nullptr) {
        return 0;
    }
    
    return _u8g2->getWidth();
}

int Screen::getHeight() const {
    if (_initialized == false || _u8g2 == nullptr) {
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
    if (_initialized == false || _u8g2 == nullptr) {
        return false;
    }

    return xSemaphoreTake(_mux, pdMS_TO_TICKS(3000));
}

void Screen::_unlock() {
    xSemaphoreGive(_mux);
}

} // namespace Utils
