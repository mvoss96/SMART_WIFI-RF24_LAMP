#include "config.h"

#include "network.h"
#include "mqtt.h"
#include "RF/radio.h"
#include "Logging/logging.h"
#include "ChipID/chipID.h"
#include "Output/ioControl.h"

#include <Arduino.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

char chipIdStr[32];
bool wifiStarted = false;
static long wifiReconnectTimer = 0;

static WiFiManager wifiManager;
static WiFiManagerParameter custom_device_name("deviceName", "Device Name", getDeviceName(), 40);
static WiFiManagerParameter custom_mqtt_server("mqttServer", "MQTT Server", mqttSettings.server, 40);
static WiFiManagerParameter custom_mqtt_port("mqttPort", "MQTT Port", String(mqttSettings.port).c_str(), 6);
static WiFiManagerParameter custom_mqtt_username("mqttUsername", "MQTT Username", mqttSettings.username, 40);
static WiFiManagerParameter custom_mqtt_password("mqttPassword", "MQTT Password", mqttSettings.password, 40);
static WiFiManagerParameter custom_mqtt_topic("mqttTopic", "MQTT Base Topic", mqttSettings.topic, 40);
#ifdef RF24RADIO_ENABLED
static WiFiManagerParameter customRadioChannel("radioChannel", "Radio Channel (0 -> 125)", String(getRadioChannel()).c_str(), 3);
static WiFiManagerParameter customRadioAddress("radioAddress", "Radio Address (00:00:00:00:00)", getRadioAddressString(), sizeof("00:00:00:00:00"));
#endif

const String translateWiFiStatus(wl_status_t status)
{
    {
        switch (status)
        {
        case WL_NO_SHIELD:
            return "No shield";
        case WL_IDLE_STATUS:
            return "Idle";
        case WL_NO_SSID_AVAIL:
            return "No ssid";
        case WL_SCAN_COMPLETED:
            return "Scan Completed";
        case WL_CONNECTED:
            return "Connected";
        case WL_CONNECT_FAILED:
            return "Connect failed";
        case WL_CONNECTION_LOST:
            return "Connection lost";
        case WL_DISCONNECTED:
            return "Disconnected";
        default:
            return "N/A";
        }
    }
}

void saveParamsCallback()
{
    setDeviceName(custom_device_name.getValue());
    setMqttSettings(custom_mqtt_server.getValue(), atoi(custom_mqtt_port.getValue()), custom_mqtt_username.getValue(), custom_mqtt_password.getValue(), custom_mqtt_topic.getValue());
#ifdef RF24RADIO_ENABLED
    setRadioSettings(atoi(customRadioChannel.getValue()), customRadioAddress.getValue());
#endif
    // wifiManager.setTitle(getDeviceName());
    delay(100);
    ESP.restart(); // Restart the device to apply the new settings
}

void networkInit()
{
    const char *chipID = ChipID::getChipID();
    WiFi.hostname(chipID);
    WiFi.begin();                       // Start WiFi connection
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // Reduce WIFI poweer for copmpatibility with some devices
    int txPower = WiFi.getTxPower();
    mqttInit(); // Initialize MQTT settings and load settings from preferences

    // Load MQTT settings into WifiManager
    custom_device_name.setValue(getDeviceName(), 40);
    custom_mqtt_server.setValue(mqttSettings.server, 40);
    custom_mqtt_port.setValue(String(mqttSettings.port).c_str(), 6);
    custom_mqtt_username.setValue(mqttSettings.username, 40);
    custom_mqtt_password.setValue(mqttSettings.password, 40);
    custom_mqtt_topic.setValue(mqttSettings.topic, 40);
#ifdef RF24RADIO_ENABLED
    customRadioChannel.setValue(String(getRadioChannel()).c_str(), 3);
    customRadioAddress.setValue(getRadioAddressString(), sizeof("00:00:00:00:00"));
#endif

    wifiManager.setTitle(String(getDeviceName()) + " (" + SW_VERSION + ")");
    wifiManager.addParameter(&custom_device_name);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_username);
    wifiManager.addParameter(&custom_mqtt_password);
    wifiManager.addParameter(&custom_mqtt_topic);
