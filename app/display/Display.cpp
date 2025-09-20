#include "display/Display.h"
#include "setup/setup.h"

namespace Display {

Display::Display() : _u8g2(nullptr), _initialized(false),
    _state(STATE_FACE), _holdTimer(0),
    _micLevel(0), _width(128), _height(64),
    _mux(nullptr), _face(nullptr), _weather(nullptr), _cube3D(nullptr),
    _spaceGame(nullptr), _battery(nullptr), _useMutex(false) {
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

    if (_battery) {
        delete _battery;
    }

    if (_micStatus) delete _micStatus;
    if (_displayStatus) delete _displayStatus;

    vSemaphoreDelete(_mux);
}

bool Display::init(int sda, int scl, int width, int height) {
    _mux = xSemaphoreCreateMutex();
    _u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
    Utils::I2CManager::getInstance().initBus("base", sda, scl);

    _u8g2->begin();
    _u8g2->setDrawColor(1);
    _u8g2->setFontMode(1);
    _u8g2->setBitmapMode(1);
    _u8g2->setFontRefHeightExtendedText();
    _u8g2->setFontPosTop();
    _u8g2->setFontDirection(0);
    _u8g2->setFont(u8g2_font_6x10_tf);

    _micBar = new MicBar(_u8g2);
    _micStatus = new MicStatus(_u8g2);
    _displayStatus = new DisplayStatus(_u8g2);
    _weather = new Weather(_u8g2, width, height);
    _cube3D = new Cube3D(_u8g2, width, height);

    // Initialize SpaceGame - it will get sensor data via notification system
    _spaceGame = new SpaceGame(_u8g2, nullptr, width, height);
    if (_spaceGame) {
        _spaceGame->init();
        _spaceGame->setAutoFire(true); // Enable auto-fire for demo
    }

    // Initialize Battery component
    _battery = new Battery::BatteryDisplay(_u8g2);
    if (_battery) {
       esp_err_t err = _battery->init();
       if (err != ESP_OK) {
        log_e("failed to initiate battery display status: %s", esp_err_to_name(err));
       }
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

void Display::clearBuffer() {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }

    _u8g2->clearBuffer();
}

void Display::update() {
    if (_initialized == false || _u8g2 == nullptr) {
        return;
    }

    if (_useMutex && _lock() == pdFAIL) return;

    if (_state != STATE_SPACE_GAME && _spaceGame->isGameActive()) {
        _spaceGame->pauseGame();
        notification->send(NOTIFICATION_NOTE, Note::STOP);
    }

    // each state maybe need clearBuffer or not, check each state to makesure.
    switch(_state) {
        case STATE_TEXT:
            if (_holdTimer == 0) {
                _holdTimer = millis() + 3000;
            }

            // dont clear buffer, the buffer text writed at DisplayGraphic.cpp and DisplayText.cpp
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
        case STATE_MIC:
            _u8g2->clearBuffer();

            static int mStatus = 0;
            if (_micLevel > 0 && _micLevel < 128) {
                mStatus = 1;
            }
            else if (_micLevel >= 128) {
                mStatus = 2;
            }

            _micStatus->Draw(mStatus);

            _u8g2->sendBuffer();
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
                    notification->send(NOTIFICATION_NOTE, Note::RANDOM);
                }

                _spaceGame->draw();
            }
            break;
        case STATE_STATUS:
            _u8g2->clearBuffer();

            _displayStatus->Draw();

            _u8g2->sendBuffer();
            break;
        case STATE_BATTERY:
            _battery->update();
            _battery->draw();
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

void Display::updateWeatherData(const Services::WeatherService::WeatherData& weatherData) {
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

Battery::BatteryDisplay* Display::getBattery() {
    return _battery;
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
