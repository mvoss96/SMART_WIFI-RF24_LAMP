#include "config.h"

#include "Button.hpp"
#include "ioControl.h"
#include "ledControl.h"
#include "Logging/logging.h"

#include <Arduino.h>
#include <vector>

namespace
{
    unsigned long lastStatusLedUpdate = 0;
    STATUS_LED_CODES statusLedCode{STATUS_LED_CODES::STARTUP};
    std::vector<Button> buttons{};

    void ioInit()
    {
#ifdef ENABLE_BUTTON1
        buttons.push_back(Button(BUTTON1_PIN, BUTTON1_ACTIVE_HIGH, BUTTON1_BEHAVIOR));
#endif
#ifdef ENABLE_STATUS_LED
        pinMode(STATUS_LED_PIN, OUTPUT);
#endif
    }

    void ioUpdate()
    {
        for (auto &button : buttons)
        {
            button.update();
        }
    }

    void statusLedUpdate()
    {
        const uint16_t SLOW_BLINKING_PERIOD = 500; // 1 Hz (1s period, toggle every 0.5s)
        const uint16_t FAST_BLINKING_PERIOD = 167; // ~3 Hz (0.33s period, toggle every ~0.167s)
        static bool ledState = false;
        unsigned long now = millis();
        switch (statusLedCode)
        {
        case STATUS_LED_CODES::STARTUP:
            digitalWrite(STATUS_LED_PIN, LOW);
            break;
        case STATUS_LED_CODES::NO_WIFI:
            if (now - lastStatusLedUpdate >= SLOW_BLINKING_PERIOD)
            {
                ledState = !ledState;
                digitalWrite(STATUS_LED_PIN, ledState ? HIGH : LOW);
                lastStatusLedUpdate = now;
            }
            break;
        case STATUS_LED_CODES::NO_MQTT:
            if (now - lastStatusLedUpdate >= FAST_BLINKING_PERIOD)
            {
                ledState = !ledState;
                digitalWrite(STATUS_LED_PIN, ledState ? HIGH : LOW);
                lastStatusLedUpdate = now;
            }
            break;
        case STATUS_LED_CODES::NONE:
        default:
            digitalWrite(STATUS_LED_PIN, HIGH);
            ledState = false;
            break;
        }
    }

} // namespace

void statusLedSetCode(STATUS_LED_CODES code)
{
    if (code == statusLedCode)
    {
        return; // No change in status LED code
    }
    LOG_INFO("Setting status LED code: %d\n", static_cast<int>(code));
    statusLedCode = code;
}

void ioTask(void *pvParameters)
{
    ledInit(); // Initialize the LED pins
    ioInit();
    for (;;)
    {
        ioUpdate();
        ledUpdate();
        statusLedUpdate();
        vTaskDelay(10); // Delay to allow other tasks to run
    }
}