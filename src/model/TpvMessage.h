// Claude Generated: version 1 - Parser declaration for gps_monitor/tpv MQTT payload
#pragma once
#include <string>

// Parses the "fix" field from a gps_monitor/tpv JSON payload.
// Returns false on invalid JSON; fixOut defaults to "No Fix" when field absent.
bool parseTpv(const char* json, std::string& fixOut);
