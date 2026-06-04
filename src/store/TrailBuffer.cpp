// Claude Generated: version 1 - TrailBuffer implementation
#include "store/TrailBuffer.h"

const std::vector<TrailSample> TrailBuffer::kEmpty_{};

void TrailBuffer::append(int prn, const TrailSample& s) {
    auto& v = byPrn_[prn];
    v.push_back(s);
    if (static_cast<int>(v.size()) > kMaxPerPrn) v.erase(v.begin());
}

void TrailBuffer::evictOlderThan(uint32_t nowSec, uint32_t windowSec) {
    for (auto& [prn, v] : byPrn_) {
        while (!v.empty() && nowSec - v.front().tsSec > windowSec)
            v.erase(v.begin());
    }
}

void TrailBuffer::retainOnly(const std::set<int>& visiblePrns) {
    for (auto it = byPrn_.begin(); it != byPrn_.end();) {
        if (visiblePrns.count(it->first) == 0)
            it = byPrn_.erase(it);
        else
            ++it;
    }
}

const std::vector<TrailSample>& TrailBuffer::samples(int prn) const {
    auto it = byPrn_.find(prn);
    return it == byPrn_.end() ? kEmpty_ : it->second;
}

std::vector<int> TrailBuffer::prns() const {
    std::vector<int> out;
    for (const auto& [prn, v] : byPrn_) out.push_back(prn);
    return out;
}
