// Claude Generated: version 1 - SkyView render implementation (port of PolarGraphView.swift)
// Claude Generated: version 2 - Sprite double-buffering to eliminate per-frame flicker
// Claude Generated: version 3 - Call ship_.tick() before frame commit
// Claude Generated: version 4 - Hide rings/labels/status while ship is active
// Claude Generated: version 5 - Draw satellites as asteroid polygons while ship is active
// Claude Generated: version 6 - Azimuth rings green
// Claude Generated: version 7 - Draw "c 1979 ATARI INC" vector copyright during ship mode
// Claude Generated: version 8 - Remove satellite trails; ROM-faithful explosion
// Claude Generated: version 9 - Spinning asteroids; GAME OVER flash; nowMs captured once per frame
#include "view/SkyView.h"
#include "view/Projection.h"
#include "view/SnrColor.h"
#include "view/AsteroidShape.h"
#include <cstdio>

namespace {

// Display is 240×240; centre is (120,120).
// kR = 0.45*240 = 108 — leaves ~12 px margin so the 12-px-offset cardinal labels don't clip.
// kBg is dark grey; kRing/kRingMinor are green (R=0, G=24/12, B=0 in RGB565).
constexpr float    kCx   = 120.0f;
constexpr float    kCy   = 120.0f;
constexpr float    kR    = 108.0f;
constexpr int      kDot  = 6;
constexpr uint16_t kBg        = 0x10A2;  // dark grey disc background
constexpr uint16_t kRing      = 0x0300;  // major elevation ring green (0°/30°/60°)
constexpr uint16_t kRingMinor = 0x0180;  // minor elevation ring green (15°/45°/75°) — dimmer
constexpr uint16_t kText      = 0xC618;  // label light grey

}  // namespace

void SkyView::begin(LovyanGFX& gfx) {
    // 8-bit color depth: 240×240 × 1 byte = 57,600 bytes — fits after WiFi/TLS heap usage.
    // Colors are converted to RGB332 in the sprite and dithered back to RGB565 on pushSprite.
    // Our palette (grey rings, green/orange/yellow/red dots) survives 8-bit quantization well.
    sprite_.setColorDepth(8);
    ready_ = sprite_.createSprite(240, 240);
}

void SkyView::render(LovyanGFX& gfx, const SatelliteStore& store) {
    // Fall back to direct draw if sprite allocation failed — avoids black screen.
    LovyanGFX* canvas = ready_ ? (LovyanGFX*)&sprite_ : &gfx;
    if (!ready_) gfx.startWrite();
    uint32_t nowMs = millis();

    auto& s = *canvas;  // draw target alias

    // Clear the full sprite buffer first so satellite dots/labels that bleed outside
    // the disc rim don't accumulate across frames. Safe with a sprite — the display
    // only sees the completed frame via pushSprite(), not intermediate draw calls.
    s.fillScreen(0x0000);
    s.fillCircle((int)kCx, (int)kCy, (int)kR, kBg);

    if (!ship_.isActive()) {
        // Minor rings at 15°/45°/75° drawn first (dimmer); major rings at 0°/30°/60° on top.
        for (int el : {15, 45, 75}) {
            float d = (1.0f - el / 90.0f) * kR;
            s.drawCircle((int)kCx, (int)kCy, (int)d, kRingMinor);
        }
        for (int el : {0, 30, 60}) {
            float d = (1.0f - el / 90.0f) * kR;
            s.drawCircle((int)kCx, (int)kCy, (int)d, kRing);
        }

        // Cardinal labels inside the rim — placing them outside would clip on the round panel.
        s.setTextColor(kText);
        s.setTextDatum(textdatum_t::middle_center);
        const struct { const char* l; int az; } card[] = {
            {"N", 0}, {"E", 90}, {"S", 180}, {"W", 270}
        };
        for (auto& c : card) {
            auto p = projection::polarPoint(0, c.az, kCx, kCy, kR + 4.0f);
            s.drawString(c.l, (int)p.x, (int)p.y);
        }
    }

    // Satellite dots: filled = used, hollow = unused, + PRN label.
    // Label is placed toward the centre from the dot so it never clips the round bezel.
    s.setTextColor(kText);
    for (const auto& sat : store.satellites()) {
        auto p   = projection::polarPoint(sat.elevation, sat.azimuth, kCx, kCy, kR);
        uint16_t col = snrcolor::toRgb565(snrcolor::classify(sat.used, sat.snr));
        if (ship_.isActive()) {
            // Knuth multiplicative hash of PRN → stable pseudo-random speed + direction per satellite.
            // speed in [0.3, 1.3] rad/s; direction from a separate hash bit.
            uint32_t h   = (uint32_t)sat.prn * 2654435761u;
            float speed  = 0.3f + ((float)(h & 0xFF) / 255.0f) * 1.0f;
            float dir    = (h & 0x100) ? 1.0f : -1.0f;
            float rotAngle = (float)nowMs * 0.001f * speed * dir;
            asteroid_shape::draw(s, (int)p.x, (int)p.y, sat.prn, col, rotAngle);
        } else if (sat.used) {
            s.fillCircle((int)p.x, (int)p.y, kDot, col);
        } else {
            s.drawCircle((int)p.x, (int)p.y, kDot, col);
        }
        if (!ship_.isActive()) {
            char label[8];  // PRN fits in 3 chars; 8 is safe headroom
            snprintf(label, sizeof(label), "%d", sat.prn);
            // Draw label on the inward-facing side of the dot so it stays inside the bezel.
            if (p.x >= kCx) {
                s.setTextDatum(textdatum_t::middle_right);
                s.drawString(label, (int)p.x - kDot - 2, (int)p.y);
            } else {
                s.setTextDatum(textdatum_t::middle_left);
                s.drawString(label, (int)p.x + kDot + 4, (int)p.y);
            }
        }
    }

    if (!ship_.isActive()) {
        // Status readout: Zulu time top, sat counts + fix bottom — symmetric offsets from centre.
        s.setTextDatum(textdatum_t::middle_center);
        s.setTextColor(kText);
        s.drawString(store.time().c_str(), (int)kCx, (int)(kCy - 58));
        char line[32];
        snprintf(line, sizeof(line), "%d used / %d vis",
                 store.satUsed(), store.satVisible());
        s.drawString(line, (int)kCx, (int)(kCy + 58));
        s.drawString(store.fix().c_str(), (int)kCx, (int)(kCy + 72));
    }

    // Draw Easter egg ship onto canvas before committing the frame.
    ship_.tick(*canvas, store, nowMs);

    // "c 1979 ATARI INC" fades over 5 s after spawn; "GAME OVER" flashes after explosion.
    ship_.drawCopyright(*canvas, nowMs);
    ship_.drawGameOver(*canvas, nowMs);

    // Push completed frame to display — or close direct-draw transaction if in fallback mode.
    if (ready_) sprite_.pushSprite(&gfx, 0, 0);
    else        gfx.endWrite();
}
