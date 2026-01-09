#pragma once
#include "config.h"
#include <cstdint>

void ledInit();
int ledSet(uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);

struct LEDSettings
{
    bool power = false;
    uint16_t color = 0;
    uint16_t brightness = 0;
    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;
    uint16_t ww = 0;
    uint16_t cw = 0;
};

void ledUpdate();
void setLedCallback(void (*callback)(void));
bool getLedPower();
bool toggleLedPower();
void setLedPower(bool power, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);

// Brightness
uint16_t getLedBrightness();
void setLedBrightness(uint16_t brightness, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void increaseLedBrightness();
void decreaseLedBrightness();

// Color
uint16_t getLedColor();
void setLedColor(uint16_t color, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void increaseLedColor();
void decreaseLedColor();
uint16_t getLedColorTemperature();
void setLedColorTemperature(uint16_t mireds, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);

// Red, Green, Blue
uint16_t getLedRed();
void setLedRed(uint16_t red, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void increaseLedRed();
void decreaseLedRed();
uint16_t getLedGreen();
void setLedGreen(uint16_t green, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void increaseLedGreen();
void decreaseLedGreen();
uint16_t getLedBlue();
void setLedBlue(uint16_t blue, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void increaseLedBlue();
void decreaseLedBlue();

// WW, CW
uint16_t getLedWW();
void setLedWW(uint16_t ww, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void increaseLedWW();
void decreaseLedWW();
uint16_t getLedCW();
void setLedCW(uint16_t cw, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void increaseLedCW();
void decreaseLedCW();

// RGB, RGBW, RGBWW
void setLedRgb(uint16_t red, uint16_t green, uint16_t blue, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void setLedRgbw(uint16_t red, uint16_t green, uint16_t blue, uint16_t ww, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
void setLedRgbww(uint16_t red, uint16_t green, uint16_t blue, uint16_t ww, uint16_t cw, uint32_t transitionTimeMs = DEFAULT_TRANSITION_TIME);
