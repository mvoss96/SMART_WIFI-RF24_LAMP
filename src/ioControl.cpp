#include "config.h"
#include "ioControl.h"
#include "ledControl.h"

#include <Arduino.h>

namespace
{
    const unsigned int debounceTime = 30;
    const unsigned int holdInterval = 150;
    unsigned long lastStatusLedUpdate = 0;
    STATUS_LED_CODES statusLedCode{STATUS_LED_CODES::NONE};

    class Button
    {
    private:
        bool dimmingDirection{true};
        unsigned int pin{};
        bool activeHigh{};
        bool state{};
        bool last_state{};
        unsigned long holdCounter{};
        BUTTON_BEHAVIOR behavior{};
        unsigned long lastChangeTime{};
        unsigned long lastHoldTime{};

        void actClick()
        {
            // Serial.println("Click");
            switch (behavior)
            {
            case BUTTON_BEHAVIOR::TOGGLE:
            case BUTTON_BEHAVIOR::DIMMER:
                toggleLedPower();
                break;
            default:
                break;
            }
        }

        void actHold(unsigned long counter)
        {
            // Serial.println("Hold");
            switch (behavior)
            {
            case BUTTON_BEHAVIOR::TOGGLE:
                if (holdCounter == 1)
                {
                    toggleLedPower();
                }
                break;
            case BUTTON_BEHAVIOR::DIMMER:
                if (dimmingDirection)
                {
                    increaseLedBrightness();
                }
                else
                {
                    decreaseLedBrightness();
                }
                break;
            default:
                break;
            }
        }

    public:
        Button(unsigned int pin, bool activeHigh, BUTTON_BEHAVIOR behavior)
        {
            this->pin = pin;
            this->activeHigh = activeHigh;
            this->behavior = behavior;
            pinMode(pin, activeHigh ? INPUT_PULLDOWN : INPUT_PULLUP);
        }

        void update()
        {
            // Read the raw state of the button
            bool current_state = digitalRead(pin) == activeHigh;

            // Detect a change in the raw state
            if (current_state != last_state)
            {
                lastChangeTime = millis(); // Reset debounce timer
            }

            // Check if the state is stable beyond the debounce time
            if ((millis() - lastChangeTime) > debounceTime)
            {
                // Confirm a state change
                if (current_state != state)
                {
                    state = current_state; // Update the stable state
                    if (state)
                    {
                        lastHoldTime = millis();                              // Start tracking hold time
                        dimmingDirection = getLedBrightness() != LED_MAX_VAL; // Set the dimming direction
                        holdCounter = 0;                                      // Reset the hold counter
                    }
                    else if (holdCounter == 0)
                    {
                        actClick(); // Perform the click action
                    }
                }
            }

            // Handle button hold logic
            if (state)
            { // Button is currently pressed
                unsigned long now = millis();
                if (now - lastHoldTime >= holdInterval)
                {
                    holdCounter++;        // Increment the hold counter
                    actHold(holdCounter); // Perform the hold action
                    lastHoldTime = now;   // Reset the hold interval timer
                }
            }

            // Update the last raw state for the next cycle
            last_state = current_state;
        }
    };

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
            digitalWrite(STATUS_LED_PIN, HIGH);
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
            digitalWrite(STATUS_LED_PIN, LOW);
            ledState = false;
            break;
        }
    }

} // namespace

void statusLedSetCode(STATUS_LED_CODES code)
{
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