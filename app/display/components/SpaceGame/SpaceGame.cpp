#include "SpaceGame.h"
#include <cstdlib>

namespace Display {

SpaceGame::SpaceGame(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display, void* unused, int width, int height)
    : _display(display), _width(width), _height(height),
      _gameActive(false), _gameState(STATE_GAME), _playerPos(AREA_HEIGHT/2), 
      _playerPoints(0), _playerPointsDelayed(0), _highScore(0), _difficulty(1), _toDiffCnt(0),
      _gyroSensitivity(1.f), _playerAccel(0.0f), _isFiring(false), _autoFire(true),
    _firePlayer(0), _firePeriod(25), _manualFireDelay(20), _isFireLastValue(0),
    _lastGyroUpdate(0), _centerPosition(AREA_HEIGHT/2.0f), _currentTilt(0.0f), _alpha(1.0f),
      _stationaryTime(0), _gyroThreshold(0.5f),
    _lastGyroX(0), _lastGyroY(0), _lastGyroZ(0), _lastAccelX(0), _lastAccelY(0), _lastAccelZ(0) {
    
    // Initialize random seed
    srand(millis());
    
    // Clear game objects
    clearObjects();
}

SpaceGame::~SpaceGame() {
    // Destructor
}

bool SpaceGame::init() {
    if (!_display) {
        return false;
    }
    
    // Enable UTF8 support for better rendering
    _display->enableUTF8Print();
    _display->setBitmapMode(1);
    
    return true;
}

void SpaceGame::startGame() {
    _gameActive = true;
    _gameState = STATE_GAME; // Start playing immediately
    setupInGame(); // Initialize the game objects right away
    _lastGyroUpdate = 0; // Reset gyro timing
}

void SpaceGame::pauseGame() {
    _gameActive = false;
}

void SpaceGame::setFireControl(bool isFiring) {
    _isFiring = isFiring;
}

void SpaceGame::setAutoFire(bool autoFire) {
    _autoFire = autoFire;
}

void SpaceGame::setGyroSensitivity(float sensitivity) {
    _gyroSensitivity = sensitivity;
}

void SpaceGame::updateGyroInput(Sensors::OrientationSensor* orientation) {
    if (!orientation) {
        return; // Just return, don't disable anything
    }
    
    unsigned long currentTime = millis();
    
    // Initialize timing on first call
    if (_lastGyroUpdate == 0) {
        _lastGyroUpdate = currentTime;
        return;
    }
    
    // Calculate time delta in seconds
    float deltaTime = (currentTime - _lastGyroUpdate) / 1000.0f;
    _lastGyroUpdate = currentTime;
    
    // Skip if time delta is too large (probably first call or long pause)
    if (deltaTime > 0.1f) {
        return;
    }
    
    // Get gyroscope and accelerometer data
    // For vertical game movement, we want roll (left/right tilt) - easier to play
    float gyroZ = orientation->getX();  // Roll rate (left/right tilt) - swapped like Cube3D
    float accelX = -orientation->getAccelY(); // For roll calculation reference
    float accelY = orientation->getAccelZ();  // For roll calculation 
    float accelZ = orientation->getAccelX();  // For roll calculation - swapped like Cube3D
    
    // Calculate tilt angle from accelerometer (absolute reference like Cube3D)
    float accelRoll = atan2(accelZ, accelY); // Roll: rotation around Z-axis (left/right tilt)
    
    // Integrate gyroscope data
    float gyroRollDelta = gyroZ * deltaTime * PI / 180.0f;
    
    // Pure gyro integration for tilt (no blending with accel, no auto-centering)
    _currentTilt = _currentTilt + gyroRollDelta;
    
    // No drift correction needed for roll - accelerometer provides absolute reference
    // (Unlike yaw in Cube3D, roll has gravity reference so it's naturally stable)
    // ...no stationary/slowdown logic...
    
    // Wrap angles to prevent overflow (like Cube3D)
    while (_currentTilt > PI) _currentTilt -= 2 * PI;
    while (_currentTilt < -PI) _currentTilt += 2 * PI;
    
    // Convert tilt angle to screen position
    // Map tilt range to screen range with smooth scaling  
    float maxTiltRange = PI / 6.0f; // 30 degrees max tilt range
    // Invert the tilt for intuitive control: tilt left = move up, tilt right = move down
    float normalizedTilt = (-_currentTilt / maxTiltRange) * _gyroSensitivity;
    
    // Add deadzone to prevent small movements when nearly level
    if (abs(normalizedTilt) < 0.05f) {
        normalizedTilt = 0.0f;
    }
    
    // Clamp to reasonable range
    if (normalizedTilt > 1.0f) normalizedTilt = 1.0f;
    if (normalizedTilt < -1.0f) normalizedTilt = -1.0f;
    
    // Calculate position relative to center with margin
    float maxDeviation = (AREA_HEIGHT / 2.0f) - 2; // Leave 2 pixels margin
    float targetPos = _centerPosition + (normalizedTilt * maxDeviation);
    
    // Ensure we stay within bounds
    if (targetPos < 1) targetPos = 1;
    if (targetPos > AREA_HEIGHT - 2) targetPos = AREA_HEIGHT - 2;
    
    _playerPos = (uint8_t)targetPos;
}

void SpaceGame::draw() {
    if (!_display) return;
    
    _display->clearBuffer();
    
    // Simplified - only draw the game
    if (_gameState == STATE_GAME) {
				stepInGame();
        drawGameObjects();
        drawGameUI();
    }
    
    _display->sendBuffer();
}

void SpaceGame::setupInGame() {
    _playerPoints = 0;
    _playerPointsDelayed = 0;
    _difficulty = 1;
    _toDiffCnt = 0;
    _playerAccel = 0.0f;
    clearObjects();
    
    // Create player
    int8_t playerIdx = findEmptyObject();
    if (playerIdx >= 0) {
        GameObject* player = &_objects[playerIdx];
        player->ot = OT_PLAYER;
        player->x = 6 << FIXED_POINT;
        player->y = (AREA_HEIGHT/2) << FIXED_POINT;
        player->x0 = -6;
        player->x1 = 0;
        player->y0 = -2;
        player->y1 = 2;
    }
}

void SpaceGame::stepInGame() {
    updatePlayerPosition();
    updateFireControl();
    moveObjects();
    handleCollisions();
    generateNewObjects();
    
    // Update difficulty
    _toDiffCnt++;
    if (_toDiffCnt == (DIFF_VIS_LEN << DIFF_FP)) {
        _toDiffCnt = 0;
        _difficulty++;
        _playerPoints += POINTS_PER_LEVEL;
    }
    
    // Update delayed points display
    if (_playerPointsDelayed < _playerPoints) {
        _playerPointsDelayed++;
    }
}

void SpaceGame::updatePlayerPosition() {
    // Gyro control is always active - no fallback needed
    // Position is updated in updateGyroInput()
}

void SpaceGame::updateFireControl() {
    // Auto-fire for now - can be controlled via voice/touch later
    if (_autoFire) {
        _firePlayer++;
        if (_firePlayer >= _firePeriod) {
            _firePlayer = 0;
        }
    } else {
        // Manual fire mode - could be triggered by voice commands
        if (_firePlayer < _manualFireDelay) {
            _firePlayer++;
        } else {
            if (_isFireLastValue == 0 && _isFiring) {
                _firePlayer = 0;
            }
        }
        _isFireLastValue = _isFiring;
    }
    
    // Fire missiles if it's time
    if (_firePlayer == 0) {
        // Find player object and create missile
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (_objects[i].ot == OT_PLAYER || _objects[i].ot == OT_PLAYER2 || _objects[i].ot == OT_PLAYER3) {
                uint8_t x = _objects[i].x >> FIXED_POINT;
                uint8_t y = _objects[i].y >> FIXED_POINT;
                
                if (_objects[i].ot == OT_PLAYER) {
                    createPlayerMissile(x, y);
                } else if (_objects[i].ot == OT_PLAYER2) {
                    createPlayerMissile(x, y);
                    createPlayerMissile(x, y + 4);
                } else if (_objects[i].ot == OT_PLAYER3) {
                    createPlayerMissile(x, y);
                    createPlayerMissile(x, y + 4);
                    createPlayerMissile(x, y - 4);
                }
                break;
            }
        }
    }
}

