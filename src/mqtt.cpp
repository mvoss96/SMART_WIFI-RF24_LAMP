#include "logging.h"
#include "config.h"
#include "chipID.h"
#include "mqtt.h"
#include "radio.h"
#include "ledControl.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>

MQTT_Settings mqttSettings;
static unsigned long mqttReconnectTimer = ULONG_MAX;

static WiFiClient espClient;
static PubSubClient mqttClient(espClient);
static Preferences preferences;
static bool ledStateChange = false;
#ifdef RF24RADIO_ENABLED
static bool newRadioSeen = false;
#endif
static char deviceTopic[64];

static char *getDecviceTopic()
{
    snprintf(deviceTopic, sizeof(deviceTopic), "%s/%s", mqttSettings.topic, ChipID::getChipID());
    return deviceTopic;
}

static void getMqttLightMessage(char *buff, size_t len)
{
    JsonDocument doc;
    const char *mode = getLEDModeStr(LED_MODE); // Common Values for all modes
    doc["mode"] = mode;
    doc["state"] = getLedPower() ? "ON" : "OFF";
    doc["brightness"] = getLedBrightness();
    if (LED_MODE == LED_MODES::CCT)
    {
        doc["color_mode"] = "color_temp";
        doc["color_temp"] = getLedColorTemperature();
    }
    else if (LED_MODE == LED_MODES::RGB || LED_MODE == LED_MODES::RGBW || LED_MODE == LED_MODES::RGBWW)
    {
        doc["red"] = getLedRed();
        doc["green"] = getLedGreen();
        doc["blue"] = getLedBlue();
    }
    if (LED_MODE == LED_MODES::RGBW || LED_MODE == LED_MODES::RGBWW)
    {
        doc["ww"] = getLedWW();
    }
    if (LED_MODE == LED_MODES::RGBWW)
    {
        doc["cw"] = getLedCW();
    }
    serializeJson(doc, buff, len);
}

#ifdef RF24RADIO_ENABLED
static void mqttRemotePublish()
{
    for (auto r : getRemoteMap())
    {
        JsonDocument doc;
        char uuid[9];
        char buff[128];
        sprintf(uuid, "%02X%02X%02X%02X", r.second.uuid[0], r.second.uuid[1], r.second.uuid[2], r.second.uuid[3]);

        doc["battery"] = String(r.second.batteryPercentage);
        doc["batteryVoltage"] = String(r.second.batteryVoltage);

        serializeJson(doc, buff, sizeof(buff));
        char topic[64];
        snprintf(topic, sizeof(topic), "%s/RF24-Remote-%s", mqttSettings.topic, uuid);
        log(LOG_LEVEL::INFO, "Publish MQTT message on topic %s with payload: %s\n", topic, buff);
        if (!mqttClient.publish(topic, buff))
        {
            LOG_ERROR("MQTT publish failed for remote: %s\n", uuid);
        }
    }
}
#endif

static void mqttPublish()
{
    // Return if MQTT is not enabled or not connected
    if (!mqttClient.connected())
    {
        return;
    }
    char buffPayload[128];
    getMqttLightMessage(buffPayload, sizeof(buffPayload));
    char topic[64];
    snprintf(topic, sizeof(topic), "%s/light", getDecviceTopic());
    log(LOG_LEVEL::INFO, "Publish MQTT message on topic %s with payload: %s\n", topic, buffPayload);
    if (!mqttClient.publish(topic, buffPayload))
    {
        LOG_ERROR("MQTT publish failed\n");
    }
#ifdef RF24RADIO_ENABLED
    mqttRemotePublish();
#endif
}

