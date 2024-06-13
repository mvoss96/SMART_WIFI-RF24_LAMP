#pragma once
#include <cstdint>
#include "config.h"

void ledInit();
int ledSet();

const uint16_t LED_MAX_VAL = 1024;

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

bool getLedPower();
bool toggleLedPower(bool power);
void setLedPower(bool power);

uint16_t getLedBrightness();
void setLedBrightness(uint16_t brightness);

uint16_t getLedColor();
void setLedColor(uint16_t color);

uint16_t getLedRed();
void setLedRed(uint16_t red);
uint16_t getLedGreen();
void setLedGreen(uint16_t green);
uint16_t getLedBlue();
void setLedBlue(uint16_t blue);
uint16_t getLedWW();
void setLedWW(uint16_t ww);
uint16_t getLedCW();
void setLedCW(uint16_t cw);

void setLedRgb(uint16_t red, uint16_t green, uint16_t blue);
void setLedRgbw(uint16_t red, uint16_t green, uint16_t blue, uint16_t ww);
void setLedRgbww(uint16_t red, uint16_t green, uint16_t blue, uint16_t ww, uint16_t cw);
