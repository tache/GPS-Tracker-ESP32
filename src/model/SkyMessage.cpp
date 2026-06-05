// Claude Generated: version 1 - ArduinoJson parser for gps_monitor/sky MQTT payload
// Claude Generated: version 2 - Parse el/az/ss as float then cast; GPSD sends them as JSON floats
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
        // el/az/ss come from GPSD as JSON floats (e.g. 72.0); | 0.0f reads them correctly,
        // then cast to int for our struct. Using | 0 (int) would fail the is<int>() check.
        out.satellites.push_back(Satellite{
            s["prn"]  | 0,
            (int)(s["el"]   | 0.0f),
            (int)(s["az"]   | 0.0f),
            (int)(s["ss"]   | 0.0f),
            s["used"] | false
        });
    }
    return true;
}
