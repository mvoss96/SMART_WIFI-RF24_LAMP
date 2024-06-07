#pragma once

struct MQTT_Settings
{
    char server[40] = "";
    char username[40];
    char password[40];
    char topic[40] = "";
    int port = 1883;
};

extern MQTT_Settings mqttSettings;

bool getMqttEnabled();
void handleMQTTConnection();
void setMqttEnabled(bool enabled);
void mqttInit();
void mqttConnect();
void setMqttSettings(const char *server, const unsigned int port, const char *username, const char *password, const char *topic);
