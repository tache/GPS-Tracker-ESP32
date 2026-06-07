// Claude Generated: version 1 - LovyanGFX polar sky view renderer for GC9A01A 240x240
// Claude Generated: version 2 - Add sprite double-buffering to eliminate per-frame flicker
// Claude Generated: version 3 - Wire in ShipEasterEgg member
// Claude Generated: version 4 - Expose isAnimating() so main loop can boost to 20 Hz
#pragma once
#include <LovyanGFX.hpp>
#include "store/SatelliteStore.h"
#include "view/ShipEasterEgg.h"

class SkyView {
 public:
    // Allocates the off-screen sprite used for flicker-free rendering.
    // Must be called once from setup() after gfx.init().
    void begin(LovyanGFX& gfx);

    // Renders a complete frame into the sprite then pushes it atomically to the display.
    void render(LovyanGFX& gfx, const SatelliteStore& store);

    // True while the Easter egg ship is flying or exploding.
    // main.cpp uses this to boost the render loop to 20 Hz during animation.
    bool isAnimating() const { return ship_.isActive(); }

 private:
    LGFX_Sprite  sprite_;
    bool         ready_ = false;
    ShipEasterEgg ship_;
};
