#include "display/Display.h"

namespace Display {

Display::Display() : _u8g2(nullptr), _initialized(false), 
    _state(STATE_FACE), _holdTimer(0),
    _micLevel(0), _width(128), _height(64),
    _mux(nullptr), _face(nullptr), _weather(nullptr), _cube3D(nullptr), _spaceGame(nullptr), _useMutex(false) {
}

Display::~Display() {
    if (_u8g2) {
        delete _u8g2;
    }

    if (_weather) {
        delete _weather;
    }

    if (_cube3D) {
        delete _cube3D;
    }

    if (_spaceGame) {
        delete _spaceGame;
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
    _weather = new Weather(_u8g2, width, height);
    _cube3D = new Cube3D(_u8g2, width, height);
    
    // Initialize SpaceGame - it will get sensor data via notification system
    _spaceGame = new SpaceGame(_u8g2, nullptr, width, height);
    if (_spaceGame) {
        _spaceGame->init();
        _spaceGame->setAutoFire(true); // Enable auto-fire for demo
    }
    
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

    if (_state != STATE_SPACE_GAME && _spaceGame->isGameActive()) {
        _spaceGame->pauseGame();
    }

    switch(_state) {
        case STATE_TEXT:
            if (_holdTimer == 0) {
                _holdTimer = millis() + 3000;
            }

            _micBar->drawBar(_micLevel);
            _u8g2->sendBuffer();

            if (millis() > _holdTimer){
                _state = STATE_FACE;
                _holdTimer = 0;
            }
            break;
        case STATE_FACE:
            _u8g2->clearBuffer();

            _micBar->drawBar(_micLevel);
            _face->Update();
            break;
        case STATE_MOCHI:
            drawMochiFrame(_u8g2);
            _state = STATE_FACE;
            break;
        case STATE_WEATHER:
            _weather->draw();
            break;
        case STATE_ORIENTATION:
            _cube3D->draw();
            break;
        case STATE_SPACE_GAME:
            if (_spaceGame) {
                if (!_spaceGame->isGameActive()) {
                    _spaceGame->startGame();
                }
                
                _spaceGame->draw();
            }
            break;
        default:
            _state = STATE_FACE;
            _u8g2->clearBuffer();
            _u8g2->sendBuffer();
    }

    if (_useMutex) _unlock();
}

// level range 0-4096
void Display::setMicLevel(int level) {
    _micLevel = level;
}

void Display::updateWeatherData(const Communication::WeatherService::WeatherData& weatherData) {
    if (_weather) {
        _weather->updateWeatherData(weatherData);
    }
}

void Display::updateOrientation(Sensors::OrientationSensor* orientation) {
    if (_cube3D && orientation) {
        _cube3D->updateRotation(orientation);
    }
    
    // Also update SpaceGame with gyro input if it's the active game
    if (_spaceGame && orientation && _state == STATE_SPACE_GAME) {
        _spaceGame->updateGyroInput(orientation);
    }
}

SpaceGame* Display::getSpaceGame() {
    return _spaceGame;
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
