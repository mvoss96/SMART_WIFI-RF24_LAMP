#pragma once
#include "config.h"

class Button {
private:
    bool dimmingDirection{true};
    unsigned int pin{};
    bool activeHigh{};
    bool state{};
    bool last_state{};
    unsigned long holdCounter{};
    BUTTON_BEHAVIOR behavior{};
    unsigned long lastChangeTime{};
    unsigned long lastHoldTime{};

    void actClick();
    void actHold(unsigned long counter);

public:
    Button(unsigned int pin, bool activeHigh, BUTTON_BEHAVIOR behavior);
    void update();
};
