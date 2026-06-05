# ESP32 Circular Skyview — Implementation Plan

> **For agentic workers:** Use superpowers:subagent-driven-development or
> superpowers:executing-plans to implement this plan task-by-task. Steps use
> checkbox (`- [ ]`) syntax for tracking.
>
> **PROJECT GIT POLICY (overrides the sub-skills):** FishDaddy performs ALL git
> operations. Do NOT run `git add`, `git commit`, `git mv`, `git rm`, or
> `git push`. Every task ends in a **Checkpoint** where work is handed to
> FishDaddy for review + commit. Remove/skip any "Commit" step the sub-skills
> inject.
>
> **BUILD/TEST EXECUTION:** Commands are shown for each step. Confirm with
> FishDaddy who runs them. Native logic tests run on the Mac via
> `pio test -e native`; firmware builds via `pio run -e esp32-c3`.

**Goal:** A standalone ESP32-C3 + round GC9A01A display that subscribes to the
GPS broadcaster's MQTT `sky`/`tpv` topics and renders a live satellite skyview,
porting the Mac app's polar graph.

**Architecture:** Pure-C++ core (projection, color, JSON parse, trail buffer,
store, router) that is unit-testable under the PlatformIO `native` env, with a
thin Arduino shell (WiFi, TLS MQTT via 256dpi/arduino-mqtt, LovyanGFX renderer,
`main`) verified on hardware.

**Tech Stack:** PlatformIO, Arduino-ESP32, LovyanGFX, 256dpi `MQTT`, ArduinoJson
7, Unity (native tests).

**Spec:** `claude-working-documents/20260603-ESP32-Circular-Skyview-Design.md`

---

## File Structure

| Path                              | Responsibility                                       | Tested                  |
| --------------------------------- | ---------------------------------------------------- | ----------------------- |
| `platformio.ini`                  | Envs: `esp32-c3` (firmware) + `native` (tests), deps | —                       |
| `.gitignore`                      | Add `src/secrets.h`                                  | —                       |
| `src/config/Pins.h`               | GPIO constants from the wiring diagram (committed)   | —                       |
| `src/secrets.h.example`           | Template for WiFi/MQTT/CA (committed)                | —                       |
| `src/secrets.h`                   | Real credentials (git-ignored)                       | —                       |
| `src/view/Projection.h`           | `polarPoint(el,az,radius)` — pure math               | ✅ native               |
| `src/view/SnrColor.h`             | `classify(used,snr)` + `toRgb565()` — pure           | ✅ native               |
| `src/model/Satellite.h`           | `Satellite` struct + `SkyData`                       | ✅ native               |
| `src/model/SkyMessage.h/.cpp`     | `parseSky(json)→SkyData`                             | ✅ native               |
| `src/model/TpvMessage.h/.cpp`     | `parseTpv(json)→fix string`                          | ✅ native               |
| `src/store/TrailBuffer.h/.cpp`    | Per-PRN ring, 15-min eviction, visible filter        | ✅ native               |
| `src/store/SatelliteStore.h/.cpp` | Live sats, counts, fix, 60-s trail cadence           | ✅ native               |
| `src/app/MessageRouter.h/.cpp`    | topic→parser→store dispatch (the MQTT seam)          | ✅ native (integration) |
| `src/net/WiFiLink.h/.cpp`         | WiFi connect/reconnect                               | manual HW               |
| `src/net/MqttClient.h/.cpp`       | TLS MQTT, subscribe, LWT → MessageRouter             | manual HW               |
| `src/view/SkyView.h/.cpp`         | LovyanGFX render of store                            | manual HW               |
| `src/main.cpp`                    | Display init + wiring + loop                         | manual HW (e2e)         |
| `test/test_*/`                    | Unity native tests + mock-MQTT integration           | —                       |

**Design seam for testing:** all time-dependent logic takes an explicit
`uint32_t nowSec` (seconds since boot) instead of calling `millis()`, so tests
are deterministic. The Arduino shell passes `millis()/1000`.

---

## Task 0: Project scaffolding

**Files:**

- Create: `platformio.ini`, `src/config/Pins.h`, `src/secrets.h.example`
- Modify: `.gitignore`

- [ ] **Step 1: Create `platformio.ini`**

```ini
[env]
build_flags = -std=gnu++17
build_unflags = -std=gnu++11

[env:esp32-c3]
platform = espressif32
board = esp32-c3-devkitm-1      ; ESP32-C3 Super Mini; adjust if needed
framework = arduino
monitor_speed = 115200
lib_deps =
    lovyan03/LovyanGFX@^1.1.16
    256dpi/MQTT@^2.5.3
    bblanchon/ArduinoJson@^7.0.0

[env:native]
platform = native
test_framework = unity
build_flags = -std=gnu++17
lib_deps =
    bblanchon/ArduinoJson@^7.0.0
```

- [ ] **Step 2: Add `src/config/Pins.h`** (from `documents/esp32-GC9A01A-wiring.png`)

