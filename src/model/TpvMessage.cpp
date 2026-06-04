// Claude Generated: version 1 - ArduinoJson parser for gps_monitor/tpv MQTT payload
#include "model/TpvMessage.h"
#include <ArduinoJson.h>

bool parseTpv(const char* json, std::string& fixOut) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return false;
    fixOut = doc["fix"] | "No Fix";
    return true;
}
