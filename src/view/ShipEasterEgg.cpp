// src/view/ShipEasterEgg.cpp
// Claude Generated: version 1 - Asteroids ship Easter egg: tick loop and LovyanGFX drawing
// Claude Generated: version 2 - Set cooldown to 30 seconds between appearances
// Claude Generated: version 3 - Collision detection and debris explosion animation
// Claude Generated: version 4 - "c 1979 ATARI INC" vector copyright on spawn, fades after 3 s
// Claude Generated: version 5 - ROM-faithful explosion: 6 fixed pieces with ROM velocity table
// Claude Generated: version 6 - Time-based physics (px/s, rad/s) for frame-rate independence
// Claude Generated: version 7 - Slower ship, more drift; GAME OVER flash after explosion
// Claude Generated: version 8 - Intro phase: copyright fades fully before ship enters
#include "view/ShipEasterEgg.h"
#include "view/VectorFont.h"
#include "view/Projection.h"
#include <cstdlib>
#include <cmath>
#include <esp_random.h>

namespace {
constexpr uint32_t kCooldownMinMs     = 58000;   // ~1 min between appearances
constexpr uint32_t kCooldownMaxMs     = 62000;   // slight variance so it feels organic
constexpr uint32_t kExplodeDurationMs = 4000;    // 4 s debris animation
constexpr uint32_t kIntroDurationMs    = 5000;   // 3 s full + 2 s fade before ship enters
constexpr uint32_t kGameOverDurationMs = 3000;   // 3 s GAME OVER flash after explosion
constexpr float    kCollisionRadius   = 15.0f;   // px center-to-center; ship ~6 + asteroid ~8
constexpr float    kSpeedPps          = 35.0f;   // ship speed in px/s — slow glide across screen
constexpr float    kMaxTurnRadPerSec  = 0.8f;    // max heading change in rad/s — lazy drift
constexpr int      kPieceCount        = 6;

// ROM explosion data from address 10E0/10EC (Asteroids VectorROM).
// Shape: each piece is a fixed line segment (dx, dy) — ROM SVEC values scaled ×0.7 to match
//        our 12 px ship. Velocity: ROM table at 10EC scaled ×0.5 → px/s on our 240 px display.
static constexpr struct { float vx, vy, dx, dy; } kPieces[kPieceCount] = {
    { -20.0f,  15.0f,  -6.0f, -8.0f },  // ROM: vel(-40, 30), shape(-8,-12)
    {  25.0f, -10.0f,   3.0f, -6.0f },  // ROM: vel( 50,-20), shape( 4, -8)
    {   0.0f, -30.0f,   4.0f,  1.0f },  // ROM: vel(  0,-60), shape( 6,  2)
    {  30.0f,  10.0f,  -6.0f,  6.0f },  // ROM: vel( 60, 20), shape(-8,  8)
    {   5.0f,  35.0f,  -4.0f,  1.0f },  // ROM: vel( 10, 70), shape(-6,  2)
    { -20.0f, -20.0f,   3.0f, -3.0f },  // ROM: vel(-40,-40), shape( 4, -4)
};
constexpr uint16_t kShipColor         = 0xFFFF;  // white — Asteroids vector aesthetic
}

using namespace ship_math;

static uint32_t randomCooldown() {
    return kCooldownMinMs +
           (uint32_t)(rand() % (kCooldownMaxMs - kCooldownMinMs));
}

ShipEasterEgg::ShipEasterEgg() {
    srand(esp_random());       // hardware RNG — different seed each boot
    cooldownMs_ = randomCooldown();
}

