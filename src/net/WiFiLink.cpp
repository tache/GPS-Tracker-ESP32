// Claude Generated: version 1 - WiFiLink implementation for ESP32 Arduino framework
// Claude Generated: version 2 - Set DHCP hostname before WiFi.begin()
#include "net/WiFiLink.h"
#include <WiFi.h>

static const char* s_ssid = nullptr;
static const char* s_pass = nullptr;

void WiFiLink::begin(const char* ssid, const char* pass) {
    s_ssid = ssid;
    s_pass = pass;
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("gps-skyview");  // advertised via DHCP; visible in router client list
    WiFi.begin(ssid, pass);
}

void WiFiLink::loop() {
    if (WiFi.status() == WL_CONNECTED) return;
    if (!s_ssid) return;
    // Rate-limit reconnect to 5 s — calling it while associating causes stack error spam.
    static uint32_t lastReconnectMs = 0;
    if (millis() - lastReconnectMs < 5000) return;
    lastReconnectMs = millis();
    WiFi.reconnect();
}

bool WiFiLink::connected() const {
    return WiFi.status() == WL_CONNECTED;
}
