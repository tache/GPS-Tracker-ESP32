// src/view/AsteroidShape.h
// Claude Generated: version 1 - Asteroid polygon shapes derived from original Asteroids ROM rock patterns
// Claude Generated: version 2 - Add rotation angle parameter for slow per-asteroid spin
#pragma once
#include <LovyanGFX.hpp>
#include <cmath>
#include <cstdint>

namespace asteroid_shape {

struct Pt { int8_t x; int8_t y; };

// Vertices accumulated from the four ROM rock-pattern SVEC sequences (first SVEC = dark move-to,
// remaining SVECs = drawn edges). Scaled by 0.25 from ROM units (ROM peaks at ±32 → ±8 px).
// Each array closes implicitly: last vertex connects back to first.

// ROM Pattern 1 — 10 vertices
static constexpr Pt kShape0[] = {
    { 0, 4},{4, 8},{ 8, 4},{ 6, 0},{ 8,-4},
    { 2,-8},{-4,-8},{-8,-4},{-8, 4},{-4, 8}
};
// ROM Pattern 2 — 12 vertices
static constexpr Pt kShape1[] = {
    { 4, 2},{ 8, 4},{ 4, 8},{ 0, 6},{-4, 8},
    {-8, 4},{-6, 0},{-8,-4},{-4,-8},{-2,-6},{ 4,-8},{ 8,-2}
};
// ROM Pattern 3 — 11 vertices
static constexpr Pt kShape2[] = {
    {-4, 0},{-8,-2},{-4,-8},{ 0,-2},{ 0,-8},
    { 4,-8},{ 8,-2},{ 8, 2},{ 4, 8},{-2, 8},{-8, 2}
};
// ROM Pattern 4 — 12 vertices
static constexpr Pt kShape3[] = {
    { 2, 0},{ 8, 2},{ 8, 4},{ 2, 8},{-4, 8},
    {-2, 4},{-8, 4},{-8,-2},{-4,-8},{ 2,-6},{ 4,-8},{ 8,-4}
};

struct ShapeRef { const Pt* pts; int n; };
static constexpr ShapeRef kShapes[] = {
    {kShape0, 10},
    {kShape1, 12},
    {kShape2, 11},
    {kShape3, 12},
};
static constexpr int kShapeCount = 4;

// Draw asteroid polygon for a satellite at screen position (cx, cy).
// shapeIdx is derived from the satellite's PRN so each PRN always uses the same shape.
// angle (radians) rotates the polygon in place — pass 0.0f for no rotation.
inline void draw(LovyanGFX& canvas, int cx, int cy, int shapeIdx, uint16_t color,
                 float angle = 0.0f) {
    const ShapeRef& sr = kShapes[shapeIdx % kShapeCount];
    float c  = cosf(angle);
    float sn = sinf(angle);
    for (int i = 0; i < sr.n; ++i) {
        const Pt& pa = sr.pts[i];
        const Pt& pb = sr.pts[(i + 1) % sr.n];
        int x1 = cx + (int)(c * pa.x - sn * pa.y);
        int y1 = cy + (int)(sn * pa.x +  c * pa.y);
        int x2 = cx + (int)(c * pb.x - sn * pb.y);
        int y2 = cy + (int)(sn * pb.x +  c * pb.y);
        canvas.drawLine(x1, y1, x2, y2, color);
    }
}

}  // namespace asteroid_shape
