// Claude Generated: version 1 - LovyanGFX polar sky view renderer for GC9A01A 240x240
// Claude Generated: version 2 - Add sprite double-buffering to eliminate per-frame flicker
#pragma once
#include <LovyanGFX.hpp>
#include "store/SatelliteStore.h"

class SkyView {
 public:
    // Allocates the off-screen sprite used for flicker-free rendering.
    // Must be called once from setup() after gfx.init().
    void begin(LovyanGFX& gfx);

    // Renders a complete frame into the sprite then pushes it atomically to the display.
    void render(LovyanGFX& gfx, const SatelliteStore& store);

 private:
    LGFX_Sprite sprite_;
    bool        ready_ = false;
};