void SpaceGame::moveObjects() {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        GameObject* obj = &_objects[i];
        if (obj->ot == OT_EMPTY) continue;
        
        switch (obj->ot) {
            case OT_PLAYER:
            case OT_PLAYER2:
            case OT_PLAYER3:
                obj->y = _playerPos << FIXED_POINT;
                break;
                
            case OT_MISSILE:
                obj->x += (1 << FIXED_POINT);
                break;
                
            case OT_TRASH1:
            case OT_TRASH2:
            case OT_BIG_TRASH:
                obj->x -= (1 << FIXED_POINT) / 8;
                obj->x -= _difficulty;
                obj->y += (int16_t)obj->tmp;
                if (obj->y >= ((AREA_HEIGHT-1) << FIXED_POINT) || obj->y <= 0) {
                    obj->tmp = -obj->tmp;
                }
                break;
                
            case OT_GADGET:
                obj->x -= (1 << FIXED_POINT) / 2;
                obj->y += (int16_t)obj->tmp;
                if (obj->y >= ((AREA_HEIGHT-1) << FIXED_POINT) || obj->y <= 0) {
                    obj->tmp = -obj->tmp;
                }
                break;
                
            case OT_WALL_SOLID:
                obj->x -= 1;
                obj->x -= (_difficulty >> 1);
                break;
                
            case OT_DUST_PY:
                obj->y += 3 << FIXED_POINT;
                break;
                
            case OT_DUST_NY:
                obj->y -= 3 << FIXED_POINT;
                break;
                
            case OT_DUST_NXPY:
                obj->y += 3 << FIXED_POINT;
                obj->x -= 3 << FIXED_POINT;
                break;
                
            case OT_DUST_NXNY:
                obj->y -= 3 << FIXED_POINT;
                obj->x -= 3 << FIXED_POINT;
                break;
                
            case OT_TRASH_IMPLODE:
            case OT_GADGET_IMPLODE:
                obj->tmp++;
                if ((obj->tmp & 0x03) == 0) {
                    if (obj->x0 != obj->x1) {
                        obj->x0++;
                    } else {
                        destroyObject(i);
                    }
                }
                break;
        }
        
        // Remove objects that are out of bounds
        if (isObjectOutOfBounds(i)) {
            destroyObject(i);
        }
    }
}