void ShipEasterEgg::tick(LovyanGFX& canvas,
                          const SatelliteStore& store,
                          uint32_t nowMs) {
    // Explosion in progress — draw debris until duration elapses, then enter GAME OVER.
    if (exploding_) {
        if (nowMs - explodeStartMs_ >= kExplodeDurationMs) {
            exploding_   = false;
            gameOver_    = true;
            gameOverMs_  = nowMs;
        } else {
            drawExplosion(canvas, nowMs);
        }
        return;
    }

    // GAME OVER phase — text drawn by drawGameOver(); tick just manages the timer.
    if (gameOver_) {
        if (nowMs - gameOverMs_ >= kGameOverDurationMs) {
            gameOver_    = false;
            lastExitMs_  = nowMs;
            cooldownMs_  = randomCooldown();
        }
        return;
    }

    // Intro phase: asteroid mode is active, copyright is showing/fading, ship not yet present.
    // spawnMs_ was set when intro started so the copyright fade math in drawCopyright() works.
    if (intro_) {
        if (nowMs - introStartMs_ >= kIntroDurationMs) {
            intro_ = false;
            spawn(nowMs);
        }
        return;
    }

    if (!state_.active) {
        // unsigned subtraction is rollover-safe (uint32_t arithmetic)
        if (nowMs - lastExitMs_ >= cooldownMs_) {
            intro_        = true;
            introStartMs_ = nowMs;
            spawnMs_      = nowMs;  // copyright timing anchored to intro start
        }
        return;
    }

    // Exit check BEFORE drawing — prevents off-screen artifact
    if (isOutsideBounds(state_.pos)) {
        state_.active = false;
        lastExitMs_   = nowMs;
        cooldownMs_   = randomCooldown();
        return;
    }

    // Build satellite screen positions for avoidance and collision detection
    std::vector<Point2f> sats;
    sats.reserve(store.satellites().size());
    for (const auto& sat : store.satellites()) {
        auto p = projection::polarPoint(sat.elevation, sat.azimuth,
                                        kCx, kCy, kEntryR);  // kCx/kCy/kEntryR from ShipSteering.h
        sats.push_back({p.x, p.y});
    }

    // Delta-time: cap at 200 ms so a late frame can't teleport the ship.
    float dtSec = (float)(nowMs - lastTickMs_) / 1000.0f;
    lastTickMs_ = nowMs;
    if (dtSec > 0.2f) dtSec = 0.2f;

    // Steer and advance — all rates in per-second units, scaled by dtSec.
    // Capture heading delta before the turn so thrust reflects whether steering was needed.
    float desired      = computeDesiredHeading(state_, sats);
    float headingDelta = fabsf(wrapAngle(desired - state_.heading));
    state_.heading = stepHeading(state_.heading, desired, kMaxTurnRadPerSec * dtSec);
    state_.pos.x  += kSpeedPps * dtSec * cosf(state_.heading);
    state_.pos.y  += kSpeedPps * dtSec * sinf(state_.heading);

    // Collision check: if ship center overlaps any asteroid, trigger explosion
    for (const auto& sat : sats) {
        float dx = state_.pos.x - sat.x;
        float dy = state_.pos.y - sat.y;
        if (dx * dx + dy * dy < kCollisionRadius * kCollisionRadius) {
            triggerExplosion(state_.pos.x, state_.pos.y, nowMs);
            drawExplosion(canvas, nowMs);  // initial starburst burst this frame
            return;
        }
    }

    // Thrust on while actively steering; off when aligned and gliding.
    // 0.2 rad (~11°) threshold gives long coasting phases with short bursts on course changes.
    state_.thrustOn = (headingDelta > 0.2f);
    frameCount_++;

    draw(canvas);
}

void ShipEasterEgg::spawn(uint32_t nowMs) {
    float entryAngle = (float(rand()) / float(RAND_MAX)) * 2.0f * 3.14159265f;
    // exitAngleDelta in [pi/2, 3pi/2] — opposite half of disc
    float exitDelta  = 3.14159265f / 2.0f +
                       (float(rand()) / float(RAND_MAX)) * 3.14159265f;
    state_       = makeSpawnedState(entryAngle, exitDelta);
    frameCount_  = 0;
    lastTickMs_  = nowMs;
}

void ShipEasterEgg::rotatePoint(float px, float py, float angle,
                                 float cx, float cy,
                                 int& outX, int& outY) {
    float c = cosf(angle);
    float s = sinf(angle);
    outX = (int)(cx + c * px - s * py);
    outY = (int)(cy + s * px + c * py);
}

void ShipEasterEgg::draw(LovyanGFX& canvas) const {
    drawShip(canvas);
    if (state_.thrustOn) drawFlame(canvas);
}

void ShipEasterEgg::drawShip(LovyanGFX& canvas) const {
    float cx = state_.pos.x;
    float cy = state_.pos.y;
    // Ship body is defined nose-up (local -y = forward). Heading 0 = +x (right).
    // Rotate body by (heading + pi/2) so local nose aligns with heading direction.
    // Verify: heading=0 (+x), nose=(0,-6) → outX=cx+6, outY=cy → correct (points right).
    float a = state_.heading + 3.14159265f / 2.0f;
    int nx, ny, lx, ly, rx, ry, bx, by;
    rotatePoint(  0.0f, -6.0f, a, cx, cy, nx, ny);  // nose
    rotatePoint( -5.0f,  4.0f, a, cx, cy, lx, ly);  // left wing
    rotatePoint(  5.0f,  4.0f, a, cx, cy, rx, ry);  // right wing
    rotatePoint(  0.0f,  2.0f, a, cx, cy, bx, by);  // rear notch
    canvas.drawLine(nx, ny, lx, ly, kShipColor);
    canvas.drawLine(nx, ny, rx, ry, kShipColor);
    canvas.drawLine(lx, ly, bx, by, kShipColor);
    canvas.drawLine(rx, ry, bx, by, kShipColor);
}

