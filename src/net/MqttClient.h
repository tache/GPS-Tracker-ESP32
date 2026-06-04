// Claude Generated: version 1 - TLS MQTT client wrapping 256dpi/arduino-mqtt over WiFiClientSecure
#pragma once
#include <functional>
#include <string>

class MqttClient {
 public:
    using Handler = std::function<void(const std::string& topic,
                                       const std::string& payload)>;

    // Sets up TLS and MQTT; subscribes on first successful connect.
    // Call once from setup().
    void begin(const char* host, int port, const char* caCert,
               const char* clientId, const char* user, const char* pass,
               Handler onMessage);

    // Pumps the MQTT loop and reconnects/resubscribes when disconnected.
    // Call every loop() iteration.
    void loop();

    bool connected() const;
};
