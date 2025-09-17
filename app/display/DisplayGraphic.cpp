#include "display/Display.h"

namespace Display {

void Display::drawLine(int x1, int y1, int x2, int y2) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }

    _u8g2->drawLine(x1, y1, x2, y2);
}

void Display::drawRect(int x, int y, int width, int height, bool fill) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }

    if (fill) {
        _u8g2->drawBox(x, y, width, height);
    } else {
        _u8g2->drawFrame(x, y, width, height);
    }
}

void Display::drawCircle(int x, int y, int radius, bool fill) {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }

    if (fill) {
        _u8g2->drawDisc(x, y, radius);
    } else {
        _u8g2->drawCircle(x, y, radius);
    }
}

} // end namespace