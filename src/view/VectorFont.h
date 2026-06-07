// src/view/VectorFont.h
// Claude Generated: version 1 - Minimal vector font for "c 1979 ATARI INC", ROM-faithful strokes
// Claude Generated: version 2 - Add G, M, E, O, V for "GAME OVER" display
// Claude Generated: version 3 - Add S, D; scale parameter for larger title rendering
#pragma once
#include <LovyanGFX.hpp>
#include <cstdint>

namespace vector_font {

// Each character fits a 5×8 px cell. kAdvance is the per-character step (width + 2 px gap).
constexpr int kCharH   = 8;
constexpr int kAdvance = 7;

struct Seg { int8_t x1, y1, x2, y2; };

// Strokes derived from Asteroids ROM DVG character routines.
// Coordinates are in local cell space: x ∈ [0,4], y ∈ [0,7] (top-left origin, y down).

static constexpr Seg kA[] = {                              // two diagonals + crossbar
    {0,7, 2,0}, {2,0, 4,7}, {1,4, 3,4} };
static constexpr Seg kC[] = {                              // open right — three sides
    {4,1, 4,0}, {4,0, 0,0}, {0,0, 0,7}, {0,7, 4,7}, {4,7, 4,6} };
static constexpr Seg kI[] = {                              // serifs top+bottom + stem
    {1,0, 3,0}, {2,0, 2,7}, {1,7, 3,7} };
static constexpr Seg kN[] = {                              // two verticals + diagonal
    {0,7, 0,0}, {0,0, 4,7}, {4,7, 4,0} };
static constexpr Seg kR[] = {                              // P-bump + diagonal leg
    {0,7, 0,0}, {0,0, 4,0}, {4,0, 4,3}, {4,3, 0,3}, {2,3, 4,7} };
static constexpr Seg kT[] = {                              // top bar + center stem
    {0,0, 4,0}, {2,0, 2,7} };
static constexpr Seg k1[] = {                              // serif + vertical + baseline
    {1,1, 2,0}, {2,0, 2,7}, {0,7, 4,7} };
static constexpr Seg k7[] = {                              // top bar + diagonal
    {0,0, 4,0}, {4,0, 2,7} };
static constexpr Seg k9[] = {                              // closed loop + right tail
    {0,4, 0,0}, {0,0, 4,0}, {4,0, 4,7}, {0,4, 4,4} };
static constexpr Seg kE[] = {                              // left vertical + top/mid/bottom bars
    {0,0, 0,7}, {0,0, 4,0}, {0,4, 3,4}, {0,7, 4,7} };
static constexpr Seg kG[] = {                              // C with inward shelf at midpoint
    {4,0, 0,0}, {0,0, 0,7}, {0,7, 4,7}, {4,7, 4,4}, {4,4, 2,4} };
static constexpr Seg kM[] = {                              // two verticals + centre V-peak
    {0,7, 0,0}, {0,0, 2,4}, {2,4, 4,0}, {4,0, 4,7} };
static constexpr Seg kO[] = {                              // closed rectangle
    {0,0, 4,0}, {4,0, 4,7}, {4,7, 0,7}, {0,7, 0,0} };
static constexpr Seg kV[] = {                              // two diagonals to bottom centre
    {0,0, 2,7}, {2,7, 4,0} };
static constexpr Seg kS[] = {                              // top bar, top-left, mid, bot-right, bot bar
    {0,0, 4,0}, {0,0, 0,4}, {0,4, 4,4}, {4,4, 4,7}, {4,7, 0,7} };
static constexpr Seg kD[] = {                              // left vertical + rounded-right bulge
    {0,0, 0,7}, {0,0, 3,0}, {3,0, 4,2}, {4,2, 4,5}, {4,5, 3,7}, {3,7, 0,7} };

struct CharDef { const Seg* segs; int n; };

inline CharDef charDef(char ch) {
    switch (ch) {
        case 'A':           return {kA, 3};
        case 'C': case 'c': return {kC, 5};
        case 'E':           return {kE, 4};
        case 'G':           return {kG, 5};
        case 'I':           return {kI, 3};
        case 'M':           return {kM, 4};
        case 'N':           return {kN, 3};
        case 'O':           return {kO, 4};
        case 'R':           return {kR, 5};
        case 'T':           return {kT, 2};
        case 'V':           return {kV, 2};
        case 'S':           return {kS, 5};
        case 'D':           return {kD, 6};
        case '1':           return {k1, 3};
        case '7':           return {k7, 2};
        case '9':           return {k9, 4};
        default:            return {nullptr, 0};  // space and unknowns
    }
}

// Draw one character with top-left corner at (x, y). scale multiplies all coordinates.
inline void drawChar(LovyanGFX& canvas, int x, int y, char ch, uint16_t color,
                     int scale = 1) {
    CharDef def = charDef(ch);
    for (int i = 0; i < def.n; ++i) {
        const Seg& sg = def.segs[i];
        canvas.drawLine(x + sg.x1 * scale, y + sg.y1 * scale,
                        x + sg.x2 * scale, y + sg.y2 * scale, color);
    }
}

// Draw null-terminated string, horizontally centred on (cx, cy). scale multiplies glyph size.
inline void drawStringCentered(LovyanGFX& canvas, int cx, int cy,
                                const char* str, uint16_t color, int scale = 1) {
    int len = 0;
    for (const char* p = str; *p; ++p) len++;
    int startX = cx - (len * kAdvance * scale) / 2;
    int startY = cy - (kCharH * scale) / 2;
    for (int i = 0; str[i]; ++i) {
        drawChar(canvas, startX + i * kAdvance * scale, startY, str[i], color, scale);
    }
}

}  // namespace vector_font
