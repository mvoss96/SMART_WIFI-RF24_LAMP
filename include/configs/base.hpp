#pragma once

#define SW_VERSION "1.2.0" // Software version
#define LED_MAX_VAL 1024   // Maximum value for LED brightness and color (1024 for 10-bit PWM) DO NOT CHANGE

// WiFi Configuration
#define WIFI_RECONNECT_ATTEMPT_INTERVAL 2000 // Interval between WiFi reconnection attempts in milliseconds

// MQTT Configuration
#define MQTT_MIN_DELAY 50                    // Minimum delay between MQTT messages in milliseconds
#define MQTT_PUBLISH_INTERVAL 5000           // Interval between MQTT publishes in milliseconds (-1 for no interval)
#define MQTT_RECONNECT_ATTEMPT_INTERVAL 5000 // Interval between MQTT reconnection attempts in milliseconds

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
