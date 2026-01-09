#pragma once

enum class STATUS_LED_CODES
{
    NONE,
    STARTUP,
    NO_WIFI,
    NO_MQTT,
};

void statusLedSetCode(STATUS_LED_CODES code);
void ioTask(void *pvParameters);