```cpp
#pragma once
// GC9A01A on ESP32-C3 Super Mini (SPI, write-only, no backlight pin).
// VCC -> 3V3 (NOT 5V), GND -> GND.
namespace pins {
constexpr int kDisplayRst  = 0;   // RST
constexpr int kDisplayCs   = 1;   // CS
constexpr int kDisplayDc   = 10;  // DC
constexpr int kDisplayMosi = 3;   // SDA (MOSI)
constexpr int kDisplaySclk = 4;   // SCL (SCLK)
}  // namespace pins
```

- [ ] **Step 3: Add `src/secrets.h.example`**

```cpp
#pragma once
// Copy to src/secrets.h (git-ignored) and fill in.
#define WIFI_SSID       "your-ssid"
#define WIFI_PASSWORD   "your-pass"
#define MQTT_HOST       "your-ha-host"
#define MQTT_PORT       8883
#define MQTT_USER       "your-mqtt-user"
#define MQTT_PASSWORD   "your-mqtt-pass"
#define MQTT_CLIENT_ID  "gps_skyview_esp32"
// PEM string of the broker CA certificate:
static const char* MQTT_CA_CERT = R"EOF(
-----BEGIN CERTIFICATE-----
...broker CA here...
-----END CERTIFICATE-----
)EOF";
```

- [ ] **Step 4: Append to `.gitignore`**

```
# Build-time credentials (never commit)
src/secrets.h
.pio/
```

- [ ] **Step 5: Verify the native env builds**

Run: `pio test -e native` (expect "no tests" / clean configure, deps resolve)
Expected: PlatformIO downloads ArduinoJson and reports no tests yet.

- [ ] **Checkpoint:** Hand to FishDaddy. Confirm `board` id matches the actual
      Super Mini and the broker CA is available before continuing.

---

## Task 1: Projection (pure math)

**Files:**

- Create: `src/view/Projection.h`
- Test: `test/test_projection/test_main.cpp`

Reference: `../GPS-Tracker/GPS Tracker/Views/PolarGraphView.swift` `polarPoint`.

- [ ] **Step 1: Write the failing test**

```cpp
#include <unity.h>
#include "view/Projection.h"

void test_zenith_maps_to_center() {
    auto p = projection::polarPoint(90, 123, 120, 120, 108);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 120.0, p.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 120.0, p.y);
}

void test_north_horizon_is_top() {
    auto p = projection::polarPoint(0, 0, 120, 120, 108);  // el=0 az=0 (N)
    TEST_ASSERT_FLOAT_WITHIN(0.01, 120.0, p.x);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 12.0, p.y);  // cy - radius
}

void test_east_horizon_is_right() {
    auto p = projection::polarPoint(0, 90, 120, 120, 108);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 228.0, p.x);  // cx + radius
    TEST_ASSERT_FLOAT_WITHIN(0.5, 120.0, p.y);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_zenith_maps_to_center);
    RUN_TEST(test_north_horizon_is_top);
    RUN_TEST(test_east_horizon_is_right);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test, verify it fails**

Run: `pio test -e native -f test_projection`
Expected: FAIL — `Projection.h` not found.

- [ ] **Step 3: Implement `src/view/Projection.h`**

```cpp
#pragma once
#include <cmath>

namespace projection {
struct Point { float x; float y; };

// Center = zenith (el 90), rim = horizon (el 0), north up, screen y-down.
inline Point polarPoint(int elevation, int azimuth,
                        float cx, float cy, float radius) {
    const float dist = (1.0f - static_cast<float>(elevation) / 90.0f) * radius;
    const float azRad = static_cast<float>(azimuth) * 3.14159265358979f / 180.0f;
    return { cx + dist * std::sin(azRad), cy - dist * std::cos(azRad) };
}
}  // namespace projection
```

- [ ] **Step 4: Run test, verify it passes**

Run: `pio test -e native -f test_projection`
Expected: PASS (3 tests).

- [ ] **Checkpoint:** Hand to FishDaddy.

---

## Task 2: SNR color classification

**Files:**

- Create: `src/view/SnrColor.h`
- Test: `test/test_snrcolor/test_main.cpp`

Reference: `../GPS-Tracker/GPS Tracker/Models/Satellite.swift` `color`.

- [ ] **Step 1: Write the failing test**

```cpp
#include <unity.h>
#include "view/SnrColor.h"
using namespace snrcolor;

void test_unused_is_red()        { TEST_ASSERT_EQUAL(SatColor::Red,    classify(false, 50)); }
void test_used_strong_is_green() { TEST_ASSERT_EQUAL(SatColor::Green,  classify(true, 35)); }
void test_used_mid_is_orange()   { TEST_ASSERT_EQUAL(SatColor::Orange, classify(true, 20)); }
void test_used_weak_is_yellow()  { TEST_ASSERT_EQUAL(SatColor::Yellow, classify(true, 19)); }
void test_rgb565_values() {
    TEST_ASSERT_EQUAL_HEX16(0xF800, toRgb565(SatColor::Red));
    TEST_ASSERT_EQUAL_HEX16(0x07E0, toRgb565(SatColor::Green));
    TEST_ASSERT_EQUAL_HEX16(0xFD20, toRgb565(SatColor::Orange));
    TEST_ASSERT_EQUAL_HEX16(0xFFE0, toRgb565(SatColor::Yellow));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_unused_is_red);
    RUN_TEST(test_used_strong_is_green);
    RUN_TEST(test_used_mid_is_orange);
    RUN_TEST(test_used_weak_is_yellow);
    RUN_TEST(test_rgb565_values);
    return UNITY_END();
}
```

- [ ] **Step 2: Run, verify fails** — `pio test -e native -f test_snrcolor` → FAIL.

- [ ] **Step 3: Implement `src/view/SnrColor.h`**

```cpp
#pragma once
#include <cstdint>

