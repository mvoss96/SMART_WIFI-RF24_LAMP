// functions to communicate with the nrf24l01 radio module
#include <Arduino.h>
#include <Preferences.h>
#include <RF24.h>

#include "radio.h"
#include "config.h"
#include "logging.h"

static RF24 radio(PIN_RADIO_CE, PIN_RADIO_CSN);
static Preferences preferences;

static bool radioInitialized = false;
static bool _radioInterruptReceived = false;
static const auto RADIO_DATARATE = RF24_250KBPS;

struct RadioSettings
{
    uint8_t channel;
    uint8_t radioAddress[5];
};

RadioSettings radioSettings;

static void radioInterrupt()
{
    _radioInterruptReceived = true;
}

static void printRadioSettings()
{
    LOG_INFO("Radio settings: Channel: %i, Address: %c%c%c%c%i\n",
             radioSettings.channel, radioSettings.radioAddress[0], radioSettings.radioAddress[1],
             radioSettings.radioAddress[2], radioSettings.radioAddress[3], radioSettings.radioAddress[4]);
}

static void loadRadioSettings()
{
    LOG_INFO("Loading Radio settings\n");
    preferences.begin("radio_config", false);
    radioSettings.channel = preferences.getInt("channel", 100);
    radioSettings.radioAddress[0] = preferences.getUChar("radioAddress0", 'R');
    radioSettings.radioAddress[1] = preferences.getUChar("radioAddress1", 'F');
    radioSettings.radioAddress[2] = preferences.getUChar("radioAddress2", '2');
    radioSettings.radioAddress[3] = preferences.getUChar("radioAddress3", '4');
    radioSettings.radioAddress[4] = preferences.getUChar("radioAddress4", 0);
    preferences.end();
    printRadioSettings();
}

void radioInit()
{
    if (!radio.begin())
    {
        LOG_ERROR("RF24Radio Connection Error!\n");
        return;
    }

    pinMode(PIN_RADIO_IRQ, INPUT);
    // let IRQ pin only trigger on "data_ready" event
    radio.maskIRQ(true, true, false); // args = "data_sent", "data_fail", "data_ready"
    attachInterrupt(digitalPinToInterrupt(PIN_RADIO_IRQ), radioInterrupt, FALLING);

    // Load the radio settings
    loadRadioSettings();

    // Set the radio address
    // radioAddress[4] = address;

    radio.setChannel(radioSettings.channel);              // Set the channel
    radio.setPALevel(RF24_PA_LOW);                        // Adjust power level
    radio.setAddressWidth(5);                             // Set address width
    radio.setCRCLength(RF24_CRC_16);                      // Set CRC length
    radio.setRetries(5, 15);                              // Set the number of retries and delay between retries
    radio.enableDynamicPayloads();                        // Enable dynamic payloads
    radio.enableAckPayload();                             // Enable ack payloads
    radio.setDataRate(RADIO_DATARATE);                    // Set data rate
    radio.openReadingPipe(1, radioSettings.radioAddress); // Open a reading pipe on address, using pipe 1 as an example
    radio.openWritingPipe(radioSettings.radioAddress);    // Set the writing pipe address
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
    if (_radioInterruptReceived)
    {
        _radioInterruptReceived = false;         // Reset the flag
        bool tx_ds, dx_df, rx_dr;                // tx_ds = data sent, dx_df = data failed, rx_dr = data ready
        radio.whatHappened(tx_ds, dx_df, rx_dr); // Reset the IRQ pin to High, allow for calling of available()
        if (rx_dr && radio.available())
        {
            packetSize = radio.getDynamicPayloadSize();
            radio.read(buf, packetSize); // Read the data into the buffer
            return true;
        }
    }

    return false; // No data available
}

static void handleRadioPacket(uint8_t *buf, uint8_t &packetSize)
{
}

static void logRadioPacket(uint8_t *buf, uint8_t &packetSize)
{
    char receivedDataStr[96] = {0};
    for (int i = 0; i < packetSize; i++)
    {
        char hex[3];
        sprintf(hex, "%02X", buf[i]);
        strcat(receivedDataStr, hex);
        if (i < packetSize - 1)
        {
            strcat(receivedDataStr, ":");
        }
    }
    strcat(receivedDataStr, "\n");
    LOG_DEBUG("Received radio packet of size %i: %s\n", packetSize, receivedDataStr);
}

void radioLoop()
{
    static uint8_t buf[32];
    if (radioInitialized)
    {
        uint8_t packetSize = 0;
        if (nrfListen(buf, packetSize))
        {
            logRadioPacket(buf, packetSize);
            handleRadioPacket(buf, packetSize);
        }
    }
}