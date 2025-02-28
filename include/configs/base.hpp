#pragma once

#define SW_VERSION "1.0.5"        // Software version
#define LED_MAX_VAL 1024          // Maximum value for LED brightness and color (1024 for 10-bit PWM) DO NOT CHANGE

enum class LED_MODES
{
    SINGLE, // Single color LED (LED1)
    CCT,    // Color temperature LED (LED1 = Warm White, LED2 = Cold White)
    RGB,    // RGB LED (LED1 = Red, LED2 = Green, LED3 = Blue)
    RGBW,   // RGBW LED (LED1 = Red, LED2 = Green, LED3 = Blue, LED4 = White)
    RGBWW   // RGBWW LED (LED1 = Red, LED2 = Green, LED3 = Blue, LED4 = Warm White, LED5 = Cold White)
};

enum class BUTTON_BEHAVIOR
{
    TOGGLE, // Toggle the LED state
    DIMMER, // Dim the LED
};

inline const char *getLEDModeStr(LED_MODES mode)
{
    switch (mode)
    {
    case LED_MODES::SINGLE:
        return "SINGLE";
    case LED_MODES::CCT:
        return "CCT";
    case LED_MODES::RGB:
        return "RGB";
    case LED_MODES::RGBW:
        return "RGBW";
    case LED_MODES::RGBWW:
        return "RGBWW";
    default:
        return "UNKNOWN";
    }
};
