#include <Arduino.h>
#include <Preferences.h>
#include "ledControl.h"
#include "config.h"
#include "logging.h"

static Preferences preferences;

static const int pins[] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN};
static const size_t numLEDs = sizeof(pins) / sizeof(pins[0]);
static bool stateChanged = false;
static uint32_t ledcTargetValues[numLEDs] = {0};
static uint32_t ledcCurrentValues[numLEDs] = {0};

// Callback function pointer for LED state change
static void (*ledCallback)(void) = NULL;

LEDSettings ledSettings;

IRAM_ATTR void setLedCallback(void (*callback)(void))
{
    ledCallback = callback;
}

static void loadLedSettings()
{
    preferences.begin("led", true);
    ledSettings.power = preferences.getBool("ledPower", true);
    ledSettings.brightness = preferences.getUShort("ledBrightness", 256);
    ledSettings.color = preferences.getUShort("ledColor", 0);
    ledSettings.red = preferences.getUShort("ledRed", 0);
    ledSettings.green = preferences.getUShort("ledGreen", 0);
    ledSettings.blue = preferences.getUShort("ledBlue", 0);
    ledSettings.ww = preferences.getUShort("ledWw", 0);
    ledSettings.cw = preferences.getUShort("ledCw", 0);
    preferences.end();

    LOG_INFO("Loaded LED settings: power=%i, brightness=%i, color=%i, red=%i, green=%i, blue=%i, ww=%i, cw=%i\n",
             ledSettings.power, ledSettings.brightness, ledSettings.color, ledSettings.red, ledSettings.green, ledSettings.blue, ledSettings.ww, ledSettings.cw);
}

static void saveLedSettings()
{
    preferences.begin("led", false);
    preferences.putBool("ledPower", ledSettings.power);
    preferences.putUShort("ledBrightness", ledSettings.brightness);
    preferences.putUShort("ledColor", ledSettings.color);
    preferences.putUShort("ledRed", ledSettings.red);
    preferences.putUShort("ledGreen", ledSettings.green);
    preferences.putUShort("ledBlue", ledSettings.blue);
    preferences.putUShort("ledWw", ledSettings.ww);
    preferences.putUShort("ledCw", ledSettings.cw);
    preferences.end();
}

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
    loadLedSettings();
    ledSet();
}

void ledUpdate()
{
    const int fadeStep = LED_FADE_STEP_SIZE;
    for (int i = 0; i < numLEDs; i++)
    {
        int delta = ledcTargetValues[i] - ledcCurrentValues[i];
        if (delta == 0)
        {
            continue; // Skip if no change needed
        }
        // Serial.printf("Current:%i, Target:%i, Delta: %i ", ledcCurrentValues[i], ledcTargetValues[i], delta);

        // Adjust the new value by the fadeStep in the direction of the delta using the ternary operator
        int step = (delta > 0) ? min(fadeStep, delta) : max(-fadeStep, delta);
        uint32_t newVal = ledcCurrentValues[i] + step;
        // Serial.printf("NewVal: %i\n", newVal);
        ledcCurrentValues[i] = newVal;
        ledcWrite(i, newVal);
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
        ledcTargetValues[0] = ledSettings.power * ledSettings.brightness;
        break;
    case LED_MODES::CCT:
        warmWhite = ledSettings.brightness * ledSettings.color / 1024;
        coldWhite = ledSettings.brightness * (1024 - ledSettings.color) / 1024;
        ledcTargetValues[0] = ledSettings.power * warmWhite;
        ledcTargetValues[1] = ledSettings.power * coldWhite;
        break;
    case LED_MODES::RGB:
    case LED_MODES::RGBW:
    case LED_MODES::RGBWW:
        channelCount = (LED_MODE == LED_MODES::RGB) ? 3 : ((LED_MODE == LED_MODES::RGBW) ? 4 : 5);

        for (uint8_t i = 0; i < channelCount; ++i)
        {
            ledcTargetValues[channels[i]] = ledSettings.power * ledSettings.brightness / divider * colors[i];
        }
        break;
    default:
        LOG_ERROR("Invalid LED mode");
        return -1;
    }
    // Call the callback function if set
    if (ledCallback)
    {
        ledCallback();
    }
    saveLedSettings(); // Save LED settings to preferences for restoration after reboot
    return 0;
}

void setLedPower(bool power)
{
    ledSettings.power = power;
    ledSet();
}

bool getLedPower()
{
    return ledSettings.power;
}

bool toggleLedPower()
{
    ledSettings.power = !ledSettings.power;
    ledSet();
    return ledSettings.power;
}

uint16_t getLedBrightness()
{
    return ledSettings.brightness;
}

void setLedBrightness(uint16_t brightness)
{
    if (brightness > LED_MAX_VAL)
    {
        LOG_WARNING("Brightness value %i is greater than maximum value %i, setting to maximum value\n", brightness, LED_MAX_VAL);
        brightness = LED_MAX_VAL;
    }
    ledSettings.brightness = brightness;
    ledSet();
}

void increaseLedBrightness()
{
    if (ledSettings.power == false)
    {
        return; // Do not increase brightness if LED is off
    }
    setLedBrightness(min(ledSettings.brightness + BRIGHTNESS_STEP_SIZE, LED_MAX_VAL));
}

