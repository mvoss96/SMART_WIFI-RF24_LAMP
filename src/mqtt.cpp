#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "logging.h"
#include "config.h"
#include "chipID.h"
#include "mqtt.h"
#include "ledControl.h"

MQTT_Settings mqttSettings;
static unsigned long mqttReconnectTimer = ULONG_MAX;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
static Preferences preferences;

void mqttHomeAssistandDiscovery()
{
}

void mqttConnect()
{
    // Make sure the connection is disconnected
    if (mqttClient.connected())
    {
        mqttClient.disconnect();
        delay(500);
    }

    mqttClient.setServer(mqttSettings.server, mqttSettings.port);
    delay(100);
    if (mqttClient.connect(ChipID::getChipID(), mqttSettings.username, mqttSettings.password))
    {
        mqttHomeAssistandDiscovery();
        LOG_INFO("MQTT connected\n");
    }
    else
    {
        LOG_INFO("MQTT connection failed, rc=%i\n", mqttClient.state());
    }
}

void mqttPublish()
{
    // Return if MQTT is not enabled or not connected
    if (!mqttClient.connected())
    {
        return;
    }

    JsonDocument doc;

    const char *mode = getLEDModeStr(LED_MODE);

    // Common Values for all modes
    doc["mode"] = mode;
    doc["power"] = ledSettings.power;
    doc["brightness"] = ledSettings.brightness;

    if (LED_MODE == LED_MODES::CCT)
    {
        doc["color"] = ledSettings.color;
    }
    else if (LED_MODE == LED_MODES::RGB || LED_MODE == LED_MODES::RGBW || LED_MODE == LED_MODES::RGBWW)
    {
        doc["red"] = ledSettings.red;
        doc["green"] = ledSettings.green;
        doc["blue"] = ledSettings.blue;
    }

    if (LED_MODE == LED_MODES::RGBW || LED_MODE == LED_MODES::RGBWW)
    {
        doc["ww"] = ledSettings.ww;
    }

    if (LED_MODE == LED_MODES::RGBWW)
    {
        doc["cw"] = ledSettings.cw;
    }

    char payload[128];
    serializeJson(doc, payload, sizeof(payload));
    char topic[sizeof(mqttSettings.topic) + sizeof("/light")];
    snprintf(topic, sizeof(topic), "%s/light", mqttSettings.topic);
    log(LOG_LEVEL::INFO, "Publish MQTT message on topic %s with payload: %s\n", topic, payload);
    mqttClient.publish(topic, payload);
}

void handleMQTTConnection()
{
    // Return if MQTT is not enabled
    if (!getMqttEnabled())
    {
        return;
    }

    // Attempt MQTT reconnection if disconnected and interval has passed
    if (!mqttClient.connected() && millis() - mqttReconnectTimer > MQTT_RECONNECT_ATTEMPT_INTERVAL)
    {
        mqttConnect();                 // Establish MQTT connection
        mqttReconnectTimer = millis(); // Reset the timer after a connection attempt
    }

    static unsigned long lastPublishTime = 0;
    if (millis() - lastPublishTime >= MQTT_PUBLISH_INTERVAL)
    {
        mqttPublish(); // Publish data to MQTT
        lastPublishTime = millis();
    }
    mqttClient.loop(); // Allow MQTT client to process incoming and outgoing messages
}

bool getMqttEnabled()
{
    bool enabled = (strlen(mqttSettings.server) > 0 && mqttSettings.port > 0);
    return enabled;
}

void printMqttConfig()
{
    LOG_INFO("MQTT Config: Server: %s Port: %i Username: %s Password: %s Topic: %s\n",
              mqttSettings.server, mqttSettings.port, mqttSettings.username, mqttSettings.password, mqttSettings.topic);
}

void saveMqttSettings()
{
    LOG_INFO("Saving MQTT settings\n");
    preferences.begin("mqtt_config", false);
    preferences.putString("mqttServer", mqttSettings.server);
    preferences.putString("mqttUsername", mqttSettings.username);
    preferences.putString("mqttPassword", mqttSettings.password);
    preferences.putString("mqttTopic", mqttSettings.topic);
    preferences.putInt("mqttPort", mqttSettings.port);
    preferences.end();
}

void setMqttSettings(const char *server, const unsigned int port, const char *username, const char *password, const char *topic)
{
    strcpy(mqttSettings.server, server);
    mqttSettings.port = port;
    strcpy(mqttSettings.username, username);
    strcpy(mqttSettings.password, password);
    strcpy(mqttSettings.topic, topic);
    LOG_INFO("MQTT settings updated\n");
    printMqttConfig();
    saveMqttSettings();
}

void loadMQTTsettings()
{
    LOG_INFO("Loading MQTT settings\n");
    preferences.begin("mqtt_config", false);
    strcpy(mqttSettings.server, preferences.getString("mqttServer", "").c_str());
    mqttSettings.port = preferences.getInt("mqttPort", 1883);
    strcpy(mqttSettings.username, preferences.getString("mqttUsername", "").c_str());
    strcpy(mqttSettings.password, preferences.getString("mqttPassword", "").c_str());
    strcpy(mqttSettings.topic, preferences.getString("mqttTopic", ChipID::getChipID()).c_str());
    preferences.end();
    printMqttConfig();
}

void mqttInit()
{
    loadMQTTsettings();
    mqttClient.setBufferSize(512);
}
