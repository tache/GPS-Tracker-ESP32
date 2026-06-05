// Claude Generated: version 1 - SkyView render implementation (port of PolarGraphView.swift)
// Claude Generated: version 2 - Sprite double-buffering to eliminate per-frame flicker
#include "view/SkyView.h"
#include "view/Projection.h"
#include "view/SnrColor.h"
#include <algorithm>
#include <cstdio>

namespace {

// Display is 240×240; centre is (120,120).
// kR = 0.45*240 = 108 — leaves ~12 px margin so the 12-px-offset cardinal labels don't clip.
// kBg/kRing are dark greys chosen to be visible but non-distracting on the GC9A01A panel.
constexpr float    kCx   = 120.0f;
constexpr float    kCy   = 120.0f;
constexpr float    kR    = 108.0f;
constexpr int      kDot  = 6;
constexpr uint16_t kBg        = 0x10A2;  // dark grey disc background
constexpr uint16_t kRing      = 0x4208;  // major elevation ring grey (0°/30°/60°)
constexpr uint16_t kRingMinor = 0x2104;  // minor elevation ring grey (15°/45°/75°) — dimmer
constexpr uint16_t kText      = 0xC618;  // label light grey

// Blends RGB565 color toward background by (1-opacity) for trail fade.
uint16_t fadeColor(uint16_t c, float opacity) {
    auto ch = [](uint16_t v, int shift, int mask) {
        return (v >> shift) & mask;
    };
    int r1 = ch(c,   11, 0x1F), g1 = ch(c,  5, 0x3F), b1 = ch(c, 0, 0x1F);
    int r2 = ch(kBg, 11, 0x1F), g2 = ch(kBg, 5, 0x3F), b2 = ch(kBg, 0, 0x1F);
    int r  = r2 + (int)((r1 - r2) * opacity);
    int g  = g2 + (int)((g1 - g2) * opacity);
    int b  = b2 + (int)((b1 - b2) * opacity);
    return (uint16_t)((r << 11) | (g << 5) | b);
}

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

    auto& s = *canvas;  // draw target alias

    // Clear the full sprite buffer first so satellite dots/labels that bleed outside
    // the disc rim don't accumulate across frames. Safe with a sprite — the display
    // only sees the completed frame via pushSprite(), not intermediate draw calls.
    s.fillScreen(0x0000);
    s.fillCircle((int)kCx, (int)kCy, (int)kR, kBg);

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

    // Trails: light grey segments, visible PRNs only, per-segment time fade.
    const auto& trail = store.trail();
    for (int prn : trail.prns()) {
        const auto& pts = trail.samples(prn);
        if (pts.size() < 2) continue;
        uint32_t newest = pts.back().tsSec;
        for (size_t i = 1; i < pts.size(); ++i) {
            const auto& a = pts[i - 1];
            const auto& b = pts[i];
            auto pa = projection::polarPoint(a.elevation, a.azimuth, kCx, kCy, kR);
            auto pb = projection::polarPoint(b.elevation, b.azimuth, kCx, kCy, kR);
            float age     = (float)(newest - b.tsSec);
            float opacity = std::max(0.2f, 1.0f - age / (float)SatelliteStore::kTrailWindowSec);
            uint16_t col  = fadeColor(kText, opacity);
            s.drawLine((int)pa.x, (int)pa.y, (int)pb.x, (int)pb.y, col);
        }
    }

    // Satellite dots: filled = used, hollow = unused, + PRN label.
    // Label is placed toward the centre from the dot so it never clips the round bezel.
    s.setTextColor(kText);
    for (const auto& sat : store.satellites()) {
        auto p   = projection::polarPoint(sat.elevation, sat.azimuth, kCx, kCy, kR);
        uint16_t col = snrcolor::toRgb565(snrcolor::classify(sat.used, sat.snr));
        if (sat.used) s.fillCircle((int)p.x, (int)p.y, kDot, col);
        else          s.drawCircle((int)p.x, (int)p.y, kDot, col);
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

    // Status readout: Zulu time top, sat counts + fix bottom — symmetric offsets from centre.
    s.setTextDatum(textdatum_t::middle_center);
    s.setTextColor(kText);
    s.drawString(store.time().c_str(), (int)kCx, (int)(kCy - 58));
    char line[32];
    snprintf(line, sizeof(line), "%d used / %d vis",
             store.satUsed(), store.satVisible());
    s.drawString(line, (int)kCx, (int)(kCy + 58));
    s.drawString(store.fix().c_str(), (int)kCx, (int)(kCy + 72));

    // Push completed frame to display — or close direct-draw transaction if in fallback mode.
    if (ready_) sprite_.pushSprite(&gfx, 0, 0);
    else        gfx.endWrite();
}
