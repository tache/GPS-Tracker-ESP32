// Claude Generated: version 1 - Unity tests for SatelliteStore live state and trail cadence
#include <unity.h>
#include "store/SatelliteStore.h"

void setUp() {}
void tearDown() {}

static SkyData sky2() {
    SkyData d;
    d.satUsed = 1; d.satVisible = 2;
    d.satellites = {{5, 72, 214, 42, true}, {9, 18, 47, 22, false}};
    return d;
}

void test_updates_live_satellites_and_counts() {
    SatelliteStore s;
    s.updateSky(sky2(), 0);
    TEST_ASSERT_EQUAL_INT(2, (int)s.satellites().size());
    TEST_ASSERT_EQUAL_INT(1, s.satUsed());
    TEST_ASSERT_EQUAL_INT(2, s.satVisible());
}

void test_first_update_writes_trail() {
    SatelliteStore s;
    s.updateSky(sky2(), 0);
    TEST_ASSERT_EQUAL_INT(1, (int)s.trail().samples(5).size());
}

void test_trail_not_written_before_interval() {
    SatelliteStore s;
    s.updateSky(sky2(),  0);
    s.updateSky(sky2(), 30);  // 30s < 60s interval
    TEST_ASSERT_EQUAL_INT(1, (int)s.trail().samples(5).size());
}

void test_trail_written_at_interval() {
    SatelliteStore s;
    s.updateSky(sky2(),  0);
    s.updateSky(sky2(), 60);  // exactly 60s: new point due
    TEST_ASSERT_EQUAL_INT(2, (int)s.trail().samples(5).size());
}

void test_trail_written_after_interval() {
    SatelliteStore s;
    s.updateSky(sky2(),  0);
    s.updateSky(sky2(), 90);  // 90s > 60s: also writes
    TEST_ASSERT_EQUAL_INT(2, (int)s.trail().samples(5).size());
}

void test_fix_and_online_state() {
    SatelliteStore s;
    s.updateFix("3D Fix");
    s.setOnline(true);
    TEST_ASSERT_EQUAL_STRING("3D Fix", s.fix().c_str());
    TEST_ASSERT_TRUE(s.online());
}

void test_default_fix_is_dashes() {
    SatelliteStore s;
    TEST_ASSERT_EQUAL_STRING("--", s.fix().c_str());
}

void test_default_online_is_false() {
    SatelliteStore s;
    TEST_ASSERT_FALSE(s.online());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_updates_live_satellites_and_counts);
    RUN_TEST(test_first_update_writes_trail);
    RUN_TEST(test_trail_not_written_before_interval);
    RUN_TEST(test_trail_written_at_interval);
    RUN_TEST(test_trail_written_after_interval);
    RUN_TEST(test_fix_and_online_state);
    RUN_TEST(test_default_fix_is_dashes);
    RUN_TEST(test_default_online_is_false);
    return UNITY_END();
}