void SpaceGame::handleCollisions() {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (_objects[i].ot != OT_MISSILE) continue;
        
        uint8_t missileX = _objects[i].x >> FIXED_POINT;
        uint8_t missileY = _objects[i].y >> FIXED_POINT;
        
        for (int j = 0; j < MAX_OBJECTS; j++) {
            if (i == j || _objects[j].ot == OT_EMPTY) continue;
            
            if (checkCollision(i, j)) {
                // Handle hit
                switch (_objects[j].ot) {
                    case OT_TRASH1:
                    case OT_TRASH2:
                        _playerPoints += 5;
                        _objects[j].ot = OT_TRASH_IMPLODE;
                        _objects[j].tmp = 0;
                        destroyObject(i); // Destroy missile
                        break;
                        
                    case OT_BIG_TRASH:
                        _playerPoints += 10;
                        // Create smaller trash pieces
                        createTrash(missileX - 1, missileY + 3, 2 + (gameRandom() & 3));
                        createTrash(missileX - 2, missileY - 3, -2 - (gameRandom() & 3));
                        destroyObject(j);
                        destroyObject(i);
                        break;
                        
                    case OT_GADGET:
                        _playerPoints += 20;
                        // Upgrade player if possible
                        for (int k = 0; k < MAX_OBJECTS; k++) {
                            if (_objects[k].ot == OT_PLAYER2) {
                                _objects[k].ot = OT_PLAYER3;
                                _objects[k].y0 = -5;
                                _objects[k].y1 = 5;
                                break;
                            } else if (_objects[k].ot == OT_PLAYER) {
                                _objects[k].ot = OT_PLAYER2;
                                _objects[k].y0 = -2;
                                _objects[k].y1 = 5;
                                break;
                            }
                        }
                        _objects[j].ot = OT_GADGET_IMPLODE;
                        _objects[j].tmp = 0;
                        destroyObject(i);
                        break;
                        
                    case OT_WALL_SOLID:
                        _objects[j].x0++;
                        if (_objects[j].x0 >= _objects[j].x1) {
                            _playerPoints += 30;
                            destroyObject(j);
                        }
                        destroyObject(i);
                        break;
                }
            }
        }
        
        // Check if trash/walls hit player
        for (int j = 0; j < MAX_OBJECTS; j++) {
            if (_objects[j].ot != OT_PLAYER && _objects[j].ot != OT_PLAYER2 && _objects[j].ot != OT_PLAYER3) continue;
            
            for (int k = 0; k < MAX_OBJECTS; k++) {
                if (_objects[k].ot == OT_TRASH1 || _objects[k].ot == OT_TRASH2 || 
                    _objects[k].ot == OT_BIG_TRASH || _objects[k].ot == OT_WALL_SOLID) {
                    
                    if (checkCollision(j, k)) {
                        // Game over - player hit, but handle via voice command stop
                        _gameActive = false; // Stop the game
                        return;
                    }
                }
            }
        }
    }
}

