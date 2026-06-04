// Claude Generated: version 1 - Mock-MQTT integration test: canned payloads through router to store
#include <unity.h>
#include "app/MessageRouter.h"

void setUp() {}
void tearDown() {}

static const char* kSky =
    R"({"sat_used":1,"sat_visible":2,"satellites":[
        {"prn":5,"el":72,"az":214,"ss":42,"used":true,"seen":1},
        {"prn":9,"el":18,"az":47,"ss":22,"used":false,"seen":1}]})";

void test_sky_message_updates_store() {
    SatelliteStore store;
    MessageRouter router(store);
    router.route("gps_monitor/sky", kSky, 0);
    TEST_ASSERT_EQUAL_INT(2, (int)store.satellites().size());
    TEST_ASSERT_EQUAL_INT(1, store.satUsed());
    TEST_ASSERT_EQUAL_INT(2, store.satVisible());
}

void test_tpv_message_updates_fix() {
    SatelliteStore store;
    MessageRouter router(store);
    router.route("gps_monitor/tpv", R"({"fix":"3D Fix"})", 0);
    TEST_ASSERT_EQUAL_STRING("3D Fix", store.fix().c_str());
}

void test_availability_online() {
    SatelliteStore store;
    MessageRouter router(store);
    router.route("gps_monitor/availability", "online", 0);
    TEST_ASSERT_TRUE(store.online());
}

void test_availability_offline() {
    SatelliteStore store;
    MessageRouter router(store);
    router.route("gps_monitor/availability", "offline", 0);
    TEST_ASSERT_FALSE(store.online());
}

void test_unknown_topic_is_ignored() {
    SatelliteStore store;
    MessageRouter router(store);
    router.route("some/other/topic", "{}", 0);
    TEST_ASSERT_EQUAL_INT(0, (int)store.satellites().size());
}

void test_full_flow_sky_tpv_avail() {
    SatelliteStore store;
    MessageRouter router(store);
    router.route("gps_monitor/sky",          kSky,                    0);
    router.route("gps_monitor/tpv",          R"({"fix":"3D Fix"})",   0);
    router.route("gps_monitor/availability", "online",                0);
    TEST_ASSERT_EQUAL_INT(2, (int)store.satellites().size());
    TEST_ASSERT_EQUAL_STRING("3D Fix", store.fix().c_str());
    TEST_ASSERT_TRUE(store.online());
}

void test_second_sky_at_60s_grows_trail() {
    SatelliteStore store;
    MessageRouter router(store);
    router.route("gps_monitor/sky", kSky,  0);
    router.route("gps_monitor/sky", kSky, 60);
    TEST_ASSERT_EQUAL_INT(2, (int)store.trail().samples(5).size());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_sky_message_updates_store);
    RUN_TEST(test_tpv_message_updates_fix);
    RUN_TEST(test_availability_online);
    RUN_TEST(test_availability_offline);
    RUN_TEST(test_unknown_topic_is_ignored);
    RUN_TEST(test_full_flow_sky_tpv_avail);
    RUN_TEST(test_second_sky_at_60s_grows_trail);
    return UNITY_END();
}
