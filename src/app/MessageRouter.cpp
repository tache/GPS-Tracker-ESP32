// Claude Generated: version 1 - MessageRouter implementation
#include "app/MessageRouter.h"
#include "model/SkyMessage.h"
#include "model/TpvMessage.h"

static bool endsWith(const std::string& s, const char* suffix) {
    const std::string suf(suffix);
    return s.size() >= suf.size() &&
           s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

void MessageRouter::route(const std::string& topic,
                          const std::string& payload,
                          uint32_t nowSec) {
    if (endsWith(topic, "/sky")) {
        SkyData d;
        if (parseSky(payload.c_str(), d)) store_.updateSky(d, nowSec);
    } else if (endsWith(topic, "/tpv")) {
        std::string fix;
        if (parseTpv(payload.c_str(), fix)) store_.updateFix(fix);
    } else if (endsWith(topic, "/availability")) {
        store_.setOnline(payload == "online");
    }
}