namespace snrcolor {
enum class SatColor { Red, Green, Orange, Yellow };

// Exact port: !used -> Red; ss>=35 -> Green; ss>=20 -> Orange; else Yellow.
inline SatColor classify(bool used, int snr) {
    if (!used) return SatColor::Red;
    if (snr >= 35) return SatColor::Green;
    if (snr >= 20) return SatColor::Orange;
    return SatColor::Yellow;
}

inline uint16_t toRgb565(SatColor c) {
    switch (c) {
        case SatColor::Red:    return 0xF800;  // 255,0,0
        case SatColor::Green:  return 0x07E0;  // 0,255,0
        case SatColor::Orange: return 0xFD20;  // 255,165,0
        case SatColor::Yellow: return 0xFFE0;  // 255,255,0
    }
    return 0xFFFF;
}
}  // namespace snrcolor
```

- [ ] **Step 4: Run, verify passes** — PASS (5 tests).

- [ ] **Checkpoint:** Hand to FishDaddy.

---

## Task 3: Satellite model + sky parser

**Files:**

- Create: `src/model/Satellite.h`, `src/model/SkyMessage.h`, `src/model/SkyMessage.cpp`
- Test: `test/test_skymessage/test_main.cpp`

Reference payload: `../NTP-GPS-PPS-MQTT-HA/docs/mqtt-gps-messages.md`.

- [ ] **Step 1: Write the failing test**

```cpp
#include <unity.h>
#include "model/SkyMessage.h"

static const char* kSky = R"({
  "sat_used": 2, "sat_visible": 2,
  "satellites": [
    {"prn":5,"el":72,"az":214,"ss":42,"used":true,"seen":312},
    {"prn":29,"el":18,"az":47,"ss":22,"used":false,"seen":88}
  ]})";

void test_parses_counts_and_sats() {
    SkyData out;
    TEST_ASSERT_TRUE(parseSky(kSky, out));
    TEST_ASSERT_EQUAL_INT(2, out.satUsed);
    TEST_ASSERT_EQUAL_INT(2, out.satVisible);
    TEST_ASSERT_EQUAL_INT(2, (int)out.satellites.size());
    TEST_ASSERT_EQUAL_INT(5, out.satellites[0].prn);
    TEST_ASSERT_EQUAL_INT(42, out.satellites[0].snr);   // ss -> snr
    TEST_ASSERT_TRUE(out.satellites[0].used);
    TEST_ASSERT_FALSE(out.satellites[1].used);
}

void test_rejects_garbage() {
    SkyData out;
    TEST_ASSERT_FALSE(parseSky("not json", out));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_parses_counts_and_sats);
    RUN_TEST(test_rejects_garbage);
    return UNITY_END();
}
```

- [ ] **Step 2: Run, verify fails.**

- [ ] **Step 3: Implement model + parser**

`src/model/Satellite.h`:

```cpp
#pragma once
#include <cstdint>
#include <vector>

struct Satellite {
    int prn;
    int elevation;  // 0-90
    int azimuth;    // 0-359 true north
    int snr;        // dBHz (from JSON "ss")
    bool used;
};

struct SkyData {
    int satUsed = 0;
    int satVisible = 0;
    std::vector<Satellite> satellites;
};
```

`src/model/SkyMessage.h`:

```cpp
#pragma once
#include "model/Satellite.h"
bool parseSky(const char* json, SkyData& out);
```

`src/model/SkyMessage.cpp`:

```cpp
#include "model/SkyMessage.h"
#include <ArduinoJson.h>

bool parseSky(const char* json, SkyData& out) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return false;
    out = SkyData{};
    out.satUsed = doc["sat_used"] | 0;
    out.satVisible = doc["sat_visible"] | 0;
    for (JsonObject s : doc["satellites"].as<JsonArray>()) {
        out.satellites.push_back(Satellite{
            s["prn"] | 0, s["el"] | 0, s["az"] | 0,
            s["ss"] | 0, s["used"] | false });
    }
    return true;
}
```

- [ ] **Step 4: Run, verify passes.**

- [ ] **Checkpoint:** Hand to FishDaddy.

---

## Task 4: TPV (fix status) parser

**Files:**

- Create: `src/model/TpvMessage.h`, `src/model/TpvMessage.cpp`
- Test: `test/test_tpvmessage/test_main.cpp`

- [ ] **Step 1: Failing test**

```cpp
#include <unity.h>
#include "model/TpvMessage.h"

