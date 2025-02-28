#pragma once
#include <string>

// Base class for Home Assistant Device based discovery
class BaseHaDiscovery
{
protected:
    std::string topic;
    std::string payloadStr;

public:
    std::string_view getTopic();
    std::string_view getPayloadString();
};

// Home Assistant Device based discovery
class HaDiscovery : public BaseHaDiscovery
{
public:
    HaDiscovery(std::string_view baseTopic);
};

// Remote Home Assistant Device based discovery
class RemoteHaDiscovery : public BaseHaDiscovery
{
public:
    RemoteHaDiscovery(std::string_view baseTopic, std::string_view uuid);
};