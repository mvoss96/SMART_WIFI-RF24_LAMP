#pragma once

#define LED_MAX_VAL 1024 // Maximum value for LED brightness and color (1024 for 10-bit PWM) DO NOT CHANGE
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

// Configuration
#define SW_VERSION "1.0.1"                   // Software version
#define MODELNAME "SMART WIFI-RF24 Lamp"     // Model name used as default device name
#define WIFI_RECONNECT_ATTEMPT_INTERVAL 2000 // Interval between WiFi reconnection attempts in milliseconds
#define MQTT_RECONNECT_ATTEMPT_INTERVAL 5000 // Interval between MQTT reconnection attempts in milliseconds
#define LED_PWM_FREQUENCY 30000              // Frequency for LED PWM Control
#define MQTT_MIN_DELAY 500                   // Minimum delay between MQTT messages in milliseconds
#define MQTT_PUBLISH_INTERVAL -1             // Interval between MQTT publishes in milliseconds (-1 for no interval)

#define LED_MODE LED_MODES::CCT               // Set the LED mode
#define BRIGHTNESS_STEP_SIZE LED_MAX_VAL / 16 // Number of brightness steps
#define COLOR_STEP_SIZE LED_MAX_VAL / 16      // Number of color steps
#define LED_FADE_STEP_SIZE 20                 // LED fade step size
#define MIN_BRIGHTNESS 5                      // Minimum brightness value
#define MIN_MIREDS 153                        // Minimum color temperature in Mireds (6500K)
#define MAX_MIREDS 370                        // Maximum color temperature in Mireds (2700K)
#define LED1_PIN 3                            // Pin for LED1
#define LED2_PIN 2                            // Pin for LED2 set to -1 if not used
#define LED3_PIN -1                           // Pin for LED3 set to -1 if not used
#define LED4_PIN -1                           // Pin for LED4 set to -1 if not used
#define LED5_PIN -1                           // Pin for LED5 set to -1 if not used

#define RF24RADIO_ENABLED true // Enable or disable RF24 radio (true or false)
#define PIN_RADIO_CE 7         // Radio CE pin
#define PIN_RADIO_CSN 8        // Radio CSN pin
#define PIN_RADIO_IRQ 9        // Radio IRQ pin

































































































































