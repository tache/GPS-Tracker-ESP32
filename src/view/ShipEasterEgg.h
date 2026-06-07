// src/view/ShipEasterEgg.h
// Claude Generated: version 1 - Asteroids ship Easter egg: timer, steering, LovyanGFX drawing
// Claude Generated: version 2 - Add isActive() so SkyView can hide clutter during ship run
// Claude Generated: version 3 - Collision detection and debris explosion animation
// Claude Generated: version 4 - "c 1979 ATARI INC" vector copyright on spawn, fades after 3 s
// Claude Generated: version 5 - ROM-faithful explosion: 6 fixed pieces with ROM velocity table
// Claude Generated: version 6 - Time-based physics (px/s, rad/s) for frame-rate independence
// Claude Generated: version 7 - Spinning asteroids; GAME OVER flash; slower ship with more drift
// Claude Generated: version 8 - Intro phase: copyright shows and fades before ship enters
#pragma once
#include <LovyanGFX.hpp>
#include "store/SatelliteStore.h"
#include "view/ShipSteering.h"

class ShipEasterEgg {
 public:
    // Seeds RNG with esp_random() and sets initial cooldown.
    ShipEasterEgg();

    // Called once per render frame. Advances timer, steers ship, draws onto canvas.
    // canvas: the active draw target (sprite or gfx in fallback mode).
    // store:  live satellite data for avoidance and collision detection.
    // nowMs:  current millis() — passed in to allow testing.
    void tick(LovyanGFX& canvas, const SatelliteStore& store, uint32_t nowMs);

    // True during intro, flying, exploding, or GAME OVER — keeps SkyView in asteroid mode at 20 Hz.
    bool isActive() const { return intro_ || state_.active || exploding_ || gameOver_; }

    // Draws "c 1979 ATARI INC" in vector font where Zulu time normally sits.
    // Visible for 3 s at full brightness, then fades over 2 s. No-op when not active or in GAME OVER.
    void drawCopyright(LovyanGFX& canvas, uint32_t nowMs) const;

    // Flashes "GAME OVER" in vector font at the same position as drawCopyright.
    // Active only during the GAME OVER phase that follows an explosion.
    void drawGameOver(LovyanGFX& canvas, uint32_t nowMs) const;

 private:
    ship_math::ShipState state_;
    uint32_t lastExitMs_  = 0;
    uint32_t cooldownMs_  = 0;
    uint32_t frameCount_  = 0;
    uint32_t spawnMs_     = 0;  // millis() when the ship most recently spawned
    uint32_t lastTickMs_  = 0;  // millis() of the previous tick — used for deltaTime physics

    // Explosion state — active from collision until pieces fade out
    bool     exploding_      = false;
    uint32_t explodeStartMs_ = 0;
    float    explodeCx_      = 0.0f;
    float    explodeCy_      = 0.0f;

    // Intro state — asteroid mode active, copyright showing, ship not yet spawned
    bool     intro_          = false;
    uint32_t introStartMs_   = 0;

    // GAME OVER state — active after explosion ends, before next cooldown begins
    bool     gameOver_       = false;
    uint32_t gameOverMs_     = 0;

    void spawn(uint32_t nowMs);
    void draw(LovyanGFX& canvas) const;
    void drawShip(LovyanGFX& canvas) const;
    void drawFlame(LovyanGFX& canvas) const;
    void triggerExplosion(float cx, float cy, uint32_t nowMs);
    void drawExplosion(LovyanGFX& canvas, uint32_t nowMs) const;

    // Rotate local point (px,py) by angle around (cx,cy), output screen int coords.
    static void rotatePoint(float px, float py, float angle,
                             float cx, float cy,
                             int& outX, int& outY);
    // Fade white toward black by alpha [0,1].
    static uint16_t debrisColor(float alpha);
};