void decreaseLedBrightness()
{
    if (ledSettings.power == false)
    {
        return; // Do not decrease brightness if LED is off
    }
    setLedBrightness(max(ledSettings.brightness - BRIGHTNESS_STEP_SIZE, MIN_BRIGHTNESS));
}

uint16_t getLedColor()
{
    return ledSettings.color;
}

uint16_t getLedColorTemperature()
{
    // Calculate color temperature in mireds
    return (uint16_t)(0.5 + (float)(ledSettings.color) * (MAX_MIREDS - MIN_MIREDS) / LED_MAX_VAL + MIN_MIREDS);
}

void setLedColorTemperature(uint16_t mireds)
{
    setLedColor((uint16_t)(0.5 + (float)(mireds - MIN_MIREDS) * LED_MAX_VAL / (MAX_MIREDS - MIN_MIREDS)));
}

void setLedColor(uint16_t color)
{
    if (color > LED_MAX_VAL)
    {
        LOG_WARNING("Color value %i is greater than maximum value %i, setting to maximum value\n", color, LED_MAX_VAL);
        color = LED_MAX_VAL;
    }
    ledSettings.color = color;
    ledSet();
}

void increaseLedColor()
{
    setLedColor(min(ledSettings.color + COLOR_STEP_SIZE, LED_MAX_VAL));
}

void decreaseLedColor()
{
    setLedColor(max(ledSettings.color - COLOR_STEP_SIZE, 0));
}

uint16_t getLedRed()
{
    return ledSettings.red;
}

void setLedRed(uint16_t red)
{
    if (red > LED_MAX_VAL)
    {
        LOG_WARNING("Red value %i is greater than maximum value %i, setting to maximum value\n", red, LED_MAX_VAL);
        red = LED_MAX_VAL;
    }
    ledSettings.red = red;
    ledSet();
}

void increaseLedRed()
{
    setLedRed(min(ledSettings.red + COLOR_STEP_SIZE, LED_MAX_VAL));
}

void decreaseLedRed()
{
    setLedRed(max(ledSettings.red - COLOR_STEP_SIZE, 0));
}

uint16_t getLedGreen()
{
    return ledSettings.green;
}

void setLedGreen(uint16_t green)
{
    if (green > LED_MAX_VAL)
    {
        LOG_WARNING("Green value %i is greater than maximum value %i, setting to maximum value\n", green, LED_MAX_VAL);
        green = LED_MAX_VAL;
    }
    ledSettings.green = green;
    ledSet();
}

void increaseLedGreen()
{
    setLedGreen(min(ledSettings.green + COLOR_STEP_SIZE, LED_MAX_VAL));
}

void decreaseLedGreen()
{
    setLedGreen(max(ledSettings.green - COLOR_STEP_SIZE, 0));
}

uint16_t getLedBlue()
{
    return ledSettings.blue;
}

void setLedBlue(uint16_t blue)
{
    if (blue > LED_MAX_VAL)
    {
        LOG_WARNING("Blue value %i is greater than maximum value %i, setting to maximum value\n", blue, LED_MAX_VAL);
        blue = LED_MAX_VAL;
    }
    ledSettings.blue = blue;
    ledSet();
}

void increaseLedBlue()
{
    setLedBlue(min(ledSettings.blue + COLOR_STEP_SIZE, LED_MAX_VAL));
}

void decreaseLedBlue()
{
    setLedBlue(max(ledSettings.blue - COLOR_STEP_SIZE, 0));
}

uint16_t getLedWW()
{
    return ledSettings.ww;
}

void setLedWW(uint16_t ww)
{
    if (ww > LED_MAX_VAL)
    {
        LOG_WARNING("WW value %i is greater than maximum value %i, setting to maximum value\n", ww, LED_MAX_VAL);
        ww = LED_MAX_VAL;
    }
    ledSettings.ww = ww;
    ledSet();
}

void increaseLedWW()
{
    setLedWW(min(ledSettings.ww + COLOR_STEP_SIZE, LED_MAX_VAL));
}

void decreaseLedWW()
{
    setLedWW(max(ledSettings.ww - COLOR_STEP_SIZE, 0));
}

uint16_t getLedCW()
{
    return ledSettings.cw;
}

void setLedCW(uint16_t cw)
{
    if (cw > LED_MAX_VAL)
    {
        LOG_WARNING("CW value %i is greater than maximum value %i, setting to maximum value\n", cw, LED_MAX_VAL);
        cw = LED_MAX_VAL;
    }
    ledSettings.cw = cw;
    ledSet();
}

void increaseLedCW()
{
    setLedCW(min(ledSettings.cw + COLOR_STEP_SIZE, LED_MAX_VAL));
}

void decreaseLedCW()
{
    setLedCW(max(ledSettings.cw - COLOR_STEP_SIZE, 0));
}

void setLedRgb(uint16_t red, uint16_t green, uint16_t blue)
{
    setLedRed(red);
    setLedGreen(green);
    setLedBlue(blue);
    ledSet();
}

void setLedRgbw(uint16_t red, uint16_t green, uint16_t blue, uint16_t ww)
{
    setLedRgb(red, green, blue);
    setLedWW(ww);
    ledSet();
}

void setLedRgbww(uint16_t red, uint16_t green, uint16_t blue, uint16_t ww, uint16_t cw)
{
    setLedRgbw(red, green, blue, ww);
    setLedCW(cw);
    ledSet();
}
