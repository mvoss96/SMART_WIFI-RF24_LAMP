#include "config.h"
#ifdef RF24RADIO_ENABLED

#include "radio.h"
#include "chipID.h"
#include "logging.h"
#include "radioMessage.h"
#include "ledControl.h"

#include <Arduino.h>
#include <Preferences.h>
#include <RF24.h>
#include <WiFi.h>

static RF24 radio(PIN_RADIO_CE, PIN_RADIO_CSN);
static Preferences preferences;
std::unordered_map<uint32_t, Remote> seenRemotes;

static bool radioInitialized = false;
static volatile bool _radioMsgReceived = false;
static const auto RADIO_DATARATE = RF24_250KBPS;
static char radioAddressStr[] = "00:00:00:00:00";

// Callback function pointer for New Remote Event
static void (*radioCallback)(void) = NULL;

IRAM_ATTR void setRadioCallback(void (*callback)(void))
{
    radioCallback = callback;
}

struct RadioSettings
{
    uint8_t channel;
    uint8_t radioAddress[5];
};
RadioSettings radioSettings;

IRAM_ATTR static void radioInterrupt()
{
    bool tx_ds, dx_df, rx_dr; // tx_ds = data sent, dx_df = data failed, rx_dr = data ready
    _radioMsgReceived = true;
}

static void loadRadioSettings()
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    preferences.begin("radio_config", false);
    radioSettings.channel = preferences.getInt("channel", 100);
    radioSettings.radioAddress[0] = preferences.getUChar("radioAddress0", mac[1]);
    radioSettings.radioAddress[1] = preferences.getUChar("radioAddress1", mac[2]);
    radioSettings.radioAddress[2] = preferences.getUChar("radioAddress2", mac[3]);
    radioSettings.radioAddress[3] = preferences.getUChar("radioAddress3", mac[4]);
    radioSettings.radioAddress[4] = preferences.getUChar("radioAddress4", mac[5]);
    preferences.end();
    LOG_INFO("Loaded Radio settings: Channel: %i, Radio Address: %02X:%02X:%02X:%02X:%02X\n",
             radioSettings.channel, radioSettings.radioAddress[0], radioSettings.radioAddress[1], radioSettings.radioAddress[2], radioSettings.radioAddress[3], radioSettings.radioAddress[4]);
}

void radioInit()
{
    loadRadioSettings(); // Load the radio settings
    if (!radio.begin())
    {
        LOG_ERROR("RF24Radio Connection Error!\n");
        return;
    }

    pinMode(PIN_RADIO_IRQ, INPUT);
    // let IRQ pin only trigger on "data_ready" event
    radio.maskIRQ(true, true, false); // args = "data_sent", "data_fail", "data_ready"
    attachInterrupt(digitalPinToInterrupt(PIN_RADIO_IRQ), radioInterrupt, FALLING);

    radio.setChannel(radioSettings.channel);              // Set the channel
    radio.setPALevel(RF24_PA_LOW);                        // Adjust power level
    radio.setAddressWidth(5);                             // Set address width
    radio.setCRCLength(RF24_CRC_16);                      // Set CRC length
    radio.setRetries(5, 15);                              // Set the number of retries and delay between retries
    radio.enableDynamicPayloads();                        // Enable dynamic payloads
    radio.setDataRate(RADIO_DATARATE);                    // Set data rate
    radio.openReadingPipe(1, radioSettings.radioAddress); // Open a reading pipe on address, using pipe 1 as an example
    radio.startListening();                               // Start listening

    LOG_INFO("RF24Radio initialized!\n");
    radioInitialized = true;
}

bool radioIsInitialized()
{
    return radioInitialized;
}

bool nrfListen(uint8_t *buf, uint8_t &packetSize)
{
    if (radio.available())
    {
        packetSize = radio.getDynamicPayloadSize();
        radio.read(buf, packetSize); // Read the data into the buffer
        return true;
    }

    return false; // No data available
}

static void logRadioPacket(uint8_t *buf, uint8_t &packetSize)
{
    char packetStr[128];
    for (int i = 0; i < packetSize; i++)
    {
        sprintf(packetStr + i * 3, "%02X ", buf[i]);
    }
    LOG_DEBUG("Received packet: %s\n", packetStr);
}

