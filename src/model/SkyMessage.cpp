// Claude Generated: version 1 - ArduinoJson parser for gps_monitor/sky MQTT payload
#include "model/SkyMessage.h"
#include <ArduinoJson.h>

bool parseSky(const char* json, SkyData& out) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return false;
    if (!doc["satellites"].is<JsonArray>()) return false;
    out = SkyData{};
    out.satUsed    = doc["sat_used"]    | 0;
    out.satVisible = doc["sat_visible"] | 0;
    for (JsonObject s : doc["satellites"].as<JsonArray>()) {
        out.satellites.push_back(Satellite{
            s["prn"]  | 0,
            s["el"]   | 0,
            s["az"]   | 0,
            s["ss"]   | 0,
            s["used"] | false
        });
    }
    return true;
}