void test_extracts_fix() {
    std::string fix;
    TEST_ASSERT_TRUE(parseTpv(R"({"fix":"3D Fix","lat":37.7})", fix));
    TEST_ASSERT_EQUAL_STRING("3D Fix", fix.c_str());
}
void test_no_fix_defaults() {
    std::string fix;
    TEST_ASSERT_TRUE(parseTpv(R"({"lat":null})", fix));
    TEST_ASSERT_EQUAL_STRING("No Fix", fix.c_str());
}
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_extracts_fix);
    RUN_TEST(test_no_fix_defaults);
    return UNITY_END();
}
```

- [ ] **Step 2: Run, verify fails.**

- [ ] **Step 3: Implement**

`src/model/TpvMessage.h`:

```cpp
#pragma once
#include <string>
bool parseTpv(const char* json, std::string& fixOut);
```

`src/model/TpvMessage.cpp`:

```cpp
#include "model/TpvMessage.h"
#include <ArduinoJson.h>

bool parseTpv(const char* json, std::string& fixOut) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return false;
    fixOut = doc["fix"] | "No Fix";
    return true;
}
```

- [ ] **Step 4: Run, verify passes.**

- [ ] **Checkpoint:** Hand to FishDaddy.

---

## Task 5: Trail buffer (15-min ring, per PRN)

**Files:**

- Create: `src/store/TrailBuffer.h`, `src/store/TrailBuffer.cpp`
- Test: `test/test_trailbuffer/test_main.cpp`

Reference fade/visible rules: `PolarGraphView.drawTrails`. Window = 15 min
(900 s) per the spec's embedded deviation.

- [ ] **Step 1: Failing test**

```cpp
#include <unity.h>
#include "store/TrailBuffer.h"

void test_appends_and_orders() {
    TrailBuffer t;
    t.append(5, {72,214,42,true,0});
    t.append(5, {73,215,41,true,60});
    TEST_ASSERT_EQUAL_INT(2, (int)t.samples(5).size());
    TEST_ASSERT_EQUAL_INT(0,  t.samples(5)[0].tsSec);
    TEST_ASSERT_EQUAL_INT(60, t.samples(5)[1].tsSec);
}

void test_evicts_older_than_window() {
    TrailBuffer t;
    t.append(5, {72,214,42,true,0});
    t.append(5, {73,215,41,true,1000});  // 1000s later
    t.evictOlderThan(1000, 900);          // window 900s
    TEST_ASSERT_EQUAL_INT(1, (int)t.samples(5).size());
    TEST_ASSERT_EQUAL_INT(1000, t.samples(5)[0].tsSec);
}

void test_drops_prns_out_of_view() {
    TrailBuffer t;
    t.append(5, {72,214,42,true,0});
    t.append(9, {10,40,18,false,0});
    t.retainOnly({5});
    TEST_ASSERT_EQUAL_INT(1, (int)t.samples(5).size());
    TEST_ASSERT_EQUAL_INT(0, (int)t.samples(9).size());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_appends_and_orders);
    RUN_TEST(test_evicts_older_than_window);
    RUN_TEST(test_drops_prns_out_of_view);
    return UNITY_END();
}
```

- [ ] **Step 2: Run, verify fails.**

- [ ] **Step 3: Implement**

`src/store/TrailBuffer.h`:

```cpp
#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <vector>

struct TrailSample {
    uint8_t elevation;
    uint16_t azimuth;
    uint8_t snr;
    bool used;
    uint32_t tsSec;
};

class TrailBuffer {
 public:
    static constexpr int kMaxPerPrn = 20;  // ~15 min at 60 s cadence + margin

    void append(int prn, const TrailSample& s);
    void evictOlderThan(uint32_t nowSec, uint32_t windowSec);
    void retainOnly(const std::set<int>& visiblePrns);

    const std::vector<TrailSample>& samples(int prn) const;
    std::vector<int> prns() const;

 private:
    std::map<int, std::vector<TrailSample>> byPrn_;
    static const std::vector<TrailSample> kEmpty_;
};
```

`src/store/TrailBuffer.cpp`:

```cpp
#include "store/TrailBuffer.h"

const std::vector<TrailSample> TrailBuffer::kEmpty_{};

void TrailBuffer::append(int prn, const TrailSample& s) {
    auto& v = byPrn_[prn];
    v.push_back(s);
    if (static_cast<int>(v.size()) > kMaxPerPrn) v.erase(v.begin());
}

void TrailBuffer::evictOlderThan(uint32_t nowSec, uint32_t windowSec) {
    for (auto& [prn, v] : byPrn_) {
        while (!v.empty() && nowSec - v.front().tsSec > windowSec) v.erase(v.begin());
    }
}

void TrailBuffer::retainOnly(const std::set<int>& visiblePrns) {
    for (auto it = byPrn_.begin(); it != byPrn_.end();) {
        if (visiblePrns.count(it->first) == 0) it = byPrn_.erase(it);
        else ++it;
    }
}

const std::vector<TrailSample>& TrailBuffer::samples(int prn) const {
    auto it = byPrn_.find(prn);
    return it == byPrn_.end() ? kEmpty_ : it->second;
}

