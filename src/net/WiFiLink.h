// Claude Generated: version 1 - WiFi connection manager with automatic reconnect
#pragma once

class WiFiLink {
 public:
    // Connects to the given SSID. Call once from setup().
    void begin(const char* ssid, const char* pass);

    // Reconnects if dropped. Call every loop() iteration.
    void loop();

    bool connected() const;
};
