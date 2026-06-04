// Claude Generated: version 1 - Parser declaration for gps_monitor/sky MQTT payload
#pragma once
#include "model/Satellite.h"

// Parses a gps_monitor/sky JSON payload into SkyData.
// Returns false if JSON is invalid or missing required structure.
bool parseSky(const char* json, SkyData& out);
