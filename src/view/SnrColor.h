// Claude Generated: version 1 - SNR-based satellite color classification and RGB565 conversion
#pragma once
#include <cstdint>

namespace snrcolor {

enum class SatColor { Red, Green, Orange, Yellow };

// Exact port of Satellite.color from the GPS-Tracker Mac app (Models/Satellite.swift).
// Red = not used in fix (hollow); Green/Orange/Yellow = used, by SNR tier.
inline SatColor classify(bool used, int snr) {
    if (!used)    return SatColor::Red;
    if (snr >= 35) return SatColor::Green;
    if (snr >= 20) return SatColor::Orange;
    return SatColor::Yellow;
}

// RGB565 equivalents of SwiftUI red/green/orange/yellow for GC9A01A rendering.
inline uint16_t toRgb565(SatColor c) {
    switch (c) {
        case SatColor::Red:    return 0xF800;  // 255,0,0
        case SatColor::Green:  return 0x07E0;  // 0,255,0
        case SatColor::Orange: return 0xFD20;  // 255,165,0
        case SatColor::Yellow: return 0xFFE0;  // 255,255,0
    }
    return 0xFFFF;
}

}  // namespace snrcolor
