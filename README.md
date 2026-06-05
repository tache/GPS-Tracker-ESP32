# GPS-Tracker-ESP32

ESP32-C3 firmware that renders a live GPS satellite skyview on a 1.28″ round GC9A01A display. Connects to a GPSD-to-MQTT broadcaster over WiFi and displays satellite positions, signal strength, and fix status in real time.

Inspired by [ESP32-Plane-Radar](https://github.com/MatixYo/ESP32-Plane-Radar) — same hardware (ESP32 + round GC9A01A display), adapted for GPS satellite skyview via MQTT rather than ADS-B aircraft tracking.

## Display

- Horizon-to-zenith polar projection — centre = zenith (90°), rim = horizon (0°)
- North up, compass labels N / E / S / W
- Elevation reference rings at 30° and 60°
- Satellite dots: **filled = used in fix**, hollow = not used
- SNR color coding: 🟢 ≥35 dBHz · 🟠 ≥20 dBHz · 🟡 <20 dBHz · 🔴 unused
- Satellite trails accumulate over 15 minutes (60-second snapshot cadence)
- Live readout: GPS Zulu time · satellite counts · fix status

## Hardware

| Component | Part |
|-----------|------|
| MCU | ESP32-C3 Super Mini |
| Display | 1.28″ round GC9A01A IPS, 240×240, SPI |

### Wiring

| Display pin | ESP32-C3 GPIO |
|-------------|---------------|
| VCC | **3V3 — not 5V** |
| GND | GND |
| RST | GPIO 0 |
| CS | GPIO 1 |
| DC | GPIO 10 |
| SDA (MOSI) | GPIO 3 |
| SCL (SCLK) | GPIO 4 |

Backlight (BLK) is tied to 3V3 — always on, no software control.

## Dependencies

Managed via PlatformIO:

| Library | Purpose |
|---------|---------|
| [LovyanGFX](https://github.com/lovyan03/LovyanGFX) | GC9A01A display driver |
| [256dpi/MQTT](https://github.com/256dpi/arduino-mqtt) | TLS MQTT client over `WiFiClientSecure` |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | JSON parsing for MQTT payloads |

## Setup

1. **Copy and fill in credentials:**

```bash
cp src/secrets.h.example src/secrets.h
```

Edit `src/secrets.h` with your WiFi SSID/password, MQTT broker host/port, credentials, and CA certificate.

2. **Build and flash:**

```bash
pio run -e esp32-c3 -t upload --upload-port /dev/cu.usbmodemXXXX
```

3. **Monitor serial output:**

```bash
pio device monitor --port /dev/cu.usbmodemXXXX -b 115200
```

## Development

### Run native logic tests (no hardware required)

```bash
pio test -e native
```

Tests cover: polar projection math · SNR color classification · sky/tpv JSON parsing · trail ring buffer · satellite store · end-to-end mock-MQTT integration.

### Build firmware only

```bash
pio run -e esp32-c3
```

### Clean build artifacts

```bash
pio run -e esp32-c3 -t clean
```

## Architecture

The pure-logic core (model, store, app layers) is Arduino-free and fully unit-tested under the PlatformIO `native` environment. The Arduino shell (WiFi, MQTT, display) wraps it.

```
src/
  config/       GPIO pin constants
  model/        Satellite structs, sky/tpv JSON parsers
  store/        SatelliteStore, TrailBuffer
  app/          MessageRouter (MQTT topic → parser → store)
  net/          WiFiLink, MqttClient (TLS)
  view/         Projection math, SNR color, SkyView renderer
  main.cpp      Display init, wiring, 1 Hz render loop
test/
  test_projection/
  test_snrcolor/
  test_skymessage/
  test_tpvmessage/
  test_trailbuffer/
  test_store/
  test_integration/   mock-MQTT end-to-end data path
```

## MQTT Topics

| Topic | Data used |
|-------|-----------|
| `gps_monitor/sky` | Satellite positions (el/az/SNR/used) + counts |
| `gps_monitor/tpv` | Fix status + GPS Zulu time |
| `gps_monitor/availability` | Broker online/offline status |
