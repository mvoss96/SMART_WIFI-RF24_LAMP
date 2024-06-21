#pragma once

struct Remote
{
    uint8_t uuid[4];
    uint8_t batteryPercentage;
    uint16_t batteryVoltage;
};

using RemoteMap = std::unordered_map<uint32_t, Remote>;

void radioInit();
void radioLoop();
void setRadioSettings(uint8_t channel, const char *radioAddress);
bool radioIsInitialized();
char* getRadioAddressString();
uint8_t getRadioChannel(); 
RemoteMap& getRemoteMap();