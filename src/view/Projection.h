#pragma once
// Claude Generated: version 1 - Polar coordinate projection for satellite skyview
#include <cmath>

namespace projection {

struct Point { float x; float y; };

// Maps satellite position to screen coordinates.
// Center = zenith (el 90), rim = horizon (el 0), north at top, screen y-down.
// Projection matches PolarGraphView.polarPoint in the GPS-Tracker Mac app.
inline Point polarPoint(int elevation, int azimuth,
                        float cx, float cy, float radius) {
    const float dist = (1.0f - static_cast<float>(elevation) / 90.0f) * radius;
    const float azRad = static_cast<float>(azimuth) * 3.14159265358979f / 180.0f;
    return { cx + dist * std::sin(azRad), cy - dist * std::cos(azRad) };
}

}  // namespace projection
