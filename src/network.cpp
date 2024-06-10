#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFi.h>

#include "chipID.h"
#include "config.h"
#include "network.h"
#include "mqtt.h"
#include "logging.h"

char chipIdStr[32];

bool wifiStarted = false;
static unsigned long wifiReconnectTimer = 0;

static WiFiManager wifiManager;
static WiFiManagerParameter custom_mqtt_server("mqttServer", "MQTT Server", mqttSettings.server, 40);
static WiFiManagerParameter custom_mqtt_port("mqttPort", "MQTT Port", String(mqttSettings.port).c_str(), 6);
static WiFiManagerParameter custom_mqtt_username("mqttUsername", "MQTT Username", mqttSettings.username, 40);
static WiFiManagerParameter custom_mqtt_password("mqttPassword", "MQTT Password", mqttSettings.password, 40);
static WiFiManagerParameter custom_mqtt_topic("mqttTopic", "MQTT Topic", mqttSettings.topic, 40);

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
    setMqttSettings(custom_mqtt_server.getValue(), atoi(custom_mqtt_port.getValue()), custom_mqtt_username.getValue(), custom_mqtt_password.getValue(), custom_mqtt_topic.getValue());
}

void wifiSetup()
{
    const char *chipID = ChipID::getChipID();
    WiFi.hostname(chipID);
    mqttInit(); // Initialize MQTT settings and load settings from preferences

    // Load MQTT settings into WifiManager
    custom_mqtt_server.setValue(mqttSettings.server, 40);
    custom_mqtt_port.setValue(String(mqttSettings.port).c_str(), 6);
    custom_mqtt_username.setValue(mqttSettings.username, 40);
    custom_mqtt_password.setValue(mqttSettings.password, 40);
    custom_mqtt_topic.setValue(mqttSettings.topic, 40);

    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_username);
    wifiManager.addParameter(&custom_mqtt_password);
    wifiManager.addParameter(&custom_mqtt_topic);
    wifiManager.setConnectTimeout(10);
    wifiManager.setParamsPage(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setSaveParamsCallback(saveParamsCallback);
    wifiManager.setEnableConfigPortal(false);
    const char *menu[] = {"wifi", "param", "info"};
    wifiManager.setMenu(menu, 3);
    wifiManager.setClass("invert");
    wifiManager.autoConnect();

    // Start WiFiManager if no WiFi credentials are saved
    if (wifiManager.getWiFiIsSaved())
    {
        Serial.println("Connecting to WiFi");
        wifiManager.setEnableConfigPortal(false); // Disable config portal so that it doesn't start when connection fails
        wifiManager.autoConnect();
    }
    else
    {
        Serial.println("Starting AP");
        wifiManager.setEnableConfigPortal(true);
        wifiManager.startConfigPortal(chipID);
    }
}

void handleWiFiConnection()
{
    // Check and handle WiFi connection status
    if (WiFi.status() == WL_CONNECTED && !wifiStarted)
    {
        wifiManager.startWebPortal(); // Start the WiFi portal if WiFi is connected and not yet started
        wifiStarted = true;           // Mark the WiFi as started
    }
    else if (WiFi.status() == WL_DISCONNECTED && millis() - wifiReconnectTimer > WIFI_RECONNECT_ATTEMPT_INTERVAL && wifiManager.getWiFiIsSaved())
    {
        wifiManager.setEnableConfigPortal(false); // Disable the configuration portal
        wifiManager.autoConnect(DEVICENAME);      // Attempt to automatically connect to WiFi
        wifiReconnectTimer = millis();            // Reset the timer after a connection attempt
    }
    else if (wifiStarted && !wifiManager.getConfigPortalActive())
    {
        wifiManager.startWebPortal(); //  Make sure the config portal stays active
    }

    wifiManager.process(); // Process WiFiManager tasks
}

void wifiLoop()
{
    handleWiFiConnection(); // Handle WiFi connectivity and reconnection
    handleMQTTConnection(); // Handle MQTT connection and publishing
}
