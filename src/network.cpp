#include <Arduino.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include "chipID.h"
#include "config.h"
#include "network.h"
#include "mqtt.h"
#include "radio.h"
#include "logging.h"

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
static WiFiManagerParameter customRadioChannel("radioChannel", "Radio Channel (0 -> 125)", String(getRadioChannel()).c_str(), 3);
static WiFiManagerParameter customRadioAddress("radioAddress", "Radio Address (00:00:00:00:00)", getRadioAddressString(), sizeof("00:00:00:00:00"));


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
    setRadioSettings(atoi(customRadioChannel.getValue()), customRadioAddress.getValue());
    wifiManager.setTitle(getDeviceName());
    //ESP.restart(); // Restart the device to apply the new settings
}

void wifiInit()
{
    const char *chipID = ChipID::getChipID();
    WiFi.hostname(chipID);
    WiFi.begin(); // Start WiFi connection
    WiFi.setTxPower(WIFI_POWER_8_5dBm); //Reduce WIFI poweer for copmpatibility with some devices
    int txPower = WiFi.getTxPower();
    mqttInit(); // Initialize MQTT settings and load settings from preferences

    // Load MQTT settings into WifiManager
    custom_device_name.setValue(getDeviceName(), 40);
    custom_mqtt_server.setValue(mqttSettings.server, 40);
    custom_mqtt_port.setValue(String(mqttSettings.port).c_str(), 6);
    custom_mqtt_username.setValue(mqttSettings.username, 40);
    custom_mqtt_password.setValue(mqttSettings.password, 40);
    custom_mqtt_topic.setValue(mqttSettings.topic, 40);
    customRadioChannel.setValue(String(getRadioChannel()).c_str(), 3);
    customRadioAddress.setValue(getRadioAddressString(), sizeof("00:00:00:00:00"));

    wifiManager.setTitle(getDeviceName());
    wifiManager.addParameter(&custom_device_name);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_username);
    wifiManager.addParameter(&custom_mqtt_password);
    wifiManager.addParameter(&custom_mqtt_topic);
    wifiManager.addParameter(&customRadioChannel);
    wifiManager.addParameter(&customRadioAddress);
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
    // Check and handle WiFi connection status
    if (WiFi.status() == WL_CONNECTED && !wifiStarted)
    {
        LOG_INFO("Connected to %s with IP %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
        ArduinoOTA.begin();
        wifiManager.startWebPortal(); // Start the WiFi portal if WiFi is connected and not yet started
        wifiStarted = true;           // Mark the WiFi as started
    }
    else if (WiFi.status() == WL_DISCONNECTED && millis() - wifiReconnectTimer > WIFI_RECONNECT_ATTEMPT_INTERVAL && wifiManager.getWiFiIsSaved())
    {
        wifiManager.setEnableConfigPortal(false); // Disable the configuration portal
        wifiManager.autoConnect(getDeviceName());      // Attempt to automatically connect to WiFi
        wifiReconnectTimer = millis();            // Reset the timer after a connection attempt
    }
    else if (wifiStarted && !wifiManager.getConfigPortalActive())
    {
        wifiManager.startWebPortal(); //  Make sure the config portal stays active
    }
    
    ArduinoOTA.handle();
    wifiManager.process(); // Process WiFiManager tasks
}

void wifiLoop()
{
    handleWiFiConnection(); // Handle WiFi connectivity and reconnection
    if (getMqttEnabled() && WiFi.status() == WL_CONNECTED)
    {
        handleMQTTConnection(); // Handle MQTT connection and publishing
    }
}