std::vector<int> TrailBuffer::prns() const {
    std::vector<int> out;
    for (auto& [prn, v] : byPrn_) out.push_back(prn);
    return out;
}
```

- [ ] **Step 4: Run, verify passes.**

- [ ] **Checkpoint:** Hand to FishDaddy.

---

## Task 6: SatelliteStore (60-s trail cadence)

**Files:**

- Create: `src/store/SatelliteStore.h`, `src/store/SatelliteStore.cpp`
- Test: `test/test_store/test_main.cpp`

Cadence (`SatelliteStore` writes history every 60 s) ports the Mac app's
`maybeWriteHistory` throttle.

- [ ] **Step 1: Failing test**

```cpp
#include <unity.h>
#include "store/SatelliteStore.h"

static SkyData sky2() {
    SkyData d; d.satUsed = 1; d.satVisible = 2;
    d.satellites = {{5,72,214,42,true},{9,18,47,22,false}};
    return d;
}

void test_updates_live_and_counts() {
    SatelliteStore s;
    s.updateSky(sky2(), 0);
    TEST_ASSERT_EQUAL_INT(2, (int)s.satellites().size());
    TEST_ASSERT_EQUAL_INT(1, s.satUsed());
    TEST_ASSERT_EQUAL_INT(2, s.satVisible());
}

void test_trail_written_once_per_minute() {
    SatelliteStore s;
    s.updateSky(sky2(), 0);     // first write
    s.updateSky(sky2(), 30);    // <60s later: no new trail point
    TEST_ASSERT_EQUAL_INT(1, (int)s.trail().samples(5).size());
    s.updateSky(sky2(), 60);    // 60s: new point
    TEST_ASSERT_EQUAL_INT(2, (int)s.trail().samples(5).size());
}

void test_fix_and_online() {
    SatelliteStore s;
    s.updateFix("3D Fix");
    s.setOnline(true);
    TEST_ASSERT_EQUAL_STRING("3D Fix", s.fix().c_str());
    TEST_ASSERT_TRUE(s.online());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_updates_live_and_counts);
    RUN_TEST(test_trail_written_once_per_minute);
    RUN_TEST(test_fix_and_online);
    return UNITY_END();
}
```

- [ ] **Step 2: Run, verify fails.**

- [ ] **Step 3: Implement**

`src/store/SatelliteStore.h`:

```cpp
#pragma once
#include <string>
#include <vector>
#include "model/Satellite.h"
#include "store/TrailBuffer.h"

class SatelliteStore {
 public:
    static constexpr uint32_t kTrailIntervalSec = 60;
    static constexpr uint32_t kTrailWindowSec = 15 * 60;  // 15 min

    void updateSky(const SkyData& d, uint32_t nowSec);
    void updateFix(const std::string& fix) { fix_ = fix; }
    void setOnline(bool on) { online_ = on; }

    const std::vector<Satellite>& satellites() const { return satellites_; }
    int satUsed() const { return satUsed_; }
    int satVisible() const { return satVisible_; }
    const std::string& fix() const { return fix_; }
    bool online() const { return online_; }
    const TrailBuffer& trail() const { return trail_; }

 private:
    std::vector<Satellite> satellites_;
    int satUsed_ = 0;
    int satVisible_ = 0;
    std::string fix_ = "--";
    bool online_ = false;
    TrailBuffer trail_;
    uint32_t lastTrailWriteSec_ = 0;
    bool trailWritten_ = false;
};
```

`src/store/SatelliteStore.cpp`:

```cpp
#include "store/SatelliteStore.h"
#include <set>

void SatelliteStore::updateSky(const SkyData& d, uint32_t nowSec) {
    satellites_ = d.satellites;
    satUsed_ = d.satUsed;
    satVisible_ = d.satVisible;

    const bool due = !trailWritten_ || (nowSec - lastTrailWriteSec_) >= kTrailIntervalSec;
    if (due) {
        for (const auto& s : satellites_) {
            trail_.append(s.prn, TrailSample{
                static_cast<uint8_t>(s.elevation),
                static_cast<uint16_t>(s.azimuth),
                static_cast<uint8_t>(s.snr), s.used, nowSec});
        }
        lastTrailWriteSec_ = nowSec;
        trailWritten_ = true;
    }

    std::set<int> visible;
    for (const auto& s : satellites_) visible.insert(s.prn);
    trail_.retainOnly(visible);
    trail_.evictOlderThan(nowSec, kTrailWindowSec);
}
```

- [ ] **Step 4: Run, verify passes.**

- [ ] **Checkpoint:** Hand to FishDaddy.

---

## Task 7: MessageRouter + mock-MQTT integration test

**Files:**

- Create: `src/app/MessageRouter.h`, `src/app/MessageRouter.cpp`
- Test: `test/test_integration/test_main.cpp`

This is the **mock MQTT integration test** from the spec: canned payloads →
router → parsers → store, asserting end-to-end data flow with no network/display.

- [ ] **Step 1: Failing integration test**

```cpp
#include <unity.h>
#include "app/MessageRouter.h"