#ifdef RF24RADIO_ENABLED
static void mqttRemotesHomeAssistandDiscovery()
{

    RemoteMap remoteMap = getRemoteMap();
    for (auto r : remoteMap)
    {
        JsonDocument doc;
        char uuid[9];
        sprintf(uuid, "%02X%02X%02X%02X", r.second.uuid[0], r.second.uuid[1], r.second.uuid[2], r.second.uuid[3]);

        // Publish remote battery percentage
        doc["name"] = "battery";
        doc["device_class"] = "battery";
        doc["unit_of_measurement"] = "%";
        doc["value_template"] = "{{ value_json.battery }}";
        doc["via_device"] = ChipID::getChipID();
        doc["entity_category"] = "diagnostic";
        doc["state_topic"] = String(mqttSettings.topic) + "/RF24-Remote-" + uuid;
        doc["unique_id"] = "RF24-Remote-" + String(uuid) + "-battery";
        doc["device"]["manufacturer"] = "MarcusVoss";
        doc["device"]["model"] = "RF24-Remote";
        doc["device"]["name"] = "RF24-Remote-" + String(uuid);
        doc["schema"] = "json";
        doc["device"]["identifiers"][0] = "RF24-Remote-" + String(uuid);
        String payload;
        serializeJson(doc, payload);
        String topic = "homeassistant/sensor/RF24-Remote-" + String(uuid) + "/battery/config";
        bool res = mqttClient.publish(topic.c_str(), payload.c_str(), false);
        if (res)
        {
            LOG_INFO("MQTT Home Assistant Remote Battery Discovery published\n");
        }
        else
        {
            LOG_ERROR("MQTT Home Assistant Remote Battery Discovery failed\n");
        }

        // Publish remote battery voltage
        doc.clear();
        doc["name"] = "battery_voltage";
        doc["device_class"] = "voltage";
        doc["unit_of_measurement"] = "mV";
        doc["value_template"] = "{{ value_json.batteryVoltage }}";
        doc["via_device"] = ChipID::getChipID();
        doc["entity_category"] = "diagnostic";
        doc["state_topic"] = String(mqttSettings.topic) + "/RF24-Remote-" + uuid;
        doc["unique_id"] = "RF24-Remote-" + String(uuid) + "-battery-voltage";
        doc["device"]["manufacturer"] = "MarcusVoss";
        doc["device"]["model"] = "RF24-Remote";
        doc["device"]["name"] = "RF24-Remote-" + String(uuid);
        doc["schema"] = "json";
        doc["device"]["identifiers"][0] = "RF24-Remote-" + String(uuid);
        payload.clear();
        serializeJson(doc, payload);
        topic = "homeassistant/sensor/RF24-Remote-" + String(uuid) + "/battery_voltage/config";
        res = mqttClient.publish(topic.c_str(), payload.c_str(), false);
        if (res)
        {
            LOG_INFO("MQTT Home Assistant Remote Battery Voltage Discovery published\n");
        }
        else
        {
            LOG_ERROR("MQTT Home Assistant Remote Battery Voltage Discovery failed\n");
        }
        mqttPublish(); // Publish current state to MQTT
    }
}
#endif

static void mqttHomeAssistandDiscovery()
{
    JsonDocument doc;
    JsonArray colorModes = doc["supported_color_modes"].to<JsonArray>();
    switch (LED_MODE)
    {
    case LED_MODES::SINGLE:
        colorModes.add("brightness");
        break;
    case LED_MODES::CCT:
        colorModes.add("color_temp");
        doc["max_mireds"] = MAX_MIREDS;
        doc["min_mireds"] = MIN_MIREDS;
        break;
    case LED_MODES::RGB:
        colorModes.add("rgb");
        break;
    case LED_MODES::RGBW:
        colorModes.add("rgbw");
        break;
    case LED_MODES::RGBWW:
        colorModes.add("rgbww");
        break;
    }

    JsonArray identifiers = doc["device"]["identifiers"].to<JsonArray>();
    identifiers.add(ChipID::getChipID());

    doc["device_class"] = "light";
    doc["configuration_url"] = WiFi.localIP().toString();
    doc["brightness"] = true;
    doc["brightness_scale"] = 1023,
    doc["command_topic"] = String(getDecviceTopic()) + "/set";
    doc["availability_topic"] = String(getDecviceTopic()) + "/status";
    doc["device"]["manufacturer"] = "MarcusVoss";
    doc["device"]["model"] = MODELNAME;
    doc["device"]["name"] = getDeviceName();
    doc["device"]["sw_version"] = SW_VERSION;
    doc["device"]["identifiers"][0] = ChipID::getChipID();
    doc["name"] = "Light";
    doc["schema"] = "json";
    doc["state_topic"] = String(getDecviceTopic()) + "/light";
    doc["unique_id"] = ChipID::getChipID();

    String payload;
    serializeJson(doc, payload);
    String topic = "homeassistant/light/" + String(ChipID::getChipID()) + "/config";
    bool res = mqttClient.publish(topic.c_str(), payload.c_str(), false);
    if (res)
    {
        LOG_INFO("MQTT Home Assistant Discovery published\n");
    }
    else
    {
        LOG_ERROR("MQTT Home Assistant Discovery failed\n");
    }
    mqttPublish(); // Publish current state to MQTT
}

