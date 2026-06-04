// Claude Generated: version 1 - SatelliteStore implementation
#include "store/SatelliteStore.h"
#include <set>

void SatelliteStore::updateSky(const SkyData& d, uint32_t nowSec) {
    satellites_  = d.satellites;
    satUsed_     = d.satUsed;
    satVisible_  = d.satVisible;

    // Write trail at most once per kTrailIntervalSec (mirrors Mac app maybeWriteHistory throttle).
    const bool due = !trailEverWritten_ ||
                     (nowSec - lastTrailWriteSec_) >= kTrailIntervalSec;
    if (due) {
        for (const auto& s : satellites_) {
            trail_.append(s.prn, TrailSample{
                static_cast<uint8_t>(s.elevation),
                static_cast<uint16_t>(s.azimuth),
                static_cast<uint8_t>(s.snr),
                s.used,
                nowSec
            });
        }
        lastTrailWriteSec_ = nowSec;
        trailEverWritten_  = true;
    }

    // Drop history for satellites that have left view, and prune the time window.
    std::set<int> visible;
    for (const auto& s : satellites_) visible.insert(s.prn);
    trail_.retainOnly(visible);
    trail_.evictOlderThan(nowSec, kTrailWindowSec);
}
