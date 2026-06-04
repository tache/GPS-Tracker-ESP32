// Claude Generated: version 1 - LovyanGFX polar sky view renderer for GC9A01A 240x240
#pragma once
#include <LovyanGFX.hpp>
#include "store/SatelliteStore.h"

class SkyView {
 public:
    // Renders a full frame to gfx from the current store state.
    // Call at ~1 Hz; uses startWrite/endWrite for atomic SPI transfer.
    void render(LovyanGFX& gfx, const SatelliteStore& store);
};
