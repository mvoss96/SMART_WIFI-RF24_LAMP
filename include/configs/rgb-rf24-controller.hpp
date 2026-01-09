#pragma once
#include "base.hpp"

// Model Configuration
#define MODELNAME "SMART-WIFI-RF24-Lamp" // Model name used as default device name

// Output LED Configuration
#define LED_MODE LED_MODES::CCT               // Set the LED mode
#define LED_PWM_FREQUENCY 30000               // Frequency for LED PWM Control
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

// Status LED Configuration
#define ENABLE_STATUS_LED // Uncomment to enable status LED
#define STATUS_LED_PIN 20 // Pin for status LED

// Button Configuration
// #define ENABLE_BUTTON1                           // Uncomment to enable Button1
// #define BUTTON1_PIN 10                           // Pin for Button1
// #define BUTTON1_ACTIVE_HIGH false                // Set to true if button is active high
// #define BUTTON1_BEHAVIOR BUTTON_BEHAVIOR::TOGGLE // Set the button behavior

// RF24 Configuration
#define RF24RADIO_ENABLED          // Uncomment to enable RF24 radio
#define RF24RADIO_WATCHDOG_ENABLED // Uncomment to enable RF24 radio watchdog
#define PIN_RADIO_CE 7             // Radio CE pin
#define PIN_RADIO_CSN 8            // Radio CSN pin
#define PIN_RADIO_IRQ 9            // Radio IRQ pin