void ShipEasterEgg::drawFlame(LovyanGFX& canvas) const {
    float cx = state_.pos.x;
    float cy = state_.pos.y;
    float a  = state_.heading + 3.14159265f / 2.0f;
    int flx, fly, frx, fry, ftx, fty;
    rotatePoint(-2.0f, 4.0f, a, cx, cy, flx, fly);  // flame left
    rotatePoint( 2.0f, 4.0f, a, cx, cy, frx, fry);  // flame right
    rotatePoint( 0.0f, 9.0f, a, cx, cy, ftx, fty);  // flame tip
    canvas.drawLine(flx, fly, frx, fry, kShipColor);
    canvas.drawLine(flx, fly, ftx, fty, kShipColor);
    canvas.drawLine(frx, fry, ftx, fty, kShipColor);
}

void ShipEasterEgg::triggerExplosion(float cx, float cy, uint32_t nowMs) {
    state_.active    = false;
    exploding_       = true;
    explodeStartMs_  = nowMs;
    explodeCx_       = cx;
    explodeCy_       = cy;
    // Piece shapes and velocities are ROM-fixed (kPieces table); no per-explosion state needed.
}

void ShipEasterEgg::drawExplosion(LovyanGFX& canvas, uint32_t nowMs) const {
    float t     = (float)(nowMs - explodeStartMs_) / 1000.0f;
    float alpha = 1.0f - t / (kExplodeDurationMs / 1000.0f);
    if (alpha <= 0.0f) return;
    uint16_t col = debrisColor(alpha);
    // Each of the 6 ROM pieces translates with its velocity; shape is the fixed ROM segment.
    for (int i = 0; i < kPieceCount; ++i) {
        float px = explodeCx_ + kPieces[i].vx * t;
        float py = explodeCy_ + kPieces[i].vy * t;
        canvas.drawLine((int)px,                      (int)py,
                        (int)(px + kPieces[i].dx),    (int)(py + kPieces[i].dy),
                        col);
    }
}

uint16_t ShipEasterEgg::debrisColor(float alpha) {
    // Fade white (0xFFFF) toward black by scaling each RGB565 channel
    int r = (int)(31.0f * alpha);
    int g = (int)(63.0f * alpha);
    int b = (int)(31.0f * alpha);
    return (uint16_t)((r << 11) | (g << 5) | b);
}

void ShipEasterEgg::drawCopyright(LovyanGFX& canvas, uint32_t nowMs) const {
    if (!isActive() || gameOver_) return;
    float t = (float)(nowMs - spawnMs_) / 1000.0f;
    float alpha;
    if (t < 3.0f) {
        alpha = 1.0f;
    } else if (t < 5.0f) {
        alpha = 1.0f - (t - 3.0f) / 2.0f;
    } else {
        return;  // fully faded
    }
    uint16_t col = debrisColor(alpha);
    // "ASTEROIDS" title at 2× scale centred above the copyright line.
    vector_font::drawStringCentered(canvas, (int)kCx, (int)(kCy - 78),
                                    "ASTEROIDS", col, 2);
    // kCx/kCy from ship_math namespace (via using directive above); -58 matches Zulu time position
    vector_font::drawStringCentered(canvas, (int)kCx, (int)(kCy - 58),
                                    "c 1979 ATARI INC", col);
}

void ShipEasterEgg::drawGameOver(LovyanGFX& canvas, uint32_t nowMs) const {
    if (!gameOver_) return;
    uint32_t elapsed = nowMs - gameOverMs_;
    if (elapsed >= kGameOverDurationMs) return;
    // Flash at 1 Hz: visible on even 500 ms intervals, dark on odd intervals.
    if ((elapsed / 500) % 2 == 0) {
        vector_font::drawStringCentered(canvas, (int)kCx, (int)(kCy - 58),
                                        "GAME OVER", 0xFFFF);
    }
}
