# ESP32 Circular Skyview — Design Spec

Date: 2026-06-03
Status: Draft for FishDaddy review
Project: GPS-Tracker-ESP32

## 1. Overview

A standalone hardware display that renders a live GPS satellite skyview on a
1.28" round LCD. It is a **third MQTT subscriber client** in the existing
GPS-Tracker family, alongside:

- **Mac app** — `../GPS-Tracker` (full-featured; the visual source of truth)
- **Home Assistant dashboard** — `../NTP-GPS-PPS-MQTT-HA`
- **Broadcaster** — `gpsd_monitor.py` in `../NTP-GPS-PPS-MQTT-HA`, publishes gpsd
  data to MQTT.

The device has **no onboard GPS**. It connects to WiFi, subscribes to the
broadcaster's TLS MQTT broker, parses the `gps_monitor/sky` payload, and draws
the polar skyview. Scope is intentionally narrow: **the circular sky view only**
— the Mac app does everything else.

Inspiration for the hardware/rendering approach: the ESP32-Plane-Radar project
(https://github.com/MatixYo/ESP32-Plane-Radar), but plotting satellites from
MQTT instead of ADS-B aircraft from an onboard receiver.

## 2. Hardware

| Item | Detail |
|------|--------|
| MCU | ESP32-C3 Super Mini |
| Display | 1.28" round GC9A01A IPS, 240×240, 4-line SPI (write-only, no MISO) |
| Driver library | LovyanGFX (GC9A01 driver) |
| Build system | PlatformIO (`espressif32`, arduino framework) |

### Pin map (from `documents/esp32-GC9A01A-wiring.png`)

| Display pin | ESP32-C3 GPIO |
|-------------|---------------|
| VCC | 3V3 — **not 5V** |
| GND | GND |
| RST | GPIO 0 |
| CS  | GPIO 1 |
| DC  | GPIO 10 |
| SDA (MOSI) | GPIO 3 |
| SCL (SCLK) | GPIO 4 |

No backlight (BL) control pin and no MISO are wired. Backlight is assumed
always-on (tied to 3V3). LovyanGFX is configured SPI-only with no backlight pin
and no read capability.

## 3. Architecture

Single-purpose firmware modules:

| Module | Responsibility | Depends on |
|--------|----------------|-----------|
| `secrets.h` (git-ignored) | WiFi SSID/pass, broker host/port, MQTT user/pass, CA cert, GPIO pin constants | — |
| `WiFiLink` | Connect and auto-reconnect WiFi | secrets |
| `MqttClient` | TLS connect, subscribe, Last-Will; deliver raw payloads via callback. Enlarged RX buffer (~4 KB) | WiFiLink, secrets |
| `SkyMessage` (parser) | ArduinoJson decode of `gps_monitor/sky` → `Satellite[]` + counts | ArduinoJson |
| `TpvMessage` (parser) | Decode `gps_monitor/tpv` → fix-status string | ArduinoJson |
| `SatelliteStore` | Current satellites, fix string, used/visible counts, and the trail ring buffer | — |
| `SkyView` (renderer) | LovyanGFX polar draw: grid, cardinals, trails, sat dots, readout | LovyanGFX, SatelliteStore |
| `main.cpp` | Display init, module wiring, loop | all |

### Data flow

```
MQTT msg ──> MqttClient callback ──> parser ──> SatelliteStore.update()
                                                      │
                          (every 60 s) append to trail ring buffer
                                                      │
   sky message received (≈1 Hz) ──> mark dirty ──> SkyView.render()
```

Rendering is triggered on receipt of a new `sky` message (≈1 Hz from gpsd), not
on a free-running frame loop, to keep the SPI bus and CPU quiet.

## 4. MQTT

| Topic | Use |
|-------|-----|
| `gps_monitor/sky` | Satellites array + `sat_used` / `sat_visible` counts (primary) |
| `gps_monitor/tpv` | `fix` string for the status readout |
| `gps_monitor/availability` | `online` / `offline` (retained) — show a disconnect indicator |

Broker is TLS on port 8883 with username/password (see
`../NTP-GPS-PPS-MQTT-HA/docs/mqtt-gps-messages.md`). Per-satellite fields used:
`prn`, `el`, `az`, `ss` (→ SNR dBHz), `used`. The `gnssid`/`svid` fields are
absent (MTK-3301 is GPS-only); PRN is treated as a unique key.

A `sky` payload with ~15 satellites is ~2 KB, so the MQTT client RX buffer and
the ArduinoJson document must be sized accordingly (target ~4 KB each).

## 5. Rendering — ported from the Mac app

Source of truth: `../GPS-Tracker/GPS Tracker/Views/PolarGraphView.swift` and
`Models/Satellite.swift`.

### Projection (center = zenith, rim = horizon, north up)

```
radius = 0.45 * 240            // 108 px on a 240×240 panel
dist   = (1 - elevation/90) * radius
x = cx + dist * sin(az * π/180)
y = cy - dist * cos(az * π/180)
```

### Color rule (exact port of `Satellite.color`)

| Condition | Color |
|-----------|-------|
| `!used` | Red (outline only) |
| `used && snr >= 35` | Green (filled) |
| `used && snr >= 20` | Orange (filled) |
| `used` (snr < 20) | Yellow (filled) |

Rendered as RGB565 equivalents of SwiftUI red/green/orange/yellow.

### Satellites

- Dot radius 6 px.
- **Used → filled circle. Unused → stroked outline (~1.5 px).**
- Small PRN label drawn beside each dot.

### Chrome (per FishDaddy's selections)

- **Elevation rings** at 0° (rim), 30°, 60°, plus center point at 90°.
- **N / E / S / W** cardinal labels just outside the rim.
- **Status readout:** "`N used / M visible`" (from `sky`) and the `fix` string
  (from `tpv`), placed to fit within the circular bezel.
- The screenshot's large "Satellite Skyview" title is **dropped** (no room on a
  round 240 px panel).

