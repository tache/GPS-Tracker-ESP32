// Claude Generated: version 1 - Satellite and SkyData structs for MQTT sky message
#pragma once
#include <vector>

struct Satellite {
    int prn;
    int elevation;  // degrees above horizon, 0-90
    int azimuth;    // degrees true north, 0-359
    int snr;        // signal-to-noise ratio dBHz (from JSON "ss" field)
    bool used;      // true if contributing to current GPS fix
};

struct SkyData {
    int satUsed    = 0;
    int satVisible = 0;
    std::vector<Satellite> satellites;
};
