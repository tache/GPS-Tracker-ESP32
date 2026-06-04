// Claude Generated: version 1 - Central in-memory state for live satellites and trail history
#pragma once
#include <string>
#include <vector>
#include "model/Satellite.h"
#include "store/TrailBuffer.h"

class SatelliteStore {
 public:
    static constexpr uint32_t kTrailIntervalSec = 60;
    static constexpr uint32_t kTrailWindowSec   = 15 * 60;  // 15 min

    // Updates live satellite array and (throttled) trail history.
    // nowSec is seconds since boot — passed explicitly for deterministic testing.
    void updateSky(const SkyData& d, uint32_t nowSec);
    void updateFix(const std::string& fix) { fix_ = fix; }
    void setOnline(bool on)                { online_ = on; }

    const std::vector<Satellite>& satellites() const { return satellites_; }
    int  satUsed()    const { return satUsed_; }
    int  satVisible() const { return satVisible_; }
    const std::string& fix()    const { return fix_; }
    bool online()               const { return online_; }
    const TrailBuffer& trail()  const { return trail_; }

 private:
    std::vector<Satellite> satellites_;
    int satUsed_    = 0;
    int satVisible_ = 0;
    std::string fix_  = "--";
    bool online_      = false;
    TrailBuffer trail_;
    uint32_t lastTrailWriteSec_ = 0;
    bool trailEverWritten_      = false;
};
