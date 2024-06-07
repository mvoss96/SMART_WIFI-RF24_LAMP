#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>

#include "config.h"
#include "chipID.h"
#include "mqtt.h"

MQTT_Settings mqttSettings;
static long mqttReconnectTimer = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
static Preferences preferences;

void mqttHomeAssistandDiscovery()
{
}

void mqttConnect()
{
    Serial.println("Attempting MQTT connection...");
    // Make sure the connection is disconnected
    if (mqttClient.connected())
    {
        mqttClient.disconnect();
    }

    mqttClient.setServer(mqttSettings.server, mqttSettings.port);
    if (mqttClient.connect(ChipID::getChipID(), mqttSettings.username, mqttSettings.password))
    {
        mqttHomeAssistandDiscovery();
        Serial.println("connected");
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.println(mqttClient.state());
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
    
    // mqttPublish(sensor); // Publish sensor data to MQTT
    mqttClient.loop(); // Allow MQTT client to process incoming and outgoing messages
}

bool getMqttEnabled()
{
    return (strlen(mqttSettings.server) > 0 && mqttSettings.port > 0);
}

void printMqttConfig()
{
    Serial.println("MQTT Config:");
    Serial.print("Server: ");
    Serial.println(mqttSettings.server);
    Serial.print("Port: ");
    Serial.println(mqttSettings.port);
    Serial.print("Username: ");
    Serial.println(mqttSettings.username);
    Serial.print("Password: ");
    Serial.println(mqttSettings.password);
    Serial.print("Topic: ");
    Serial.println(mqttSettings.topic);
}

void saveMqttSettings()
{
    preferences.putString("mqttServer", mqttSettings.server);
    preferences.putString("mqttUsername", mqttSettings.username);
    preferences.putString("mqttPassword", mqttSettings.password);
    preferences.putString("mqttTopic", mqttSettings.topic);
    preferences.putInt("mqttPort", mqttSettings.port);
}

void setMqttSettings(const char *server, const unsigned int port, const char *username, const char *password, const char *topic)
{
    strcpy(mqttSettings.server, server);
    mqttSettings.port = port;
    strcpy(mqttSettings.username, username);
    strcpy(mqttSettings.password, password);
    strcpy(mqttSettings.topic, topic);
    Serial.println("MQTT settings updated");
    printMqttConfig();
    saveMqttSettings();
}

void loadMQTTsettings()
{
    strcpy(mqttSettings.server, preferences.getString("mqtt_server", "").c_str());
    mqttSettings.port = preferences.getInt("mqtt_port", 1883);
    strcpy(mqttSettings.username, preferences.getString("mqtt_username", "").c_str());
    strcpy(mqttSettings.password, preferences.getString("mqtt_password", "").c_str());
    strcpy(mqttSettings.topic, preferences.getString("mqtt_topic", ChipID::getChipID()).c_str());
    printMqttConfig();
}

void mqttInit()
{
    preferences.begin("mqtt_config", false);
    loadMQTTsettings();
    mqttClient.setBufferSize(512);
}
