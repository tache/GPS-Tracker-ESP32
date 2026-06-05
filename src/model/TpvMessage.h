// Claude Generated: version 1 - Parser declaration for gps_monitor/tpv MQTT payload
// Claude Generated: version 2 - Also parse GPS time (HH:MM:SS Zulu) from ISO 8601 field
#pragma once
#include <string>

// Parses fix and GPS time from a gps_monitor/tpv JSON payload.
// Returns false on invalid JSON.
// fixOut  defaults to "No Fix"   when the "fix"  field is absent.
// timeOut defaults to "--:--:--" when the "time" field is absent or malformed.
bool parseTpv(const char* json, std::string& fixOut, std::string& timeOut);
