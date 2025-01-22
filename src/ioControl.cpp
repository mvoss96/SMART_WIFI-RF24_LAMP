#include "config.h"
#include "ioControl.h"
#include "ledControl.h"

#include <Arduino.h>

namespace
{
    const unsigned int debounceTime = 30;
    const unsigned int holdInterval = 150;

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
            Serial.println("Click");
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
            Serial.println("Hold");
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
}

void ioInit()
{
#ifdef ENABLE_BUTTON1
    buttons.push_back(Button(BUTTON1_PIN, BUTTON1_ACTIVE_HIGH, BUTTON1_BEHAVIOR));
#endif
}

void ioUpdate()
{
    for (auto &button : buttons)
    {
        button.update();
    }
}

void ioTask(void *pvParameters)
{
    ledInit(); // Initialize the LED pins
    ioInit();
    for (;;)
    {
        ioUpdate();
        ledUpdate();
        vTaskDelay(10); // Delay to allow other tasks to run
    }
}