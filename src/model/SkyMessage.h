// Claude Generated: version 1 - Parser for gps_monitor/sky MQTT payload
#pragma once
#include "model/Satellite.h"
#include <ArduinoJson.h>

// Parses a gps_monitor/sky JSON payload into SkyData.
// Returns false if JSON is invalid or missing required structure.
inline bool parseSky(const char* json, SkyData& out) {
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
