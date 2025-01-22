#pragma once
#include "config.h"
#ifdef RF24RADIO_ENABLED

#include <cstdint>
#include <unordered_map>

struct Remote
{
    uint8_t uuid[4];
    uint8_t batteryPercentage;
    uint16_t batteryVoltage;
};

using RemoteMap = std::unordered_map<uint32_t, Remote>;

void setRadioSettings(uint8_t channel, const char *radioAddress);
bool radioIsInitialized();
char* getRadioAddressString();
uint8_t getRadioChannel(); 
RemoteMap& getRemoteMap();
void setRadioCallback(void (*callback)(void));
void radioTask(void *pvParameters);
#endif