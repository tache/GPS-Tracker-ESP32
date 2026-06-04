// Claude Generated: version 1 - WiFiLink implementation for ESP32 Arduino framework
#include "net/WiFiLink.h"
#include <WiFi.h>

static const char* s_ssid = nullptr;
static const char* s_pass = nullptr;

void WiFiLink::begin(const char* ssid, const char* pass) {
    s_ssid = ssid;
    s_pass = pass;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
}

void WiFiLink::loop() {
    if (WiFi.status() != WL_CONNECTED && s_ssid) WiFi.reconnect();
}

bool WiFiLink::connected() const {
    return WiFi.status() == WL_CONNECTED;
}
