# Screen and Face Animation System Documentation

## Overview

The Screen and Face Animation System is a component of the Cozmo-System project that provides an expressive character face with animated eyes on OLED displays. This system creates lifelike eye animations, expressions, and behaviors that enhance the robot's character and user interaction.

## Table of Contents

1. [Screen Component](#screen-component)
2. [Face System Architecture](#face-system-architecture)
3. [Eye Animation System](#eye-animation-system)
4. [Face Expressions](#face-expressions)
5. [Behavior System](#behavior-system)
6. [Implementation Details](#implementation-details)
7. [Usage Examples](#usage-examples)

---

## Screen Component

The `Screen` class provides a high-level interface for controlling an OLED display using the U8g2 library. It manages display initialization, drawing operations, and integrates with the Face animation system.

### Key Features

- Integration with U8g2 library for OLED control
- Thread-safe access with FreeRTOS semaphore protection
- Abstract drawing API (text, shapes, lines)
- Integration with Face animation system

### API Reference

```cpp
namespace Screen {

class Screen {
public:
    Screen();
    ~Screen();
    
    bool init(int sda = 14, int scl = 15);
    
    // Drawing methods
    void clear();
    void drawText(int x, int y, const String& text, const uint8_t* font = nullptr);
    void drawCenteredText(int y, const String& text, const uint8_t* font = nullptr);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x, int y, int width, int height, bool fill = false);
    void drawCircle(int x, int y, int radius, bool fill = false);
    
    // Screen update methods
    void update();
    void mutexUpdate(); // Thread-safe update

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* _u8g2;
    bool _initialized;
    SemaphoreHandle_t _mux;
    Face* _face;
    bool _holdFace;
    
    bool _lock();
    void _unlock();
};

} // namespace Screen
```

### Example Usage

```cpp
#include "Screen/Screen.h"

Screen::Screen screen;

void setup() {
    // Initialize the screen
    screen.init(14, 15);  // SDA, SCL pins
    
    // Draw some text and shapes
    screen.clear();
    screen.drawCenteredText(10, "Cozmo Robot");
    screen.drawRect(10, 30, 108, 20, false);
    screen.drawCenteredText(40, "Ready!");
    
    // Update the display
    screen.update();
}
```

---

## Face System Architecture

The Face Animation System is built with a hierarchical architecture that separates concerns and provides flexible control:

```
Face
├── LeftEye
│   ├── EyeConfig
│   ├── EyeTransformation
│   ├── EyeVariation
├── RightEye
│   ├── EyeConfig
│   ├── EyeTransformation
│   ├── EyeVariation
├── BlinkAssistant
├── LookAssistant
├── FaceBehavior
└── FaceExpression
```

Each component in this hierarchy has a specific responsibility:

- **Face**: Coordinates the overall face animation and manages the eyes
- **Eye**: Represents a single eye with its configuration and state
- **EyeConfig**: Stores the visual parameters that define an eye's appearance
- **EyeTransformation**: Handles geometric transformations like scaling and rotation
- **EyeVariation**: Manages subtle variations for more natural movement
- **BlinkAssistant**: Controls blinking behavior and timing
- **LookAssistant**: Manages eye gaze direction
- **FaceBehavior**: Controls higher-level behaviors and emotional states
- **FaceExpression**: Defines preset facial expressions

---

## Eye Animation System

The Eye Animation System is the core of the face animation capabilities. Each eye is drawn based on a set of configurable parameters that determine its shape, size, and emotional expression.

### Key Components

#### Eye Class

The `Eye` class represents a single eye, with configuration and drawing capabilities.

```cpp
class Eye {
public:
    Eye();
    
    int16_t CenterX;
    int16_t CenterY;
    EyeConfig Config;
    
    void Draw(U8G2_SSD1306_128X64_NONAME_F_HW_I2C *_u8g2);
    
    void SetLookXY(int16_t x, int16_t y);
    void SetLookX(int16_t x);
    void SetLookY(int16_t y);
    
    void AddVariation(EyeVariation *variation);
    void AddTransform(EyeTransformation *tranform);
    
    void Update();
};
```

#### EyeConfig

The `EyeConfig` struct contains all parameters that define how an eye looks:

```cpp
struct EyeConfig {
    int16_t Height;              // Eye height
    int16_t Width;               // Eye width
    int16_t EyelidPosition;      // Position of eyelid (0-100%)
    int8_t TopRadiusX;           // Top X-radius
    int8_t TopRadiusY;           // Top Y-radius
    int8_t BottomRadiusX;        // Bottom X-radius
    int8_t BottomRadiusY;        // Bottom Y-radius
    float Slope_Top;             // Slant of top eyelid
    float Slope_Bottom;          // Slant of bottom eyelid
    
    // Pupil parameters
    int16_t PupilHeight;         // Pupil height
    int16_t PupilWidth;          // Pupil width
    int16_t PupilPosX;           // Pupil X position offset
    int16_t PupilPosY;           // Pupil Y position offset
    
    // Expression parameters
    bool IsAngry;                // Angry expression flag
    bool IsHappy;                // Happy expression flag
};
```

#### EyeDrawer

The `EyeDrawer` class handles the actual drawing of eyes based on the eye configuration:

```cpp
class EyeDrawer {
public:
    static void Draw(U8G2_SSD1306_128X64_NONAME_F_HW_I2C *_u8g2, 
                   int16_t centerX, int16_t centerY, 
                   EyeConfig *config);
};
```

---

## Face Expressions

The `FaceExpression` class manages transitions between different facial expressions. It provides preset expressions like normal, angry, happy, suspicious, and sad.

### Key Features

- Preset expressions with appropriate eye configurations
- Smooth transitions between expressions
- Expression interpolation for natural movement

### API Reference

```cpp
class FaceExpression {
public:
    FaceExpression(Eye* _leftEye, Eye* _rightEye);
    
    void Update();
    
    void GoTo_Normal();
    void GoTo_Angry();
    void GoTo_Happy();
    void GoTo_Suspicious();
    void GoTo_Sad();
    
private:
    Eye* LeftEye;
    Eye* RightEye;
    
    EyeTransition* Transition;
};
```

### Example Usage

```cpp
// In a setup function
face->Expression.GoTo_Normal();

// Later, change expression
face->Expression.GoTo_Happy();

// In an angry situation
face->Expression.GoTo_Angry();
```

---

## Behavior System

The `FaceBehavior` class manages randomized behaviors that make the face appear more lifelike and engaging.

### Key Features

- Emotion weighting system
- Random expression transitions based on weighted probability
- Timing control for natural pacing

### API Reference

```cpp
enum class eEmotions {
    Normal,
    Angry,
    Happy,
    Suspicious,
    Sad
};

class FaceBehavior {
public:
    FaceBehavior(FaceExpression* _expression);
    
    void SetEmotion(eEmotions emotion, float weight);
    void Update();

private:
    FaceExpression* Expression;
    unsigned long LastEmotionChange;
    std::map<eEmotions, float> EmotionWeights;
    
    eEmotions GetRandomEmotion();
};
```

---

## Implementation Details

### Blinking

The `BlinkAssistant` class manages eye blinking with natural timing and animations:

```cpp
class BlinkAssistant {
public:
    BlinkAssistant(Eye* _leftEye, Eye* _rightEye);
    
    void Update();
    void ForceBlink();
    
    bool IsBlinking = false;
    bool RandomBlink = true;

private:
    Eye* LeftEye;
    Eye* RightEye;
    
    // Blink timing variables and state management
};
```

Blinking follows a natural pattern with:
1. Random intervals between blinks (2-8 seconds)
2. Quick closing and opening phases
3. Occasional double-blinks for added realism

### Looking Around

The `LookAssistant` class manages eye gaze with natural movement patterns:

```cpp
class LookAssistant {
public:
    LookAssistant(Eye* _leftEye, Eye* _rightEye);
    
    void Update();
    void LookAt(int16_t x, int16_t y);
    
    bool RandomLook = true;

private:
    Eye* LeftEye;
    Eye* RightEye;
    
    // Look timing variables and state management
};
```

The looking behavior creates natural eye movement with:
1. Random gaze points within the possible eye range
2. Smooth transitions between gaze points
3. Variable dwell time at each gaze point

### Eye Transitions

The `EyeTransition` class handles smooth animations between eye states:

```cpp
class EyeTransition {
public:
    EyeTransition(Eye* _eye);
    
    void Start(EyeConfig startConfig, EyeConfig endConfig, unsigned long duration);
    void Update();
    
    bool InProgress = false;

private:
    Eye* TargetEye;
    
    EyeConfig StartConfig;
    EyeConfig EndConfig;
    EyeConfig CurrentConfig;
    
    unsigned long StartTime;
    unsigned long Duration;
};
```

Transitions use linear interpolation to smoothly animate between eye configurations, creating fluid and natural movements between expressions and states.

---

## Usage Examples

### Basic Face Initialization

```cpp
#include "Screen/Screen.h"
#include "Screen/Face/Face.h"

Screen::Screen screen;
Face* face;

void setup() {
    // Initialize screen
    screen.init(14, 15);  // SDA, SCL pins
    
    // Create a face with appropriate dimensions
    // (u8g2 object, screen width, screen height, eye size)
    face = new Face(screen._u8g2, 128, 64, 40);
    
    // Set default expression
    face->Expression.GoTo_Normal();
    
    // Enable random behaviors
    face->RandomBehavior = true;
    face->RandomLook = true;
    face->RandomBlink = true;
}

void loop() {
    // Update face animation
    face->Update();
    
    // Other code...
}
```

### Changing Expressions

```cpp
// Show happy expression
face->Expression.GoTo_Happy();

// After 2 seconds, show angry expression
delay(2000);
face->Expression.GoTo_Angry();

// After another 2 seconds, return to normal
delay(2000);
face->Expression.GoTo_Normal();
```

### Controlling Eye Direction

```cpp
// Look left
face->LookLeft();

// Look right after 1 second
delay(1000);
face->LookRight();

// Look forward after 1 second
delay(1000);
face->LookFront();

// Force a blink
face->Blink.ForceBlink();
```

### Setting Emotion Weights

```cpp
// Configure emotion probabilities
face->Behavior.SetEmotion(eEmotions::Normal, 0.7);     // 70% normal
face->Behavior.SetEmotion(eEmotions::Happy, 0.2);      // 20% happy 
face->Behavior.SetEmotion(eEmotions::Suspicious, 0.1); // 10% suspicious
face->Behavior.SetEmotion(eEmotions::Angry, 0.0);      // 0% angry
face->Behavior.SetEmotion(eEmotions::Sad, 0.0);        // 0% sad
```
