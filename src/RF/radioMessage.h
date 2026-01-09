#pragma once
#include "config.h"
#ifdef RF24RADIO_ENABLED

#include <Arduino.h>

enum class MessageTypes : uint8_t
{
    EMPTY,
    REMOTE,
};

enum class RemoteEvents : uint8_t
{
    EMPTY,
    ON,
    OFF,
    TOGGLE,
    UP1,
    DOWN1,
    UP2,
    DOWN2,
};

class RadioMessageReceived
{
protected:
    uint8_t PROTOCOL_VERSION = 0;
    uint8_t UUID[4] = {0};
    uint8_t MSG_NUM = 0;
    MessageTypes MSG_TYPE = MessageTypes::EMPTY;
    uint8_t DATA[25] = {0}; // Last two bytes are checksum

    bool valid = false;
    size_t dataSize = 0;
    void checkChecksum();

public:
    RadioMessageReceived(uint8_t *data, size_t size);
    void print();
    size_t getDataSize();
    uint8_t *getData();
    uint8_t getProtocolVersion();
    uint8_t *getUUID();
    uint8_t getMsgNum();
    MessageTypes getMsgType();
    bool getValid();
};

class RemoteRadioMessageData
{
private:
    RemoteEvents EVENT = RemoteEvents::EMPTY;
    uint8_t BATTERY_PERCENTAGE = 0;
    uint16_t BATTERY_VOLTAGE_MV = 0;
    bool valid = false;

public:
    RemoteRadioMessageData(uint8_t *data, size_t size);
    RemoteEvents getEvent();
    uint8_t getBatteryPercentage();
    uint16_t getBatteryVoltage();
    bool getValid();
    void print();
};

#endif