void SpaceGame::generateNewObjects() {
    // Generate trash
    uint8_t trashCount = 0;
    uint8_t maxX = 0;
    
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (_objects[i].ot == OT_TRASH1 || _objects[i].ot == OT_TRASH2 || 
            _objects[i].ot == OT_GADGET || _objects[i].ot == OT_BIG_TRASH) {
            trashCount++;
            uint8_t objX = _objects[i].x >> FIXED_POINT;
            if (maxX < objX) maxX = objX;
        }
    }
    
    uint8_t minDistForNew = 20 - _min(_difficulty, 14);
    uint8_t maxL = AREA_WIDTH - minDistForNew;
    
    if (trashCount < MAX_OBJECTS - 7 && maxX < maxL) {
        if (_difficulty >= 3 && (gameRandom() & 7) == 0) {
            createGadget(AREA_WIDTH - 1, gameRandom() % AREA_HEIGHT);
        } else {
            createTrash(AREA_WIDTH - 1, gameRandom() % AREA_HEIGHT, 0);
        }
    }
    
    // Generate walls occasionally
    if (_difficulty >= 2) {
        uint8_t wallCount = 0;
        maxX = 0;
        
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (_objects[i].ot == OT_WALL_SOLID) {
                wallCount++;
                uint8_t objX = _objects[i].x >> FIXED_POINT;
                if (maxX < objX) maxX = objX;
            }
        }
        
        uint8_t wallMinDist = 40 - _min(_difficulty, 30);
        if (maxX < AREA_WIDTH - wallMinDist) {
            createWall();
        }
    }
}