static const char* kSky =
    R"({"sat_used":1,"sat_visible":2,"satellites":[
        {"prn":5,"el":72,"az":214,"ss":42,"used":true,"seen":1},
        {"prn":9,"el":18,"az":47,"ss":22,"used":false,"seen":1}]})";

void test_sky_then_tpv_then_avail_flow() {
    SatelliteStore store;
    MessageRouter router(store);

    router.route("gps_monitor/sky", kSky, 0);
    router.route("gps_monitor/tpv", R"({"fix":"3D Fix"})", 0);
    router.route("gps_monitor/availability", "online", 0);

    TEST_ASSERT_EQUAL_INT(2, (int)store.satellites().size());
    TEST_ASSERT_EQUAL_INT(1, store.satUsed());
    TEST_ASSERT_EQUAL_STRING("3D Fix", store.fix().c_str());
    TEST_ASSERT_TRUE(store.online());

    // A minute later, a second sky message grows the trail.
    router.route("gps_monitor/sky", kSky, 60);
    TEST_ASSERT_EQUAL_INT(2, (int)store.trail().samples(5).size());
}

void test_offline_payload() {
    SatelliteStore store;
    MessageRouter router(store);
    router.route("gps_monitor/availability", "offline", 0);
    TEST_ASSERT_FALSE(store.online());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_sky_then_tpv_then_avail_flow);
    RUN_TEST(test_offline_payload);
    return UNITY_END();
}
```

- [ ] **Step 2: Run, verify fails.**

- [ ] **Step 3: Implement**

`src/app/MessageRouter.h`:

```cpp
#pragma once
#include <string>
#include "store/SatelliteStore.h"

class MessageRouter {
 public:
    explicit MessageRouter(SatelliteStore& store) : store_(store) {}
    // topic: full MQTT topic; payload: raw JSON / text; nowSec: seconds uptime.
    void route(const std::string& topic, const std::string& payload, uint32_t nowSec);

 private:
    SatelliteStore& store_;
};
```

`src/app/MessageRouter.cpp`:

```cpp
#include "app/MessageRouter.h"
#include "model/SkyMessage.h"
#include "model/TpvMessage.h"

