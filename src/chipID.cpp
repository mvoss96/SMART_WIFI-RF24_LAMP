#include <WiFi.h>
#include "chipID.h"
#include "config.h"
#include "logging.h"

static char chipIdStr[32] = ""; // Inline static variable
static char modifiedMac[13];
static bool initialized = false;

const char *ChipID::getChipID()
{
    if (!initialized)
    {
        String macAddress = WiFi.macAddress();
        LOG_INFO("ESP32 Chip ID = %s\n", macAddress.c_str());
        macAddress.replace(":", "");
        macAddress.substring(6, 17).toCharArray(modifiedMac, sizeof(modifiedMac));
        sprintf(chipIdStr, "%s-%s", DEVICENAME, modifiedMac);
        initialized = true;
    }

    return chipIdStr;
}

const char* ChipID::getShortChipID()
{
    if (!initialized)
    {
        getChipID();
    }

    return modifiedMac;
}