// Claude Generated: version 1 - MessageRouter implementation
// Claude Generated: version 2 - Pass GPS Zulu time through to store alongside fix
#include "app/MessageRouter.h"
#include "model/SkyMessage.h"
#include "model/TpvMessage.h"
#include <string_view>

// std::string::ends_with is C++20; we target C++17, so use string_view comparison.
static bool endsWith(const std::string& s, std::string_view suffix) {
    return s.size() >= suffix.size() &&
           std::string_view(s).substr(s.size() - suffix.size()) == suffix;
}

void MessageRouter::route(const std::string& topic,
                          const std::string& payload,
                          uint32_t nowSec) {
    if (endsWith(topic, "/sky")) {
        SkyData d;
        if (parseSky(payload.c_str(), d)) store_.updateSky(d, nowSec);
    } else if (endsWith(topic, "/tpv")) {
        std::string fix, time;
        if (parseTpv(payload.c_str(), fix, time)) store_.updateFix(fix, time);
    } else if (endsWith(topic, "/availability")) {
        store_.setOnline(payload == "online");
    }
}
