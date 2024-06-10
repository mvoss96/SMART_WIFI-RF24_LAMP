#include <Arduino.h>
#include "ledControl.h"
#include "config.h"

static const int pins[] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN};
static const size_t numLEDs = sizeof(pins) / sizeof(pins[0]);

LEDSettings ledSettings;

void ledInit()
{
    for (int i = 0; i < numLEDs; ++i)
    {
        if (pins[i] != -1)
        {
            ledcSetup(i, LED_PWM_FREQUENCY, 10);
            ledcAttachPin(pins[i], i);
            ledcWrite(i, 0);
        }
    }
}

int ledSet()
{
    const uint16_t divider = 65535; // max for uint16_t
    uint16_t warmWhite, coldWhite;
    uint8_t channels[] = {0, 1, 2, 3, 4};
    uint16_t colors[] = {ledSettings.red, ledSettings.green, ledSettings.blue, ledSettings.ww, ledSettings.cw};
    uint8_t channelCount;

    switch (LED_MODE)
    {
    case LED_MODES::SINGLE:
        ledcWrite(0, ledSettings.power * ledSettings.brightness);
        break;
    case LED_MODES::CCT:
        warmWhite = ledSettings.brightness * ledSettings.color / 1024;
        coldWhite = ledSettings.brightness * (1024 - ledSettings.color) / 1024;
        ledcWrite(0, ledSettings.power * warmWhite);
        ledcWrite(1, ledSettings.power * coldWhite);
        break;
    case LED_MODES::RGB:
    case LED_MODES::RGBW:
    case LED_MODES::RGBWW:
        channelCount = (LED_MODE == LED_MODES::RGB) ? 3 : ((LED_MODE == LED_MODES::RGBW) ? 4 : 5);

        for (uint8_t i = 0; i < channelCount; ++i)
        {
            ledcWrite(channels[i], ledSettings.power * ledSettings.brightness / divider * colors[i]);
        }
        break;
    default:
        Serial.println("Invalid LED mode");
        return -1;
    }
    return 0;
}
