#include "display/Display.h"

namespace Display {

void Display::setFont(const uint8_t* font) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    _u8g2->setFont(font);
}

void Display::drawText(int x, int y, const String& text, const uint8_t* font) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    if (font) {
        _u8g2->setFont(font);
    }
    
    _state = STATE_TEXT;
    _holdTimer = 0; // Reset timer to ensure it's initialized in update()
    _u8g2->drawStr(x, y, text.c_str());
}

void Display::drawCenteredText(int y, const String& text, const uint8_t* font) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }
    
    const uint8_t* currentFont = font;
    bool usingCustomFont = (font != nullptr);
    
    int displayWidth = getWidth();
    int textWidth = _u8g2->getStrWidth(text.c_str());
    
    // Check if text fits on display
    if (textWidth <= displayWidth) {
        // Simple case: text fits, center it
        int x = (displayWidth - textWidth) / 2;
        
    
        _state = STATE_TEXT;
        _holdTimer = 0; // Reset timer to ensure it's initialized in update()
        _u8g2->drawStr(x, y, text.c_str());
    } else {
        // Text is too long - handle wrapping
    
        _state = STATE_TEXT;
        
        // Try smaller font for long text if using default font
        bool usingDefaultFont = !font;
        if (usingDefaultFont && textWidth > displayWidth * 1.5) {
            // Switch to smaller font
            _u8g2->setFont(u8g2_font_4x6_tf);
        }
        
        // Get font height for line spacing
        int fontHeight = _u8g2->getMaxCharHeight();
        
        // Simple word wrap algorithm
        String remainingText = text;
        int currentY = y;
        int maxLines = 4;  // Prevent too many lines from going off display
        int lineCount = 0;
        
        while (remainingText.length() > 0 && lineCount < maxLines) {
            int charsToFit = remainingText.length();
            String currentLine = remainingText;
            
            // Find how many characters can fit on one line
            while (_u8g2->getStrWidth(currentLine.c_str()) > displayWidth && charsToFit > 1) {
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
            int x = (displayWidth - lineWidth) / 2;
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
            _u8g2->drawStr((displayWidth - _u8g2->getStrWidth("...")) / 2, currentY, "...");
        }
        
        // Restore original font if we changed it
        if (usingDefaultFont && textWidth > displayWidth * 1.5) {
            _u8g2->setFont(u8g2_font_6x10_tf);  // Default font
        } else if (usingCustomFont) {
            _u8g2->setFont(currentFont);
        }
    }
}

} // end namespace