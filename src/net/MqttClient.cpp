// Claude Generated: version 1 - MqttClient implementation using 256dpi/arduino-mqtt over TLS
#include "net/MqttClient.h"
#include <MQTT.h>
#include <WiFiClientSecure.h>

namespace {

WiFiClientSecure s_net;
// 4 KB buffer — sky payloads with ~15 satellites reach ~2 KB; headroom for edge cases.
MQTTClient s_mqtt(4096);
MqttClient::Handler s_handler;
std::string s_host;
std::string s_clientId;
std::string s_user;
std::string s_pass;
int s_port = 8883;

void onMqttMessage(String& topic, String& payload) {
    if (s_handler) {
        s_handler(std::string(topic.c_str()), std::string(payload.c_str()));
    }
}

void ensureConnected() {
    if (s_mqtt.connected()) return;
    // LWT mirrors the broadcaster's availability convention so HA sees us go offline.
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
    s_host     = host;
    s_port     = port;
    s_clientId = clientId;
    s_user     = user;
    s_pass     = pass;
    s_handler  = std::move(onMessage);
    s_net.setCACert(caCert);
    s_mqtt.begin(s_host.c_str(), s_port, s_net);
    s_mqtt.onMessage(onMqttMessage);
}

void MqttClient::loop() {
    s_mqtt.loop();
    ensureConnected();
}

bool MqttClient::connected() const {
    return s_mqtt.connected();
}