IRAM_ATTR static void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] payload: ");
    payload[length] = '\0'; // Null terminate the payload
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    Serial.println(strcmp(topic, "homeassistant/status"));
    Serial.println(strcasecmp((char *)payload, "online"));

    // Resend Home Assistant Discovery message if Home Assistant status recieved is "online"
    if (strcmp(topic, "homeassistant/status") == 0 && strcasecmp((char *)payload, "online") == 0)
    {
        LOG_INFO("Home Assistant status online, resending MQTT Discovery\n");
        mqttHomeAssistandDiscovery();
#ifdef RF24RADIO_ENABLED
        mqttRemotesHomeAssistandDiscovery();
#endif
        return;
    }

    // Convert payload to JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error)
    {
        LOG_ERROR("deserializeJson() failed: %s\n", error.c_str());
        return;
    }
    if (doc["state"].is<const char *>())
    {
        const char *state = doc["state"];
        if (strcasecmp(state, "ON") == 0)
        {
            setLedPower(true);
        }
        else if (strcasecmp(state, "OFF") == 0)
        {
            setLedPower(false);
        }
        else
        {
            LOG_WARNING("Invalid state value: %s\n", state);
        }
    }
    if (doc["brightness"].is<uint16_t>())
    {
        uint16_t brightness = doc["brightness"];
        setLedBrightness(brightness);
    }
    if (LED_MODE == LED_MODES::CCT && doc["color_temp"].is<uint16_t>())
    {
        uint16_t color_temp = doc["color_temp"];
        setLedColorTemperature(color_temp);
    }
}

static void mqttConnect()
{
    // Make sure the connection is disconnected
    if (mqttClient.connected())
    {
        mqttClient.disconnect();
        delay(500);
    }

    mqttClient.setServer(mqttSettings.server, mqttSettings.port);
    mqttClient.setCallback(mqttCallback);
    if (mqttClient.connect(ChipID::getChipID(), mqttSettings.username, mqttSettings.password, (String(getDecviceTopic()) + "/status").c_str(), 1, true, "offline"))
    {
        mqttClient.setBufferSize(1024);
        mqttClient.subscribe((String(getDecviceTopic()) + "/set").c_str()); // Subscribe to the set topic
        mqttClient.subscribe("homeassistant/status");                       // Subscribe to the homeassistant status topic
        setLedCallback([]()
                       { ledStateChange = true; });
#ifdef RF24RADIO_ENABLED
        setRadioCallback([]()
                         { newRadioSeen = true; });
#endif
        LOG_INFO("MQTT connected\n");
        delay(100);
        mqttHomeAssistandDiscovery();
        mqttClient.publish((String(getDecviceTopic()) + "/status").c_str(), "online", true);
    }
    else
    {
        LOG_INFO("MQTT connection failed, rc=%i\n", mqttClient.state());
    }
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
    unsigned long delta = millis() - lastPublishTime;

    if (delta < MQTT_MIN_DELAY)
    {
        return; // Prevent MQTT messages from being sent too frequently
    }

    if (ledStateChange || (MQTT_PUBLISH_INTERVAL != -1 && (delta >= MQTT_PUBLISH_INTERVAL)))
    {
        mqttPublish(); // Publish data to MQTT
        lastPublishTime = millis();
        ledStateChange = false;
    }
#ifdef RF24RADIO_ENABLED
    if (newRadioSeen)
    {
        mqttRemotesHomeAssistandDiscovery();
        newRadioSeen = false;
    }
#endif
    mqttClient.loop(); // Allow MQTT client to process incoming and outgoing messages
}

bool getMqttEnabled()
{
    bool enabled = (strlen(mqttSettings.server) > 0 && mqttSettings.port > 0);
    return enabled;
}

static void saveMqttSettings()
{
    LOG_INFO("Saving MQTT settings\n");
    preferences.begin("mqtt_config", false);
    preferences.putString("deviceName", getDeviceName());
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
    saveMqttSettings();
    mqttConnect(); // Reconnect to MQTT with new settings
}

static void loadMQTTsettings()
{
    preferences.begin("mqtt_config", false);
    char deviceName[32];
    if (preferences.getString("deviceName", deviceName, sizeof(deviceName)))
    {
        setDeviceName(deviceName);
    }
    strcpy(mqttSettings.server, preferences.getString("mqttServer", "").c_str());
    mqttSettings.port = preferences.getInt("mqttPort", 1883);
    strcpy(mqttSettings.username, preferences.getString("mqttUsername", "").c_str());
    strcpy(mqttSettings.password, preferences.getString("mqttPassword", "").c_str());
    strcpy(mqttSettings.topic, preferences.getString("mqttTopic", "").c_str());
    preferences.end();
    LOG_INFO("MQTT Settings loaded: Server: %s Port: %i Username: %s Password: %s Topic: %s\n",
             mqttSettings.server, mqttSettings.port, mqttSettings.username, mqttSettings.password, mqttSettings.topic);
}

void mqttInit()
{
    loadMQTTsettings();
    mqttClient.setBufferSize(512);
}