static void handleRemoteRadioMessage(RadioMessageReceived &msg)
{
    RemoteRadioMessageData remoteData(msg.getData(), msg.getDataSize());
    if (!remoteData.getValid())
    {
        LOG_WARNING("Skipping invalid remote message\n");
        return;
    }
    remoteData.print();

    // Store the remote data
    Remote remote;
    memcpy(remote.uuid, msg.getUUID(), sizeof(remote.uuid));
    remote.batteryPercentage = remoteData.getBatteryPercentage();
    remote.batteryVoltage = remoteData.getBatteryVoltage();
    const uint32_t uuid = *((const uint32_t *)(remote.uuid));

    // Check if the remote is new
    if (seenRemotes.find(uuid) == seenRemotes.end())
    {
        if (radioCallback)
        {
            radioCallback();
        }
    }
    seenRemotes[uuid] = remote;

    // Handle the remote event
    switch (remoteData.getEvent())
    {
    case RemoteEvents::ON:
    {
        LOG_DEBUG("Remote ON event\n");
        setLedPower(true);
        break;
    }
    case RemoteEvents::OFF:
    {
        LOG_DEBUG("Remote OFF event\n");
        setLedPower(false);
        break;
    }
    case RemoteEvents::TOGGLE:
    {
        LOG_DEBUG("Remote TOGGLE event\n");
        toggleLedPower();
        break;
    }
    case RemoteEvents::UP1:
    {
        LOG_DEBUG("Remote UP1 event\n");
        increaseLedBrightness();
        break;
    }
    case RemoteEvents::DOWN1:
    {
        LOG_DEBUG("Remote DOWN1 event\n");
        decreaseLedBrightness();
        break;
    }
    case RemoteEvents::UP2:
    {
        LOG_DEBUG("Remote UP2 event\n");
        switch (LED_MODE)
        {
        case LED_MODES::CCT:
        {
            increaseLedColor();
            break;
        }
        default:
            increaseLedBrightness();
        }
        break;
    }
    case RemoteEvents::DOWN2:
    {
        LOG_DEBUG("Remote DOWN2 event\n");
        switch (LED_MODE)
        {
        case LED_MODES::CCT:
        {
            decreaseLedColor();
            break;
        }
        default:
        {
            decreaseLedBrightness();
        }
        }
        break;
    }
    }
}

static void handleRadioPacket(uint8_t *buf, uint8_t &packetSize)
{
    // Handle the received packet
    // logRadioPacket(buf, packetSize);
    RadioMessageReceived radioMessage(buf, packetSize);
    // radioMessage.print();
    MessageTypes msgType = radioMessage.getMsgType();
    switch (msgType)
    {
    case MessageTypes::REMOTE:
    {
        handleRemoteRadioMessage(radioMessage);
        break;
    }
    default:
    {
        LOG_WARNING("Unknown message type: %i\n", (uint8_t)msgType);
        break;
    }
    }
}

void radioLoop()
{
    uint8_t buf[32];
    uint8_t packetSize = 0;
    if (radioInitialized)
    {
        if (_radioMsgReceived)
        {
            _radioMsgReceived = false;
            while (nrfListen(buf, packetSize))
            {
                handleRadioPacket(buf, packetSize);
            }
        }
    }
}

char *getRadioAddressString()
{
    // return radio address in format "XX:XX:XX:XX:XX"
    sprintf(radioAddressStr, "%02X:%02X:%02X:%02X:%02X", radioSettings.radioAddress[0], radioSettings.radioAddress[1], radioSettings.radioAddress[2], radioSettings.radioAddress[3], radioSettings.radioAddress[4]);
    return radioAddressStr;
}

uint8_t getRadioChannel()
{
    return radioSettings.channel;
}

void setRadioSettings(uint8_t channel, const char *radioAddress)
{
    radioSettings.channel = channel;
    sscanf(radioAddress, "%02X:%02X:%02X:%02X:%02X", &radioSettings.radioAddress[0], &radioSettings.radioAddress[1], &radioSettings.radioAddress[2], &radioSettings.radioAddress[3], &radioSettings.radioAddress[4]);
    LOG_INFO("Radio settings updated: Channel: %i, Radio Address: %02X:%02X:%02X:%02X:%02X\n",
             radioSettings.channel, radioSettings.radioAddress[0], radioSettings.radioAddress[1], radioSettings.radioAddress[2], radioSettings.radioAddress[3], radioSettings.radioAddress[4]);

    // Save the radio settings
    preferences.begin("radio_config", false);
    preferences.putInt("channel", radioSettings.channel);
    preferences.putUChar("radioAddress0", radioSettings.radioAddress[0]);
    preferences.putUChar("radioAddress1", radioSettings.radioAddress[1]);
    preferences.putUChar("radioAddress2", radioSettings.radioAddress[2]);
    preferences.putUChar("radioAddress3", radioSettings.radioAddress[3]);
    preferences.putUChar("radioAddress4", radioSettings.radioAddress[4]);
    preferences.end();

    // Restart the radio
    radio.stopListening();
    delay(100);
    radioInit();
}

RemoteMap &getRemoteMap()
{
    return seenRemotes;
}

// radio task
void radioTask(void *pvParameters)
{
    radioInit(); // Initialize the RF radio
    unsigned long radioWatchdogTimer = millis();
    for (;;)
    {
        radioLoop();

        #ifdef RF24RADIO_WATCHDOG_ENABLED
        // RF24 Radio can become unresponsive after a while, so we need to reset it
        // Watchdog triggers every 30 seconds after the last received message
        if (millis() - radioWatchdogTimer > 30000)
        {
            LOG_INFO("Radio watchdog\n");
            radioInit();
            radioWatchdogTimer = millis();
        }
        #endif

        vTaskDelay(10); // Delay to allow other tasks to run
    }
}

#endif