static bool endsWith(const std::string& s, const char* suffix) {
    const std::string suf(suffix);
    return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

void MessageRouter::route(const std::string& topic, const std::string& payload,
                          uint32_t nowSec) {
    if (endsWith(topic, "/sky")) {
        SkyData d;
        if (parseSky(payload.c_str(), d)) store_.updateSky(d, nowSec);
    } else if (endsWith(topic, "/tpv")) {
        std::string fix;
        if (parseTpv(payload.c_str(), fix)) store_.updateFix(fix);
    } else if (endsWith(topic, "/availability")) {
        store_.setOnline(payload == "online");
    }
}
```

- [ ] **Step 4: Run, verify passes.** This proves the full data path.

- [ ] **Checkpoint:** Hand to FishDaddy. **The entire testable core is now green.**

---

## Task 8: WiFiLink (hardware)

**Files:**

- Create: `src/net/WiFiLink.h`, `src/net/WiFiLink.cpp`

No native test (Arduino dependency); verified on hardware in Task 11.

- [ ] **Step 1: Implement**

`src/net/WiFiLink.h`:

```cpp
#pragma once
class WiFiLink {
 public:
    void begin(const char* ssid, const char* pass);
    void loop();          // call from main loop; reconnects if dropped
    bool connected() const;
};
```

`src/net/WiFiLink.cpp`:

```cpp
#include "net/WiFiLink.h"
#include <WiFi.h>

static const char* s_ssid = nullptr;
static const char* s_pass = nullptr;

void WiFiLink::begin(const char* ssid, const char* pass) {
    s_ssid = ssid; s_pass = pass;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
}

void WiFiLink::loop() {
    if (WiFi.status() != WL_CONNECTED && s_ssid) WiFi.reconnect();
}

bool WiFiLink::connected() const { return WiFi.status() == WL_CONNECTED; }
```

- [ ] **Step 2: Build firmware** — `pio run -e esp32-c3` → compiles clean.

- [ ] **Checkpoint:** Hand to FishDaddy.

---

## Task 9: MqttClient (TLS, 256dpi/arduino-mqtt)

**Files:**

- Create: `src/net/MqttClient.h`, `src/net/MqttClient.cpp`

Forwards each message to `MessageRouter`. RX buffer sized to 4 KB for the ~2 KB
`sky` payload. No native test; verified on hardware in Task 11.

- [ ] **Step 1: Implement**

`src/net/MqttClient.h`:

```cpp
#pragma once
#include <functional>
#include <string>

class MqttClient {
 public:
    using Handler = std::function<void(const std::string& topic,
                                       const std::string& payload)>;
    void begin(const char* host, int port, const char* caCert,
               const char* clientId, const char* user, const char* pass,
               Handler onMessage);
    void loop();           // pumps MQTT + reconnects/subscribes
    bool connected() const;
};
```

`src/net/MqttClient.cpp`:

```cpp
#include "net/MqttClient.h"
#include <MQTT.h>               // 256dpi/arduino-mqtt
#include <WiFiClientSecure.h>

namespace {
WiFiClientSecure s_net;
MQTTClient s_mqtt(4096);        // 4 KB RX/TX buffer for ~2 KB sky payloads
MqttClient::Handler s_handler;
std::string s_host, s_clientId, s_user, s_pass;
int s_port = 8883;

void onMqtt(String& topic, String& payload) {
    if (s_handler) s_handler(std::string(topic.c_str()), std::string(payload.c_str()));
}

void ensureConnected() {
    if (s_mqtt.connected()) return;
    // LWT: mirror the broadcaster's availability convention.
    s_mqtt.setWill("gps_skyview/availability", "offline", true, 1);
    if (s_mqtt.connect(s_clientId.c_str(), s_user.c_str(), s_pass.c_str())) {
        s_mqtt.subscribe("gps_monitor/sky");
        s_mqtt.subscribe("gps_monitor/tpv");
        s_mqtt.subscribe("gps_monitor/availability");
    }
}
}  // namespace

void MqttClient::begin(const char* host, int port, const char* caCert,
                       const char* clientId, const char* user, const char* pass,
                       Handler onMessage) {
    s_host = host; s_port = port; s_clientId = clientId;
    s_user = user; s_pass = pass; s_handler = std::move(onMessage);
    s_net.setCACert(caCert);
    s_mqtt.begin(s_host.c_str(), s_port, s_net);
    s_mqtt.onMessage(onMqtt);
}

void MqttClient::loop() {
    s_mqtt.loop();
    ensureConnected();
}

bool MqttClient::connected() const { return s_mqtt.connected(); }
```

- [ ] **Step 2: Build firmware** — `pio run -e esp32-c3` → compiles clean.

- [ ] **Checkpoint:** Hand to FishDaddy. Verify the 256dpi `MQTT` API names
      (`setWill`, `onMessage`, buffer ctor) against the installed version.

---

## Task 10: SkyView renderer (LovyanGFX)

**Files:**

- Create: `src/view/SkyView.h`, `src/view/SkyView.cpp`

Direct port of `PolarGraphView` draw routines. No native test; verified on
hardware in Task 11.

- [ ] **Step 1: Implement**

`src/view/SkyView.h`:

```cpp
#pragma once
#include <LovyanGFX.hpp>
#include "store/SatelliteStore.h"

class SkyView {
 public:
    void render(LovyanGFX& gfx, const SatelliteStore& store);
};
```

`src/view/SkyView.cpp`:

```cpp
#include "view/SkyView.h"
#include "view/Projection.h"
#include "view/SnrColor.h"

namespace {
constexpr float kCx = 120, kCy = 120, kR = 108;
constexpr int kDot = 6;
constexpr uint16_t kBg = 0x10A2;     // dark grey disc
constexpr uint16_t kRing = 0x4208;   // grid grey
constexpr uint16_t kLabel = 0xC618;  // light grey text

// Blend an RGB565 color toward the background by (1-opacity), for trail fade.
uint16_t fade(uint16_t c, float opacity) {
    auto split = [](uint16_t v, int& r, int& g, int& b) {
        r = (v >> 11) & 0x1F; g = (v >> 5) & 0x3F; b = v & 0x1F;
    };
    int r1, g1, b1, r2, g2, b2;
    split(c, r1, g1, b1); split(kBg, r2, g2, b2);
    int r = r2 + (int)((r1 - r2) * opacity);
    int g = g2 + (int)((g1 - g2) * opacity);
    int b = b2 + (int)((b1 - b2) * opacity);
    return (uint16_t)((r << 11) | (g << 5) | b);
}
}  // namespace

void SkyView::render(LovyanGFX& gfx, const SatelliteStore& store) {
    gfx.startWrite();
    gfx.fillScreen(0x0000);
    gfx.fillCircle(kCx, kCy, kR, kBg);

    // Elevation rings at 0/30/60.
    for (int el : {0, 30, 60}) {
        float d = (1.0f - el / 90.0f) * kR;
        gfx.drawCircle(kCx, kCy, d, kRing);
    }

    // Cardinals, offset 12 px outside the rim.
    gfx.setTextColor(kLabel);
    gfx.setTextDatum(textdatum_t::middle_center);
    const struct { const char* l; int az; } card[] = {
        {"N", 0}, {"E", 90}, {"S", 180}, {"W", 270}};
    for (auto& c : card) {
        auto p = projection::polarPoint(0, c.az, kCx, kCy, kR + 12);
        gfx.drawString(c.l, p.x, p.y);
    }

    // Trails (SNR-colored, visible PRNs only, per-segment fade).
    const auto& trail = store.trail();
    for (int prn : trail.prns()) {
        const auto& pts = trail.samples(prn);
        if (pts.size() < 2) continue;
        for (size_t i = 1; i < pts.size(); ++i) {
            const auto& a = pts[i - 1];
            const auto& b = pts[i];
            auto pa = projection::polarPoint(a.elevation, a.azimuth, kCx, kCy, kR);
            auto pb = projection::polarPoint(b.elevation, b.azimuth, kCx, kCy, kR);
            uint32_t newest = pts.back().tsSec;
            float age = (float)(newest - b.tsSec);
            float opacity = std::max(0.2f, 1.0f - age / (float)SatelliteStore::kTrailWindowSec);
            uint16_t col = fade(snrcolor::toRgb565(snrcolor::classify(b.used, b.snr)), opacity);
            gfx.drawLine(pa.x, pa.y, pb.x, pb.y, col);
        }
    }

    // Satellites: filled (used) / hollow (unused) + PRN label.
    for (const auto& s : store.satellites()) {
        auto p = projection::polarPoint(s.elevation, s.azimuth, kCx, kCy, kR);
        uint16_t col = snrcolor::toRgb565(snrcolor::classify(s.used, s.snr));
        if (s.used) gfx.fillCircle(p.x, p.y, kDot, col);
        else        gfx.drawCircle(p.x, p.y, kDot, col);
        gfx.setTextColor(kLabel);
        gfx.setTextDatum(textdatum_t::middle_left);
        gfx.drawString(String(s.prn), p.x + kDot + 4, p.y);
    }

    // Readout: "U/V" and fix near the bottom inside the disc (tune on HW).
    gfx.setTextDatum(textdatum_t::middle_center);
    gfx.setTextColor(kLabel);
    char line[32];
    snprintf(line, sizeof(line), "%d used / %d vis", store.satUsed(), store.satVisible());
    gfx.drawString(line, kCx, kCy + 58);
    gfx.drawString(store.fix().c_str(), kCx, kCy + 72);

    gfx.endWrite();
}
```

- [ ] **Step 2: Build firmware** — `pio run -e esp32-c3` → compiles clean.

- [ ] **Checkpoint:** Hand to FishDaddy.

---

## Task 11: main.cpp — display init + wiring + e2e

**Files:**

- Create: `src/main.cpp`

This is the manual end-to-end tier from the spec.

- [ ] **Step 1: Implement display config + setup/loop**

`src/main.cpp`:

```cpp
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "config/Pins.h"
#include "secrets.h"
#include "app/MessageRouter.h"
#include "net/MqttClient.h"
#include "net/WiFiLink.h"
#include "store/SatelliteStore.h"
#include "view/SkyView.h"

// LovyanGFX config for GC9A01A on ESP32-C3 (SPI, write-only).
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_GC9A01 panel_;
    lgfx::Bus_SPI bus_;
 public:
    LGFX() {
        { auto c = bus_.config();
          c.spi_host = SPI2_HOST; c.spi_mode = 0;
          c.freq_write = 40000000;
          c.pin_sclk = pins::kDisplaySclk; c.pin_mosi = pins::kDisplayMosi;
          c.pin_miso = -1; c.pin_dc = pins::kDisplayDc;
          bus_.config(c); panel_.setBus(&bus_); }
        { auto c = panel_.config();
          c.pin_cs = pins::kDisplayCs; c.pin_rst = pins::kDisplayRst;
          c.panel_width = 240; c.panel_height = 240;
          c.offset_x = 0; c.offset_y = 0;
          panel_.config(c); }
        setPanel(&panel_);
    }
};