void SpaceGame::drawGameObjects() {
    _display->setDrawColor(1);
    
    for (int i = 0; i < MAX_OBJECTS; i++) {
        GameObject* obj = &_objects[i];
        if (obj->ot == OT_EMPTY) continue;
        
        int16_t x = obj->x >> FIXED_POINT;
        int16_t y = obj->y >> FIXED_POINT;
        
        // Calculate bounding box
        int16_t x0 = x + obj->x0;
        int16_t y0 = y + obj->y0;
        int16_t x1 = x + obj->x1;
        int16_t y1 = y + obj->y1;
        
        // Clip to screen bounds
        if (x0 < 0) x0 = 0;
        if (y0 < 0) y0 = 0;
        if (x1 >= AREA_WIDTH) x1 = AREA_WIDTH - 1;
        if (y1 >= AREA_HEIGHT) y1 = AREA_HEIGHT - 1;
        
        if (x0 >= AREA_WIDTH || y0 >= AREA_HEIGHT || x1 < 0 || y1 < 0) continue;
        
        // Convert Y coordinate (U8g2 has Y=0 at top)
        int16_t drawY0 = AREA_HEIGHT - y1 - 1;
        int16_t drawY1 = AREA_HEIGHT - y0 - 1;
        
        switch (obj->ot) {
            case OT_PLAYER:
            case OT_PLAYER2: 
            case OT_PLAYER3:
                // Draw simple player representation
                _display->drawFrame(x0, drawY0, x1-x0+1, drawY1-drawY0+1);
                _display->drawPixel(x1, drawY0 + (drawY1-drawY0)/2); // nose
                break;
                
            case OT_MISSILE:
                _display->drawPixel(x, AREA_HEIGHT - y - 1);
                break;
                
            case OT_TRASH1:
            case OT_TRASH2:
            case OT_BIG_TRASH:
                _display->drawBox(x0, drawY0, x1-x0+1, drawY1-drawY0+1);
                break;
                
            case OT_GADGET:
                // Draw cross pattern for gadget
                _display->drawPixel(x, AREA_HEIGHT - y - 1);
                _display->drawPixel(x-1, AREA_HEIGHT - y - 1);
                _display->drawPixel(x+1, AREA_HEIGHT - y - 1);
                _display->drawPixel(x, AREA_HEIGHT - y);
                _display->drawPixel(x, AREA_HEIGHT - y - 2);
                break;
                
            case OT_WALL_SOLID:
                _display->drawBox(x0, drawY0, x1-x0+1, drawY1-drawY0+1);
                break;
                
            case OT_DUST_PY:
            case OT_DUST_NY:
            case OT_DUST_NXPY:
            case OT_DUST_NXNY:
                _display->drawPixel(x, AREA_HEIGHT - y - 1);
                break;
                
            case OT_TRASH_IMPLODE:
            case OT_GADGET_IMPLODE:
                // Draw shrinking box
                _display->drawFrame(x0, drawY0, x1-x0+1, drawY1-drawY0+1);
                break;
        }
    }
}

void SpaceGame::drawGameUI() {
    _display->setDrawColor(0);
    _display->drawBox(0, AREA_HEIGHT, _width, _height - AREA_HEIGHT);
    
    _display->setDrawColor(1);
    _display->drawHLine(0, AREA_HEIGHT, AREA_WIDTH);
    _display->drawHLine(0, _height - 1, AREA_WIDTH);
    
    _display->setFont(u8g2_font_4x6_tr);
    
    // Draw difficulty
    _display->drawStr(0, _height - 2, intToString(_difficulty));
    
    // Draw difficulty progress bar
    int16_t progX = 10 + (_toDiffCnt >> DIFF_FP);
    _display->drawHLine(10, _height - 5, DIFF_VIS_LEN);
    _display->drawVLine(10, _height - 6, 3);
    _display->drawVLine(10 + DIFF_VIS_LEN, _height - 6, 3);
    if (progX <= 10 + DIFF_VIS_LEN) {
        _display->drawVLine(progX, _height - 6, 3);
    }
    
    // Draw score
    char* scoreStr = intToString(_playerPointsDelayed);
    int scoreWidth = strlen(scoreStr) * 4;
    _display->drawStr(AREA_WIDTH - scoreWidth - 2, _height - 2, scoreStr);
    
    // ...no debug info drawn...
}

// Utility functions
int8_t SpaceGame::findEmptyObject() {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (_objects[i].ot == OT_EMPTY) {
            return i;
        }
    }
    return -1;
}

void SpaceGame::clearObjects() {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        _objects[i].ot = OT_EMPTY;
    }
}

uint8_t SpaceGame::isObjectOutOfBounds(int8_t objIdx) {
    GameObject* obj = &_objects[objIdx];
    int16_t x = obj->x >> FIXED_POINT;
    int16_t y = obj->y >> FIXED_POINT;
    
    int16_t x0 = x + obj->x0;
    int16_t y0 = y + obj->y0;
    int16_t x1 = x + obj->x1;
    int16_t y1 = y + obj->y1;
    
    if (x0 >= AREA_WIDTH || x1 < 0 || y0 >= AREA_HEIGHT || y1 < 0) {
        return 1;
    }
    return 0;
}

void SpaceGame::destroyObject(int8_t objIdx) {
    _objects[objIdx].ot = OT_EMPTY;
}

