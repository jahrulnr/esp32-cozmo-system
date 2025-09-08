#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include "core/Sensors/OrientationSensor.h"

namespace Display {

/**
 * SpaceTrash-style game adapted for gyroscope control
 * Based on the U8g2 SpaceTrash example but using orientation sensor for player movement
 */
class SpaceGame {
public:
    SpaceGame(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display, void* unused = nullptr, int width = 128, int height = 64);
    ~SpaceGame();

    /**
     * Initialize the game
     * @return true if initialization was successful
     */
    bool init();

    /**
     * Draw the game (called during display update cycle)
     */
    void draw();

    /**
     * Start the game
     */
    void startGame();

    /**
     * Pause the game
     */
    void pauseGame();

    /**
     * Check if game is running
     * @return true if game is active
     */
    bool isGameActive() const { return _gameActive; }

    /**
     * Check if game is in game over state
     * @return true if showing game over screen
     */
    bool isGameOver() const { return _gameState == STATE_END; }

    /**
     * Get current score
     * @return Current player score
     */
    uint16_t getScore() const { return _playerPoints; }

    /**
     * Get high score
     * @return High score
     */
    uint16_t getHighScore() const { return _highScore; }

    /**
     * Set fire control (external button/trigger)
     * @param isFiring true when fire button is pressed
     */
    void setFireControl(bool isFiring);

    /**
     * Enable/disable auto-fire mode
     * @param autoFire true to enable auto-fire
     */
    void setAutoFire(bool autoFire);

    /**
     * Set gyro sensitivity for player movement
     * @param sensitivity Sensitivity multiplier (0.1 = low, 1.0 = high)
     */
    void setGyroSensitivity(float sensitivity);

    /**
     * Update player position based on orientation sensor data
     * @param orientation The orientation sensor instance
     */
    void updateGyroInput(Sensors::OrientationSensor* orientation);

private:
    // Game constants
    static const int FIXED_POINT = 4;
    static const int MAX_OBJECTS = 45;
    static const int AREA_HEIGHT = 56; // Leave space for UI
    static const int AREA_WIDTH = 128;
    static const int POINTS_PER_LEVEL = 25;
    static const int DIFF_VIS_LEN = 30;
    static const int DIFF_FP = 5;

    // Game states (simplified - only need GAME state)
    enum GameState {
        STATE_GAME = 0,
        STATE_END = 1
    };

    // Object types
    enum ObjectType {
        OT_EMPTY = 0,
        OT_WALL_SOLID = 1,
        OT_BIG_TRASH = 2,
        OT_MISSILE = 3,
        OT_TRASH1 = 4,
        OT_PLAYER = 5,
        OT_DUST_PY = 6,
        OT_DUST_NY = 7,
        OT_TRASH_IMPLODE = 8,
        OT_TRASH2 = 9,
        OT_PLAYER2 = 10,
        OT_PLAYER3 = 11,
        OT_GADGET = 12,
        OT_GADGET_IMPLODE = 13,
        OT_DUST_NXPY = 14,
        OT_DUST_NXNY = 15
    };

    // Game object structure
    struct GameObject {
        uint8_t ot;        // object type
        int8_t tmp;        // generic value
        int16_t x, y;      // position (fixed point)
        int8_t x0, y0, x1, y1; // bounding box
    };

    // Hardware references
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* _display;
    int _width;
    int _height;

    // Game state
    bool _gameActive;
    GameState _gameState;
    GameObject _objects[MAX_OBJECTS];
    
    // Player state
    uint8_t _playerPos;
    uint16_t _playerPoints;
    uint16_t _playerPointsDelayed;
    uint16_t _highScore;
    
    // Game progression
    uint8_t _difficulty;
    uint16_t _toDiffCnt;
    
    // Input state
    float _gyroSensitivity;
    float _playerAccel;
    bool _isFiring;
    bool _autoFire;
    uint8_t _firePlayer;
    uint8_t _firePeriod;
    uint8_t _manualFireDelay;
    uint8_t _isFireLastValue;
    
    // Timing
    unsigned long _lastGyroUpdate;
    
    // Gyro control (following Cube3D pattern)
    float _centerPosition;
    float _currentTilt;
    float _alpha; // Complementary filter coefficient
    float _stationaryTime; // Track how long device has been stationary
    float _gyroThreshold; // Threshold for considering device "stationary"
    
    // Debug variables for gyro/accel display (like Cube3D)
    float _lastGyroX, _lastGyroY, _lastGyroZ;
    float _lastAccelX, _lastAccelY, _lastAccelZ;
    float _debugAccelRoll; // Store calculated accelerometer roll angle
    float _debugGyroRoll;  // Store processed gyro roll rate

    /**
     * Initialize game objects for new game
     */
    void setupInGame();

    /**
     * Update game logic for one frame
     */
    void stepInGame();

    /**
     * Update player position based on gyroscope input
     */
    void updatePlayerPosition();

    /**
     * Handle fire control logic
     */
    void updateFireControl();

    /**
     * Move all game objects
     */
    void moveObjects();

    /**
     * Handle collision detection and missile impacts
     */
    void handleCollisions();

    /**
     * Generate new enemies and obstacles
     */
    void generateNewObjects();

    /**
     * Draw game objects
     */
    void drawGameObjects();

    /**
     * Draw game UI (score, level, etc.)
     */
    void drawGameUI();

    /**
     * Draw game over screen
     */
    void drawGameOver();

    /**
     * Utility functions
     */
    int8_t findEmptyObject();
    void clearObjects();
    uint8_t isObjectOutOfBounds(int8_t objIdx);
    void destroyObject(int8_t objIdx);
    bool checkCollision(int8_t obj1, int8_t obj2);
    void createPlayerMissile(uint8_t x, uint8_t y);
    void createTrash(uint8_t x, uint8_t y, int8_t dir);
    void createGadget(uint8_t x, uint8_t y);
    void createWall();

    /**
     * Random number generator
     */
    uint8_t gameRandom();

    /**
     * Simple itoa implementation for score display
     */
    char* intToString(unsigned long value);
    char _itoaBuf[12];
};

} // namespace Display