LGFX gfx;
SatelliteStore store;
MessageRouter router(store);
WiFiLink wifi;
MqttClient mqtt;
SkyView view;
uint32_t lastRenderMs = 0;

void setup() {
    Serial.begin(115200);
    gfx.init();
    gfx.setRotation(0);
    gfx.fillScreen(0x0000);

    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    mqtt.begin(MQTT_HOST, MQTT_PORT, MQTT_CA_CERT, MQTT_CLIENT_ID,
               MQTT_USER, MQTT_PASSWORD,
               [](const std::string& topic, const std::string& payload) {
                   router.route(topic, payload, millis() / 1000);
               });
}

void loop() {
    wifi.loop();
    mqtt.loop();
    if (millis() - lastRenderMs >= 1000) {   // redraw ~1 Hz
        view.render(gfx, store);
        lastRenderMs = millis();
    }
}
```

- [ ] **Step 2: Build + upload** — `pio run -e esp32-c3 -t upload` (FishDaddy).

- [ ] **Step 3: Manual hardware verification**

Observe on the device and confirm:

- Display powers on, dark disc + 3 rings + N/E/S/W render.
- Satellites appear at plausible az/el; **used = filled, unused = hollow**.
- Colors follow green≥35 / orange≥20 / yellow<20 / red-unused.
- PRN labels sit beside dots.
- Trails extend over a few minutes; segments fade with age.
- Readout shows used/visible counts and the fix string.
- Pull WiFi/broker: device reconnects and resumes without reboot.

- [ ] **Step 4: Capture serial log**

Run: `pio device monitor -b 115200` and confirm no error spew / no malloc
failures during steady-state.

- [ ] **Checkpoint:** Hand to FishDaddy for final on-device sign-off.

---

## Done criteria

- All `native` tests green: `pio test -e native`.
- Firmware builds clean: `pio run -e esp32-c3`.
- On hardware: live skyview matches the Mac app's conventions (Task 11 Step 3).
- `src/secrets.h` git-ignored; `secrets.h.example` committed.
