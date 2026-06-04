// Claude Generated: version 1 - SkyView render implementation (port of PolarGraphView.swift)
#include "view/SkyView.h"
#include "view/Projection.h"
#include "view/SnrColor.h"
#include <algorithm>
#include <cstdio>

namespace {

constexpr float    kCx   = 120.0f;
constexpr float    kCy   = 120.0f;
constexpr float    kR    = 108.0f;   // 0.45 * 240
constexpr int      kDot  = 6;
constexpr uint16_t kBg   = 0x10A2;  // dark grey disc background
constexpr uint16_t kRing = 0x4208;  // elevation ring grey
constexpr uint16_t kText = 0xC618;  // label light grey

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

void SkyView::render(LovyanGFX& gfx, const SatelliteStore& store) {
    gfx.startWrite();

    // Black frame then dark disc.
    gfx.fillScreen(0x0000);
    gfx.fillCircle((int)kCx, (int)kCy, (int)kR, kBg);

    // Elevation rings at 0° (rim), 30°, 60°.
    for (int el : {0, 30, 60}) {
        float d = (1.0f - el / 90.0f) * kR;
        gfx.drawCircle((int)kCx, (int)kCy, (int)d, kRing);
    }

    // Cardinal labels offset 12 px outside the rim.
    gfx.setTextColor(kText);
    gfx.setTextDatum(textdatum_t::middle_center);
    const struct { const char* l; int az; } card[] = {
        {"N", 0}, {"E", 90}, {"S", 180}, {"W", 270}
    };
    for (auto& c : card) {
        auto p = projection::polarPoint(0, c.az, kCx, kCy, kR + 12.0f);
        gfx.drawString(c.l, (int)p.x, (int)p.y);
    }

    // Trails: SNR-colored segments, visible PRNs only, per-segment time fade.
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
            uint16_t col  = fadeColor(
                snrcolor::toRgb565(snrcolor::classify(b.used, b.snr)), opacity);
            gfx.drawLine((int)pa.x, (int)pa.y, (int)pb.x, (int)pb.y, col);
        }
    }

    // Satellite dots: filled = used, hollow = unused, + PRN label.
    gfx.setTextDatum(textdatum_t::middle_left);
    for (const auto& s : store.satellites()) {
        auto p   = projection::polarPoint(s.elevation, s.azimuth, kCx, kCy, kR);
        uint16_t col = snrcolor::toRgb565(snrcolor::classify(s.used, s.snr));
        if (s.used) gfx.fillCircle((int)p.x, (int)p.y, kDot, col);
        else        gfx.drawCircle((int)p.x, (int)p.y, kDot, col);
        gfx.setTextColor(kText);
        char label[8];
        snprintf(label, sizeof(label), "%d", s.prn);
        gfx.drawString(label, (int)p.x + kDot + 4, (int)p.y);
    }

    // Status readout: sat counts + fix string near the bottom of the disc.
    gfx.setTextDatum(textdatum_t::middle_center);
    gfx.setTextColor(kText);
    char line[32];
    snprintf(line, sizeof(line), "%d used / %d vis",
             store.satUsed(), store.satVisible());
    gfx.drawString(line, (int)kCx, (int)(kCy + 58));
    gfx.drawString(store.fix().c_str(), (int)kCx, (int)(kCy + 72));

    gfx.endWrite();
}
