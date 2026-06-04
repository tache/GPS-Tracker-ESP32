// Claude Generated: version 1 - MQTT topic dispatcher: routes payloads to parsers and store
#pragma once
#include <string>
#include "store/SatelliteStore.h"

class MessageRouter {
 public:
    explicit MessageRouter(SatelliteStore& store) : store_(store) {}

    // Dispatches an MQTT message to the appropriate parser and store update.
    // nowSec: seconds since boot, passed through for deterministic trail cadence.
    void route(const std::string& topic, const std::string& payload, uint32_t nowSec);

 private:
    SatelliteStore& store_;
};
