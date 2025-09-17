#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include "../../Icons.h"
#include "battery_manager.h"

namespace Battery {
    class BatteryDisplay {
    private:
        U8G2_SSD1306_128X64_NONAME_F_HW_I2C* _display;
				int _width, _height;

        // Animation state
        unsigned long lastUpdate;
        unsigned long animationDelay;
        int animationFrame;

        // Helper methods
        const unsigned char* getBatteryIcon(int level, bool charging = false);
        const char* getStateText(BatteryState state);

    public:
        BatteryDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display);
        ~BatteryDisplay();

        esp_err_t init(int width = 128, int height = 64);
        void update();
        void draw();
        void reset();

        // Configuration
        void setAnimationDelay(unsigned long delay) { animationDelay = delay; }

        // State queries
        bool isAnimating() const { return animationFrame > 0; }
        bool needsUpdate() const;
    };
}
