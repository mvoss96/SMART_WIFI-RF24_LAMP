#include "Button.hpp"
#include "ledControl.h"
#include <Arduino.h>

const unsigned int debounceTime = 30;
const unsigned int holdInterval = 150;

Button::Button(unsigned int pin, bool activeHigh, BUTTON_BEHAVIOR behavior)
{
    this->pin = pin;
    this->activeHigh = activeHigh;
    this->behavior = behavior;
    pinMode(pin, activeHigh ? INPUT_PULLDOWN : INPUT_PULLUP);
}

void Button::actClick()
{
    switch (behavior)
    {
    case BUTTON_BEHAVIOR::TOGGLE:
    case BUTTON_BEHAVIOR::DIMMER:
        toggleLedPower();
        break;
    default:
        break;
    }
}

void Button::actHold(unsigned long counter)
{
    switch (behavior)
    {
    case BUTTON_BEHAVIOR::TOGGLE:
        if (holdCounter == 1)
        {
            toggleLedPower();
        }
        break;
    case BUTTON_BEHAVIOR::DIMMER:
        if (dimmingDirection)
        {
            increaseLedBrightness();
        }
        else
        {
            decreaseLedBrightness();
        }
        break;
    default:
        break;
    }
}

void Button::update()
{
    bool current_state = digitalRead(pin) == activeHigh;
    if (current_state != last_state)
    {
        lastChangeTime = millis();
    }
    if ((millis() - lastChangeTime) > debounceTime)
    {
        if (current_state != state)
        {
            state = current_state;
            if (state)
            {
                lastHoldTime = millis();
                dimmingDirection = getLedBrightness() != LED_MAX_VAL;
                holdCounter = 0;
            }
            else if (holdCounter == 0)
            {
                actClick();
            }
        }
    }
    if (state)
    {
        unsigned long now = millis();
        if (now - lastHoldTime >= holdInterval)
        {
            holdCounter++;
            actHold(holdCounter);
            lastHoldTime = now;
        }
    }
    last_state = current_state;
}
