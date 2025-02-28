#pragma once
#include <string>

// Home Assitant Device based discovery
class HaDiscovery
{
private:
    std::string topic;
    std::string payloadStr;
    
public:
    HaDiscovery(std::string_view baseTopic);
    std::string_view getTopic();
    std::string_view getPayloadString();
};

