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
        uint8_t mac[6];
        WiFi.macAddress(mac);
        LOG_INFO("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        snprintf(modifiedMac, sizeof(modifiedMac), "%02X%02X%02X", mac[3], mac[4], mac[5]);
        snprintf(chipIdStr, sizeof(chipIdStr), "%s-%s", DEVICENAME, modifiedMac);
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