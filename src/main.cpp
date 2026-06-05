// Claude Generated: version 1 - Main entry point: display init, module wiring, setup/loop
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "config/Pins.h"
#include "secrets.h"
#include "app/MessageRouter.h"
#include "net/MqttClient.h"
#include "net/WiFiLink.h"
#include "store/SatelliteStore.h"
#include "view/SkyView.h"

// LovyanGFX driver config for GC9A01A on ESP32-C3 Super Mini.
// SPI write-only (no MISO): backlight tied to 3V3, no BLK pin.
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_GC9A01  panel_;
    lgfx::Bus_SPI       bus_;
 public:
    LGFX() {
        {
            auto c = bus_.config();
            c.spi_host    = SPI2_HOST;
            c.spi_mode    = 0;
            c.freq_write  = 40000000;  // 40 MHz
            c.pin_sclk    = pins::kDisplaySclk;
            c.pin_mosi    = pins::kDisplayMosi;
            c.pin_miso    = -1;
            c.pin_dc      = pins::kDisplayDc;
            bus_.config(c);
            panel_.setBus(&bus_);
        }
        {
            auto c = panel_.config();
            c.pin_cs        = pins::kDisplayCs;
            c.pin_rst       = pins::kDisplayRst;
            c.panel_width   = 240;
            c.panel_height  = 240;
            c.offset_x      = 0;
            c.offset_y      = 0;
            c.invert        = true;   // GC9A01A default is inverted; this restores correct polarity
            panel_.config(c);
        }
        setPanel(&panel_);
    }
};

static LGFX            gfx;
static SatelliteStore  store;
static MessageRouter   router(store);
static WiFiLink        wifi;
static MqttClient      mqtt;
static SkyView         view;
static uint32_t        lastRenderMs   = 0;
static uint32_t        lastStatusMs   = 0;

void setup() {
    Serial.begin(115200);
    // ESP32-C3 native USB needs a moment before Serial is ready.
    delay(1500);
    Serial.println("Booting...");
    gfx.init();
    gfx.setRotation(0);
    gfx.fillScreen(0x0000);
    view.begin(gfx);
    Serial.printf("Sprite init done. Free heap: %u bytes\n", ESP.getFreeHeap());

    wifi.begin(secrets::kWifiSsid, secrets::kWifiPassword);

    // Wait for WiFi before starting MQTT — DNS fails if we attempt while still associating.
    // Don't call wifi.loop() here; reconnect() during association causes stack errors.
    Serial.print("Connecting WiFi");
    while (!wifi.connected()) {
        Serial.print(".");
        delay(500);
    }
    Serial.println(" OK");

    mqtt.begin(
        secrets::kMqttHost,
        secrets::kMqttPort,
        secrets::kMqttCaCert,
        secrets::kMqttClientId,
        secrets::kMqttUser,
        secrets::kMqttPassword,
        [](const std::string& topic, const std::string& payload) {
            router.route(topic, payload, millis() / 1000);
        });
}

void loop() {
    wifi.loop();
    mqtt.loop();
    // Log WiFi/MQTT/satellite status every 5 s for diagnostics.
    if (millis() - lastStatusMs >= 5000) {
        Serial.printf("WiFi:%d  MQTT:%d  sats:%d  fix:%s  heap:%u\n",
            wifi.connected(), mqtt.connected(),
            (int)store.satellites().size(), store.fix().c_str(),
            ESP.getFreeHeap());
        for (const auto& s : store.satellites()) {
            Serial.printf("  PRN%2d  el:%3d  az:%3d  snr:%2d  used:%d\n",
                s.prn, s.elevation, s.azimuth, s.snr, s.used);
        }
        lastStatusMs = millis();
    }
    // Redraw at ~1 Hz — matches the ~1 Hz GPSD sky message rate.
    if (millis() - lastRenderMs >= 1000) {
        view.render(gfx, store);
        lastRenderMs = millis();
    }
}
