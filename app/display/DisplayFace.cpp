#include "display/Display.h"

namespace Display {

void Display::faceInit() {
    _face = new Face(_u8g2, _width, _height - 14, 40); // last 20 px for status bar
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
}

Face *Display::getFace() {
    return _face;
}

void Display::autoFace(bool exp) {
  _face->RandomBehavior =
  _face->RandomBlink =
  _face->RandomLook =
    exp;
}

} // end namespace