#ifdef RF24RADIO_ENABLED
    wifiManager.addParameter(&customRadioChannel);
    wifiManager.addParameter(&customRadioAddress);
#endif
    wifiManager.setConnectTimeout(10);
    wifiManager.setParamsPage(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setSaveParamsCallback(saveParamsCallback);
    wifiManager.setEnableConfigPortal(false);
    const char *menu[] = {"wifi", "param", "info"};
    wifiManager.setMenu(menu, 3);
    wifiManager.setClass("invert");
    delay(1000); // Wait for WiFi to initialize

    // Start WiFiManager if no WiFi credentials are saved
    if (wifiManager.getWiFiIsSaved())
    {
        LOG_INFO("Connecting to WiFi\n");
        wifiManager.setEnableConfigPortal(false); // Disable config portal so that it doesn't start when connection fails
        wifiManager.autoConnect();
    }
    else
    {
        LOG_INFO("Starting AP\n");
        wifiManager.setEnableConfigPortal(true);
        wifiManager.startConfigPortal(chipID);
    }
}

void handleWiFiConnection()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        // If WiFi is connected
        if (!wifiStarted)
        {
            // Initial setup when WiFi connects for the first time
            LOG_INFO("Connected to %s with IP %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
            ArduinoOTA.begin();           // Start OTA updates
            wifiManager.startWebPortal(); // Start the WiFi portal
            wifiStarted = true;           // Mark WiFi as started
        }
        else if (!wifiManager.getWebPortalActive())
        {
            // Ensure the configuration portal remains active
            LOG_INFO("Starting web portal\n");
            wifiManager.startWebPortal();
        }
        ArduinoOTA.handle(); // Handle OTA updates
    }
    else
    {
        // If WiFi is not connected
        if (millis() - wifiReconnectTimer > WIFI_RECONNECT_ATTEMPT_INTERVAL && wifiManager.getWiFiIsSaved())
        {
            // Attempt to reconnect if the reconnection interval has passed
            LOG_INFO("Attempting to reconnect to WiFi\n");
            wifiManager.setEnableConfigPortal(false); // Disable the configuration portal
            wifiManager.autoConnect(getDeviceName()); // Attempt to reconnect to WiFi
            wifiReconnectTimer = millis();            // Reset the reconnect timer
        }
    }

#ifdef DEBUG_WIFI_STATUS_INTERVAL
    // Print WiFi status every 10 seconds
    static long lastTime = 0;
    if (DEBUG_WIFI_STATUS_INTERVAL > 0 && millis() - lastTime > DEBUG_WIFI_STATUS_INTERVAL)
    {
        lastTime = millis();
        LOG_INFO("---------------------WIFI-STATUS------------------\n");
        LOG_INFO("wifiStarted: %d, getConfigPortalActive: %d wifiReconnectTimer %d, wifiManager.getWiFiIsSaved: %d\n", wifiStarted, wifiManager.getConfigPortalActive(), wifiReconnectTimer, wifiManager.getWiFiIsSaved());
        LOG_INFO("WiFi status: %s\n", translateWiFiStatus(WiFi.status()).c_str());
        LOG_INFO("WiFi SSID: %s\n", WiFi.SSID().c_str());
        LOG_INFO("WiFi IP: %s\n", WiFi.localIP().toString().c_str());
        LOG_INFO("--------------------------------------------------\n");
    }
#endif

    wifiManager.process(); // Process WiFiManager tasks
}

void networkLoop()
{
    handleWiFiConnection(); // Handle WiFi connectivity and reconnection

    bool wifiConnected = getNetworkConnected();
    bool mqttEnabled = getMqttEnabled();
    
    if (mqttEnabled && wifiConnected)
    {
        handleMQTTConnection(); // Handle MQTT connection and publishing
    }

    if (!wifiConnected)
    {
        statusLedSetCode(STATUS_LED_CODES::NO_WIFI);
    }
    else if (!getMQTTConnected())
    {
        statusLedSetCode(STATUS_LED_CODES::NO_MQTT);
    }
    else
    {
        statusLedSetCode(STATUS_LED_CODES::NONE);
    }
}

bool getNetworkConnected()
{
    return (WiFi.status() == WL_CONNECTED);
}
