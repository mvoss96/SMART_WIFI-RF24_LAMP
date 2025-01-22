#include "config.h"
#ifdef RF24RADIO_ENABLED

#include "radioMessage.h"
#include "logging.h"

RadioMessageReceived::RadioMessageReceived(uint8_t *data, size_t size)
{
    const size_t minSize = sizeof(PROTOCOL_VERSION) + sizeof(UUID) + sizeof(MSG_NUM) + sizeof(MSG_TYPE) + 2;
    if (size < minSize)
    {
        LOG_WARNING("RadioMessageReceived: size too small: %i\n", size);
        return;
    }
    if (size > 32)
    {
        LOG_WARNING("RadioMessageReceived: size too large: %i\n", size);
        return;
    }

    dataSize = size - minSize;
    PROTOCOL_VERSION = data[0];
    memcpy(UUID, data + 1, 4);
    MSG_NUM = data[5];
    MSG_TYPE = static_cast<MessageTypes>(data[6]);
    memcpy(DATA, data + (minSize - 2), dataSize + 2);
    checkChecksum();
}

void RadioMessageReceived::checkChecksum()
{
    uint16_t calculatedChecksum = 0;
    calculatedChecksum += PROTOCOL_VERSION;
    for (size_t i = 0; i < sizeof(UUID); i++)
    {
        calculatedChecksum += UUID[i];
    }
    calculatedChecksum += MSG_NUM;
    for (size_t i = 0; i < dataSize; i++)
    {
        calculatedChecksum += DATA[i];
    }

    uint16_t receivedChecksum = (static_cast<uint16_t>(DATA[dataSize + 1]) << 8) | DATA[dataSize];
    if (calculatedChecksum == receivedChecksum)
    {
        LOG_DEBUG("RadioMessageReceived: checksum match %04X\n", calculatedChecksum);
        valid = true;
    }
    else
    {
        valid = false;
        LOG_WARNING("RadioMessageReceived: checksum mismatch %04X != %04X\n", calculatedChecksum, receivedChecksum);
    }
}

void RadioMessageReceived::print()
{
    char output[LOG_BUFFER_SIZE];
    int offset = 0; // Offset to keep track of the current position in the output buffer.
    offset += snprintf(output + offset, sizeof(output) - offset, "RadioMessage: valid: %s PV: %X UUID: ", (valid ? "true" : "false"), PROTOCOL_VERSION);
    for (size_t i = 0; i < sizeof(UUID); i++)
    {
        offset += snprintf(output + offset, sizeof(output) - offset, "%X ", UUID[i]);
    }
    offset += snprintf(output + offset, sizeof(output) - offset, "MSG_NUM: %X MSG_TYPE: %X DATA: ", MSG_NUM, (uint8_t)MSG_TYPE);
    for (size_t i = 0; i < dataSize; i++)
    {
        offset += snprintf(output + offset, sizeof(output) - offset, "%X ", DATA[i]);
    }
    offset += snprintf(output + offset, sizeof(output) - offset, "Checksum: %X %X\n", DATA[dataSize], DATA[dataSize + 1]);
    LOG_INFO(output);
}

size_t RadioMessageReceived::getDataSize()
{
    return dataSize;
}

uint8_t *RadioMessageReceived::getData()
{
    return DATA;
}

uint8_t RadioMessageReceived::getProtocolVersion()
{
    return PROTOCOL_VERSION;
}

uint8_t *RadioMessageReceived::getUUID()
{
    return UUID;
}

uint8_t RadioMessageReceived::getMsgNum()
{
    return MSG_NUM;
}

MessageTypes RadioMessageReceived::getMsgType()
{
    return MSG_TYPE;
}

bool RadioMessageReceived::getValid()
{
    return valid;
}

#endif