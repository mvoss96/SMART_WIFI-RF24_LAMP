#include <WiFi.h>
#include "chipID.h"
#include "config.h"

const char *ChipID::getChipID()
{
    static char chipIdStr[32] = ""; // Inline static variable
    static bool initialized = false;

    if (!initialized)
    {
        String macAddress = WiFi.macAddress();
        Serial.printf("ESP32 Chip ID = %s\n", macAddress.c_str());
        macAddress.replace(":", "");
        char modifiedMac[13];
        macAddress.substring(6, 17).toCharArray(modifiedMac, sizeof(modifiedMac));
        sprintf(chipIdStr, "%s-%s", DEVICENAME, modifiedMac);
        initialized = true;
    }

    return chipIdStr;
}