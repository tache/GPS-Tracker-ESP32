// src/view/ShipSteering.h
// Claude Generated: version 1 - Pure-C++ steering math for ShipEasterEgg (no LovyanGFX)
#pragma once
#include <cmath>
#include <vector>

namespace ship_math {

struct Point2f { float x, y; };

// Display geometry
constexpr float kCx       = 120.0f;
constexpr float kCy       = 120.0f;
constexpr float kEntryR   = 108.0f;  // disc rim — entry point radius
constexpr float kExitR    = 140.0f;  // beyond screen edge — exit target radius

// Tunable steering constants.
// Note: these live here (not in ShipEasterEgg.cpp's anonymous namespace as the spec suggests)
// because the Unity tests reference kMaxTurnRad directly. Moving them to the .cpp would
// break test_stepHeading_beyond_range_clamps.
constexpr float kSpeedPx     = 10.0f;
constexpr float kMaxTurnRad  = 20.0f * 3.14159265f / 180.0f;
constexpr float kAvoidRadius = 40.0f;
constexpr float kAvoidScale  = 8000.0f;

struct ShipState {
    bool    active    = false;
    Point2f pos       = {kCx, kCy};
    float   heading   = 0.0f;  // radians; 0 = +x (right), pi/2 = +y (down)
    Point2f exitPt    = {kCx, kCy};
    bool    thrustOn  = false;
};

// Wrap angle into [-pi, +pi]
inline float wrapAngle(float a) {
    while (a >  3.14159265f) a -= 2.0f * 3.14159265f;
    while (a < -3.14159265f) a += 2.0f * 3.14159265f;
    return a;
}

// Turn current heading toward desired by at most maxTurn radians.
// Takes shortest angular path.
inline float stepHeading(float current, float desired,
                          float maxTurn = kMaxTurnRad) {
    float delta = wrapAngle(desired - current);
    if (fabsf(delta) <= maxTurn) return desired;
    return current + (delta > 0.0f ? maxTurn : -maxTurn);
}

// Compute desired heading: toward exitPt, repelled by satellites within kAvoidRadius.
// Guards: skips satellites at distance < 1.0f; keeps current heading if steer vector
// is degenerate (magnitude < 1e-4).
inline float computeDesiredHeading(const ShipState& s,
                                    const std::vector<Point2f>& sats) {
    // Target vector (unit)
    float tx  = s.exitPt.x - s.pos.x;
    float ty  = s.exitPt.y - s.pos.y;
    float len = sqrtf(tx * tx + ty * ty);
    if (len > 1e-4f) { tx /= len; ty /= len; }

    // Avoidance vector: sum repulsion from nearby satellites
    float ax = 0.0f, ay = 0.0f;
    for (const auto& sat : sats) {
        float dx   = s.pos.x - sat.x;
        float dy   = s.pos.y - sat.y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < 1.0f || dist > kAvoidRadius) continue;
        float scale = kAvoidScale / (dist * dist);
        ax += (dx / dist) * scale;
        ay += (dy / dist) * scale;
    }

    float sx   = tx + ax;
    float sy   = ty + ay;
    float slen = sqrtf(sx * sx + sy * sy);
    if (slen < 1e-4f) return s.heading;  // degenerate: keep heading
    return atan2f(sy, sx);
}

// True when ship has left the 250x250 exit detection box around the display.
inline bool isOutsideBounds(const Point2f& pos) {
    return pos.x < -5.0f || pos.x > 245.0f ||
           pos.y < -5.0f || pos.y > 245.0f;
}

// Produce a spawned ShipState.
// entryAngleRad: angle on disc rim for entry [0, 2pi)
// exitAngleDelta: angular offset from entry for exit [pi/2, 3pi/2] — opposite half
inline ShipState makeSpawnedState(float entryAngleRad, float exitAngleDelta) {
    ShipState s;
    s.active = true;
    s.pos    = { kCx + kEntryR * cosf(entryAngleRad),
                 kCy + kEntryR * sinf(entryAngleRad) };
    float exitAngle = entryAngleRad + exitAngleDelta;
    s.exitPt = { kCx + kExitR * cosf(exitAngle),
                 kCy + kExitR * sinf(exitAngle) };
    s.heading   = atan2f(s.exitPt.y - s.pos.y, s.exitPt.x - s.pos.x);
    s.thrustOn  = false;
    return s;
}

}  // namespace ship_math
