// Claude Generated: version 1 - Per-PRN satellite trail ring buffer with time-window eviction
#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <vector>

struct TrailSample {
    uint8_t  elevation;
    uint16_t azimuth;
    uint8_t  snr;
    bool     used;
    uint32_t tsSec;
};

class TrailBuffer {
 public:
    // 15 min at 60 s cadence with a small margin for timing jitter.
    static constexpr int kMaxPerPrn = 20;

    void append(int prn, const TrailSample& s);
    void evictOlderThan(uint32_t nowSec, uint32_t windowSec);
    void retainOnly(const std::set<int>& visiblePrns);

    const std::vector<TrailSample>& samples(int prn) const;
    std::vector<int> prns() const;

 private:
    std::map<int, std::vector<TrailSample>> byPrn_;
    static const std::vector<TrailSample> kEmpty_;
};
