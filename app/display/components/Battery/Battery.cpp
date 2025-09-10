#include "Battery.h"
#include "setup/setup.h"

namespace Battery {

BatteryDisplay::BatteryDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display) 
    : _display(display), lastUpdate(0), animationDelay(100), animationFrame(0) {
}

BatteryDisplay::~BatteryDisplay() {
    // Nothing to clean up
}

esp_err_t BatteryDisplay::init(int width, int height) {
    
    lastUpdate = millis();
    animationFrame = 0;
		_width = width;
		_height = height;
    
    if (!_display){ 
        return ESP_FAIL;
    }

    return ESP_OK;
}

void BatteryDisplay::update() {
    if (!_display) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= animationDelay) {
        animationFrame++;
        if (animationFrame > 20) {  // Reset animation cycle
            animationFrame = 0;
        }
        lastUpdate = currentTime;
    }
    
    // Update battery data only if battery manager is available
    if (batteryManager) {
        batteryManager->update();
    }
}

void BatteryDisplay::draw() {
    if (!_display || !batteryManager) return;
    
    // Get current battery data
    float voltage = batteryManager->getVoltage();
    int level = batteryManager->getLevel();
    BatteryState state = batteryManager->getState();
    bool charging = batteryManager->isCharging();
    
    _display->clearBuffer();
    _display->setFontMode(1);
    _display->setBitmapMode(1);
    
    // Battery icon (centered)
		const unsigned char* iconData = getBatteryIcon(level, charging);
		if (iconData) {
			int iconX = (_width - 24) / 2;  // Center 24px wide icon
			int iconY = (_height / 2) - 20; // Position above center
			_display->drawXBM(iconX, iconY, 24, 16, iconData);
		}
		
		_display->setFont(u8g2_font_5x7_tr);
		
		// Battery level
		char levelText[16];
		snprintf(levelText, sizeof(levelText), "Level: %d%%", level);
		int levelWidth = strlen(levelText) * 5; // Approximate width for 5x7 font
		_display->drawStr((_width - levelWidth) / 2, (_height / 2) - 2, levelText);
		
		// Voltage
		char voltageText[20];
		snprintf(voltageText, sizeof(voltageText), "Voltage: %.1fv", voltage);
		int voltageWidth = strlen(voltageText) * 5; // Approximate width for 5x7 font
		_display->drawStr((_width - voltageWidth) / 2, (_height / 2) + 10, voltageText);
		
		// State or charging status
		if (charging && (animationFrame / 3) % 2 == 0) {  // Blinking charging
			int chargingWidth = 8 * 5; // "CHARGING" length * font width
			_display->drawStr((_width - chargingWidth) / 2, (_height / 2) + 22, "CHARGING");
		} else {
			const char* stateText = getStateText(state);
			char stateDisplay[20];
			snprintf(stateDisplay, sizeof(stateDisplay), "State: %s", stateText);
			int stateWidth = strlen(stateDisplay) * 5; // Approximate width for 5x7 font
			_display->drawStr((_width - stateWidth) / 2, (_height / 2) + 22, stateDisplay);
		}
    
    _display->sendBuffer();
}

const unsigned char* BatteryDisplay::getBatteryIcon(int level, bool charging) {
    if (charging) {
        return big_icon::battery_charging_bits;
    }
    
    if (level >= 95) {
        return big_icon::battery_full_bits;
    } else if (level >= 80) {
        return big_icon::battery_83_bits;
    } else if (level >= 65) {
        return big_icon::battery_67_bits;
    } else if (level >= 45) {
        return big_icon::battery_50_bits;
    } else if (level >= 25) {
        return big_icon::battery_33_bits;
    } else {
        return big_icon::battery_17_bits;
    }
}

const char* BatteryDisplay::getStateText(BatteryState state) {
    switch (state) {
        case BATTERY_STATE_CRITICAL: return "CRITICAL";
        case BATTERY_STATE_LOW: return "LOW";
        case BATTERY_STATE_MEDIUM: return "MEDIUM";
        case BATTERY_STATE_HIGH: return "HIGH";
        case BATTERY_STATE_FULL: return "FULL";
        default: return "UNKNOWN";
    }
}

void BatteryDisplay::reset() {
    animationFrame = 0;
    lastUpdate = millis();
    if (_display) {
        _display->clearBuffer();
    }
}

bool BatteryDisplay::needsUpdate() const {
    return (millis() - lastUpdate) >= animationDelay;
}

} // namespace Battery
