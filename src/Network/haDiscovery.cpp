#include "config.h"
#include "haDiscovery.h"
#include "Logging/logging.h"
#include "ChipID/chipID.h"

#include <ArduinoJson.h>
#include <sstream>

static const char* REMOTE_NAME = "RF24-Remote";

std::string_view BaseHaDiscovery::getTopic()
{
    return topic;
}

std::string_view BaseHaDiscovery::getPayloadString()
{
    return payloadStr;
}

// Constructor
HaDiscovery::HaDiscovery(std::string_view baseTopic)
{
    // Create the topic
    std::ostringstream topicStream;
    topicStream << "homeassistant/device/" << ChipID::getChipID() << "/config";
    topic = topicStream.str();

    // Create the payload
    JsonDocument doc;

    // Device Sub-Object
    JsonObject device = doc["dev"].to<JsonObject>();
    device["ids"] = ChipID::getChipID();
    device["name"] = getDeviceName();
    device["mf"] = "MarcusVoss";
    device["mdl"] = MODELNAME;
    device["sw"] = SW_VERSION;
    device["sn"] = ChipID::getChipID();

    // Orogin Sub-Object
    JsonObject origin = doc["o"].to<JsonObject>();
    origin["name"] = "MarcusVoss";

    // Components
    JsonObject components = doc["cmps"].to<JsonObject>();

    // Light Sub-Object
    JsonObject light = components["light"].to<JsonObject>();
    light["p"] = "light";
    light["name"] = "Light";
    light["schema"] = "json";
    light["device_class"] = "light";
    light["brightness_scale"] = LED_MAX_VAL;
    std::ostringstream commandTopicStream;
    commandTopicStream << baseTopic << "/" << ChipID::getChipID() << "/set";
    light["command_topic"] = commandTopicStream.str();
    std::ostringstream availabilityTopicStream;
    availabilityTopicStream << baseTopic << "/" << ChipID::getChipID() << "/status";
    light["availability_topic"] = availabilityTopicStream.str();
    JsonArray colorModes = light["supported_color_modes"].to<JsonArray>();
    switch (LED_MODE)
    {
    case LED_MODES::SINGLE:
        colorModes.add("brightness");
        break;
    case LED_MODES::CCT:
        colorModes.add("color_temp");
        light["max_mireds"] = MAX_MIREDS;
        light["min_mireds"] = MIN_MIREDS;
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
    std::ostringstream stateTopicStream;
    stateTopicStream << baseTopic << "/" << ChipID::getChipID() << "/light";
    light["state_topic"] = stateTopicStream.str();
    std::ostringstream uniqueIDStream;
    uniqueIDStream << ChipID::getChipID() << "_light";
    light["unique_id"] = uniqueIDStream.str();

    // Diagnostic IP Address
    JsonObject diagnosticIP = components["diagIP"].to<JsonObject>();
    diagnosticIP["p"] = "sensor";
    diagnosticIP["name"] = "IP Address";
    diagnosticIP["entity_category"] = "diagnostic";
    std::ostringstream uniqueIDIPStream;
    uniqueIDIPStream << ChipID::getChipID() << "_ip";
    diagnosticIP["unique_id"] = uniqueIDIPStream.str();
    std::ostringstream stateTopicIPStream;
    stateTopicIPStream << baseTopic << "/" << ChipID::getChipID() << "/diagnostic";
    diagnosticIP["state_topic"] = stateTopicIPStream.str();
    diagnosticIP["value_template"] = "{{ value_json.ip }}";

    // Diagnostics RSSI
    JsonObject diagnosticRSSI = components["diagRSSI"].to<JsonObject>();
    diagnosticRSSI["p"] = "sensor";
    diagnosticRSSI["name"] = "RSSI";
    diagnosticRSSI["device_class"] = "signal_strength";
    diagnosticRSSI["entity_category"] = "diagnostic";
    std::ostringstream uniqueIDRSSIStream;
    uniqueIDRSSIStream << ChipID::getChipID() << "_rssi";
    diagnosticRSSI["unique_id"] = uniqueIDRSSIStream.str();
    std::ostringstream stateTopicRSSIStream;
    stateTopicRSSIStream << baseTopic << "/" << ChipID::getChipID() << "/diagnostic";
    diagnosticRSSI["state_topic"] = stateTopicRSSIStream.str();
    diagnosticRSSI["unit_of_measurement"] = "dBm";
    diagnosticRSSI["value_template"] = "{{ value_json.rssi }}";

#ifdef RF24RADIO_ENABLED
    JsonObject radioChannel = components["radioChannel"].to<JsonObject>();
    radioChannel["p"] = "sensor";
    radioChannel["name"] = "Radio Channel";
    radioChannel["entity_category"] = "diagnostic";
    std::ostringstream uniqueIDRadioChannelStream;
    uniqueIDRadioChannelStream << ChipID::getChipID() << "_radioChannel";
    radioChannel["unique_id"] = uniqueIDRadioChannelStream.str();
    std::ostringstream stateTopicRadioChannelStream;
    stateTopicRadioChannelStream << baseTopic << "/" << ChipID::getChipID() << "/diagnostic";
    radioChannel["state_topic"] = stateTopicRadioChannelStream.str();
    radioChannel["value_template"] = "{{ value_json.radioChannel }}";

    JsonObject radioAddress = components["radioAddress"].to<JsonObject>();
    radioAddress["p"] = "sensor";
    radioAddress["name"] = "Radio Address";
    radioAddress["entity_category"] = "diagnostic";
    std::ostringstream uniqueIDRadioAddressStream;
    uniqueIDRadioAddressStream << ChipID::getChipID() << "_radioAddress";
    radioAddress["unique_id"] = uniqueIDRadioAddressStream.str();
    std::ostringstream stateTopicRadioAddressStream;
    stateTopicRadioAddressStream << baseTopic << "/" << ChipID::getChipID() << "/diagnostic";
    radioAddress["state_topic"] = stateTopicRadioAddressStream.str();
    radioAddress["value_template"] = "{{ value_json.radioAddress }}";
#endif

    // Serialize the JSON document
    size_t jsize = serializeJson(doc, payloadStr);
    LOG_INFO("Serialized JSON size: %d str size: %d\n", jsize, payloadStr.size());
}

RemoteHaDiscovery::RemoteHaDiscovery(std::string_view baseTopic, std::string_view uuid)
{
    std:: ostringstream remoteName;
    remoteName << REMOTE_NAME << "-" << uuid;

    // Create the topic
    std::ostringstream topicStream;
    topicStream << "homeassistant/device/" << remoteName.str() << "/config";
    topic = topicStream.str();

    // Create the payload
    JsonDocument doc;
    
    // Device Sub-Object
    JsonObject device = doc["dev"].to<JsonObject>();
    device["ids"] = remoteName.str();
    device["name"] = remoteName.str();
    device["mf"] = "MarcusVoss";
    device["mdl"] = REMOTE_NAME;
    std::ostringstream stateTopicStream;
    stateTopicStream << baseTopic << "/" << remoteName.str();
    device["state_topic"] = stateTopicStream.str();
    device["schema"] = "json";

    // Orogin Sub-Object
    JsonObject origin = doc["o"].to<JsonObject>();
    origin["name"] = "MarcusVoss";

    // Components
    JsonObject components = doc["cmps"].to<JsonObject>();

    // Battery Sub-Object
    JsonObject battery = components["battery"].to<JsonObject>();
    battery["p"] = "sensor";
    battery["name"] = "Battery";
    battery["device_class"] = "battery";
    battery["unit_of_measurement"] = "%";
    std::ostringstream uniqueIDBatteryStream;
    uniqueIDBatteryStream << remoteName.str() << "_battery";
    battery["unique_id"] = uniqueIDBatteryStream.str();
    battery["value_template"] = "{{ value_json.battery }}";

    // Battery Voltage Sub-Object
    JsonObject batteryVoltage = components["batteryVoltage"].to<JsonObject>();
    batteryVoltage["p"] = "sensor";
    batteryVoltage["name"] = "Battery Voltage";
    batteryVoltage["device_class"] = "voltage";
    batteryVoltage["unit_of_measurement"] = "mV";
    std::ostringstream uniqueIDBatteryVoltageStream;
    uniqueIDBatteryVoltageStream << remoteName.str() << "_batteryVoltage";
    batteryVoltage["unique_id"] = uniqueIDBatteryVoltageStream.str();
    batteryVoltage["value_template"] = "{{ value_json.batteryVoltage }}";

    // lastSeenBy Sub-Object
    JsonObject lastSeenBy = components["lastSeenBy"].to<JsonObject>();
    lastSeenBy["p"] = "sensor";
    lastSeenBy["name"] = "Last Seen By";
    lastSeenBy["entity_category"] = "diagnostic";
    std::ostringstream uniqueIDLastSeenByStream;
    uniqueIDLastSeenByStream << remoteName.str() << "_lastSeenBy";
    lastSeenBy["unique_id"] = uniqueIDLastSeenByStream.str();
    lastSeenBy["value_template"] = "{{ value_json.lastSeenBy }}";

    serializeJson(doc, payloadStr);
}
