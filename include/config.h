#pragma once

enum class LED_MODES
{
    SINGLE, // Single color LED (LED1)
    CCT,    // Color temperature LED (LED1 = Warm White, LED2 = Cold White)
    RGB,    // RGB LED (LED1 = Red, LED2 = Green, LED3 = Blue)
    RGBW,   // RGBW LED (LED1 = Red, LED2 = Green, LED3 = Blue, LED4 = White)
    RGBWW   // RGBWW LED (LED1 = Red, LED2 = Green, LED3 = Blue, LED4 = Warm White, LED5 = Cold White)
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
}

#define DEVICENAME "ESP32RF-Moon-Lamp"       // Name of the device also used as Hostname for WiFi AP
#define WIFI_RECONNECT_ATTEMPT_INTERVAL 2000 // Interval between WiFi reconnection attempts in milliseconds
#define MQTT_RECONNECT_ATTEMPT_INTERVAL 5000 // Interval between MQTT reconnection attempts in milliseconds
#define LED_PWM_FREQUENCY 15000              // Frequency for LED PWM Control
#define MQTT_PUBLISH_INTERVAL 10000          // Interval between MQTT publishes in milliseconds

#define LED_MODE LED_MODES::SINGLE           // Set the LED mode
#define LED1_PIN 3                           // Pin for LED1
#define LED2_PIN -1                          // Pin for LED2 set to -1 if not used
#define LED3_PIN -1                          // Pin for LED3 set to -1 if not used
#define LED4_PIN -1                          // Pin for LED4 set to -1 if not used
#define LED5_PIN -1                          // Pin for LED5 set to -1 if not used