### Trail

Port of `PolarGraphView.drawTrails`:

- A line segment between consecutive recorded positions of a satellite.
- **SNR-colored** (colored mode), using the same color rule per segment.
- Drawn **only for PRNs currently in view**.
- Per-segment fade: `opacity = max(0.2, 1 - age/window)`.
- New positions recorded every **60 s** (matches the Mac app's history cadence).

## 6. Trail memory model (embedded deviation)

The Mac app persists 24 h of history to SwiftData on disk and fades over
86 400 s. That is not feasible in ESP32-C3 RAM. Decision:

- **In-RAM ring buffer, ~15 minutes per visible PRN** (≈15 points/PRN at the
  60 s cadence).
- Fade denominator set to the 15-minute window so fade stays visible.
- When a PRN leaves view, its ring is dropped.
- Memory: ~15 sats × 15 points × ~10 B ≈ a few KB.
- Future option (out of scope for MVP): persist to LittleFS to approach the full
  24 h window.

## 7. Provisioning

Build-time secrets via a **git-ignored `secrets.h`** (and/or PlatformIO
`build_flags`): WiFi SSID/pass, broker host/port, MQTT user/pass, broker CA
certificate, and the display GPIO constants. `.gitignore` updated to exclude
`secrets.h`. A `secrets.h.example` template is committed.

## 8. Libraries

| Library | Purpose |
|---------|---------|
| LovyanGFX | GC9A01 display driver + drawing primitives |
| ArduinoJson | Parse `sky` / `tpv` payloads |
| **256dpi `arduino-mqtt`** (`MQTTClient`) | MQTT subscribe over `WiFiClientSecure`, with RX buffer sized ≥ 4 KB at construction |
| WiFiClientSecure (core) | TLS transport with CA cert |

MQTT client decision (2026-06-03): **256dpi/arduino-mqtt**, chosen for the best
balance of FishDaddy's criteria — mature, actively maintained, popular. GitHub
data at selection time:

| Library | Stars | Latest release | Open issues |
|---------|-------|----------------|-------------|
| 256dpi/arduino-mqtt | 1092 | v2.5.3 (2026-04-16) | 24 |
| knolleary/PubSubClient | 4006 | v2.8 (2020-05-20) | 566 |
| bertmelis/espMqttClient | 119 | v1.7.2 (2025-10-30) | 1 |

PubSubClient is the most popular but unmaintained since 2020, and its 256-byte
default buffer is insufficient for the ~2 KB `sky` payload. arduino-mqtt allows
the RX buffer to be sized at construction.

## 9. Testing

PlatformIO `native` environment with the Unity framework.

- **Unit tests (pure logic):**
  - Color classification (`!used`/35/20 thresholds, boundary values).
  - Polar projection math (zenith→center, horizon→rim, N/E/S/W bearings).
  - JSON parsing of representative `sky` and `tpv` payloads, including the
    no-fix case and missing optional fields.
  - Trail ring buffer: append cadence, 15-minute eviction, drop-on-leave.
- **Integration test (mock MQTT, native):** feed canned `sky` payloads through
  `MqttClient` callback → parser → `SatelliteStore` and assert the resulting
  satellite set, counts, and trail state. No display involved.
- **Hardware / e2e:** manual — rendering, WiFi, and live MQTT/TLS verified on the
  physical device. Flagged explicitly as a manual tier (not "not applicable");
  no automated hardware-in-loop rig exists.

## 10. Out of scope (MVP)

- Multi-constellation coloring (receiver is GPS-only).
- 24 h on-flash trail persistence.
- Any non-skyview UI (the Mac app owns it).
- Touch / buttons / interactivity.
- OTA updates, captive-portal provisioning.

## 11. Open items to confirm during implementation

1. Final MQTT client library and exact TLS RX buffer size.
2. Exact readout placement/typography within the round bezel (tune on hardware).
3. Whether `availability=offline` should blank the view or show a stale-data
   banner.