bool SpaceGame::checkCollision(int8_t obj1, int8_t obj2) {
    GameObject* o1 = &_objects[obj1];
    GameObject* o2 = &_objects[obj2];
    
    int16_t x1 = o1->x >> FIXED_POINT;
    int16_t y1 = o1->y >> FIXED_POINT;
    int16_t x2 = o2->x >> FIXED_POINT;
    int16_t y2 = o2->y >> FIXED_POINT;
    
    // Simple bounding box collision
    int16_t x1_0 = x1 + o1->x0, x1_1 = x1 + o1->x1;
    int16_t y1_0 = y1 + o1->y0, y1_1 = y1 + o1->y1;
    int16_t x2_0 = x2 + o2->x0, x2_1 = x2 + o2->x1;
    int16_t y2_0 = y2 + o2->y0, y2_1 = y2 + o2->y1;
    
    return !(x1_1 < x2_0 || x1_0 > x2_1 || y1_1 < y2_0 || y1_0 > y2_1);
}

void SpaceGame::createPlayerMissile(uint8_t x, uint8_t y) {
    int8_t objIdx = findEmptyObject();
    if (objIdx < 0) return;
    
    GameObject* obj = &_objects[objIdx];
    obj->ot = OT_MISSILE;
    obj->x = x << FIXED_POINT;
    obj->y = y << FIXED_POINT;
    obj->x0 = -4;
    obj->x1 = 1;
    obj->y0 = 0;
    obj->y1 = 0;
}

void SpaceGame::createTrash(uint8_t x, uint8_t y, int8_t dir) {
    int8_t objIdx = findEmptyObject();
    if (objIdx < 0) return;
    
    GameObject* obj = &_objects[objIdx];
    obj->ot = (gameRandom() & 1) ? OT_TRASH1 : OT_TRASH2;
    obj->x = x << FIXED_POINT;
    obj->y = y << FIXED_POINT;
    obj->x0 = -3;
    obj->x1 = 1;
    obj->y0 = -2;
    obj->y1 = 2;
    
    if (dir == 0) {
        obj->tmp = 0;
        if (gameRandom() & 1) {
            obj->tmp = (gameRandom() & 1) ? 1 : -1;
        }
    } else {
        obj->tmp = dir;
    }
    
    // Chance for big trash at higher difficulties
    if (_difficulty >= 5 && (gameRandom() & 3) == 0) {
        obj->ot = OT_BIG_TRASH;
        obj->y0--;
        obj->y1++;
        obj->x0--;
        obj->x1++;
    }
}

void SpaceGame::createGadget(uint8_t x, uint8_t y) {
    int8_t objIdx = findEmptyObject();
    if (objIdx < 0) return;
    
    GameObject* obj = &_objects[objIdx];
    obj->ot = OT_GADGET;
    obj->x = x << FIXED_POINT;
    obj->y = y << FIXED_POINT;
    obj->x0 = -3;
    obj->x1 = 1;
    obj->y0 = -2;
    obj->y1 = 2;
    obj->tmp = 8;
}

void SpaceGame::createWall() {
    int8_t objIdx = findEmptyObject();
    if (objIdx < 0) return;
    
    GameObject* obj = &_objects[objIdx];
    obj->ot = OT_WALL_SOLID;
    
    int8_t h = gameRandom() & 63;
    h = (int8_t)(((int16_t)h * (int16_t)(AREA_HEIGHT/4)) >> 6);
    h += AREA_HEIGHT/6;
    
    obj->x0 = 0;
    obj->x1 = 5;
    obj->x = (AREA_WIDTH - 1) << FIXED_POINT;
    
    if (gameRandom() & 1) {
        obj->y = (AREA_HEIGHT - 1) << FIXED_POINT;
        obj->y0 = -h;
        obj->y1 = 0;
    } else {
        obj->y = 0;
        obj->y0 = 0;
        obj->y1 = h;
    }
}

uint8_t SpaceGame::gameRandom() {
    return rand() & 0xFF;
}

char* SpaceGame::intToString(unsigned long value) {
    volatile unsigned char i = 11;
    _itoaBuf[11] = '\0';
    while (i > 0) {
        i--;
        _itoaBuf[i] = (value % 10) + '0';
        value /= 10;
        if (value == 0) break;
    }
    return _itoaBuf + i;
}

} // namespace Display
