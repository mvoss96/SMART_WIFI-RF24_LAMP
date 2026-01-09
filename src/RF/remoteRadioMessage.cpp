#include "config.h"

#ifdef RF24RADIO_ENABLED
#include "radioMessage.h"
#include "Logging/logging.h"

RemoteRadioMessageData::RemoteRadioMessageData(uint8_t *data, size_t size)
{
    if (size != 4)
    {
        LOG_WARNING("RemoteRadioMessageData: size Mismatch: %i\n", size);
        return;
    }
    valid = true;
    EVENT = static_cast<RemoteEvents>(data[0]);
    BATTERY_PERCENTAGE = data[1];
    BATTERY_VOLTAGE_MV = (data[3] << 8) | data[2];
}

RemoteEvents RemoteRadioMessageData::getEvent()
{
    return EVENT;
}

uint8_t RemoteRadioMessageData::getBatteryPercentage()
{
    return (BATTERY_PERCENTAGE * 100) / 255;
}

uint16_t RemoteRadioMessageData::getBatteryVoltage()
{
    return BATTERY_VOLTAGE_MV;
}

bool RemoteRadioMessageData::getValid()
{
    return valid;
}

void RemoteRadioMessageData::print()
{
    char output[LOG_BUFFER_SIZE];
    int offset = 0; // Offset to keep track of the current position in the output buffer.
    offset += snprintf(output + offset, sizeof(output) - offset,
                       "RemoteRadioMessageData: Valid: %s Event: %X Battery: %d %dmV\n",
                       (valid ? "true" : "false"), (uint8_t)getEvent(), getBatteryPercentage(), getBatteryVoltage());
    LOG_INFO(output);
}

#endif