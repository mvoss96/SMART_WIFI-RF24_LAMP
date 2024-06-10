#pragma once
#include <cstdint>
#include "config.h"

void ledInit();
int ledSet();

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

extern LEDSettings ledSettings;
