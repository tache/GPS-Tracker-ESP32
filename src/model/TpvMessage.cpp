// Claude Generated: version 1 - ArduinoJson parser for gps_monitor/tpv MQTT payload
// Claude Generated: version 2 - Also extract HH:MM:SS Zulu time from ISO 8601 "time" field
#include "model/TpvMessage.h"
#include <ArduinoJson.h>
#include <cstring>

bool parseTpv(const char* json, std::string& fixOut, std::string& timeOut) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return false;
    fixOut = doc["fix"] | "No Fix";
    // "time" field is ISO 8601: "2026-04-01T18:32:10Z" — extract HH:MM:SS at index 11.
    const char* t = doc["time"] | "";
    if (strlen(t) >= 19 && t[10] == 'T') {
        timeOut = std::string(t + 11, 8) + " Z";  // "HH:MM:SS Z"
    } else {
        timeOut = "--:--:-- Z";
    }
    return true;
}
