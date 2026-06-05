// Claude Generated: version 1 - Unity tests for sky message JSON parsing
// Claude Generated: version 2 - Add float-payload test matching real GPSD output
#include <unity.h>
#include "model/SkyMessage.h"

void setUp() {}
void tearDown() {}

static const char* kSky = R"({
  "sat_used": 2, "sat_visible": 2,
  "satellites": [
    {"prn":5,"el":72,"az":214,"ss":42,"used":true,"seen":312},
    {"prn":29,"el":18,"az":47,"ss":22,"used":false,"seen":88}
  ]})";

void test_parses_counts() {
    SkyData out;
    TEST_ASSERT_TRUE(parseSky(kSky, out));
    TEST_ASSERT_EQUAL_INT(2, out.satUsed);
    TEST_ASSERT_EQUAL_INT(2, out.satVisible);
}

void test_parses_satellite_count() {
    SkyData out;
    parseSky(kSky, out);
    TEST_ASSERT_EQUAL_INT(2, (int)out.satellites.size());
}

void test_parses_first_satellite() {
    SkyData out;
    parseSky(kSky, out);
    TEST_ASSERT_EQUAL_INT(5,   out.satellites[0].prn);
    TEST_ASSERT_EQUAL_INT(72,  out.satellites[0].elevation);
    TEST_ASSERT_EQUAL_INT(214, out.satellites[0].azimuth);
    TEST_ASSERT_EQUAL_INT(42,  out.satellites[0].snr);  // ss -> snr
    TEST_ASSERT_TRUE(out.satellites[0].used);
}

void test_parses_unused_satellite() {
    SkyData out;
    parseSky(kSky, out);
    TEST_ASSERT_FALSE(out.satellites[1].used);
    TEST_ASSERT_EQUAL_INT(29, out.satellites[1].prn);
}

// Real GPSD output sends el/az/ss as JSON floats (e.g. "el":64.0).
// This test guards against the | 0 (int) regression where float variants returned 0.
static const char* kSkyFloat = R"({
  "sat_used": 1, "sat_visible": 1,
  "satellites": [
    {"prn":31,"el":64.0,"az":34.0,"ss":39.0,"used":true,"seen":15364,"gnssid":0,"svid":31}
  ]})";

void test_parses_float_fields() {
    SkyData out;
    TEST_ASSERT_TRUE(parseSky(kSkyFloat, out));
    TEST_ASSERT_EQUAL_INT(64, out.satellites[0].elevation);
    TEST_ASSERT_EQUAL_INT(34, out.satellites[0].azimuth);
    TEST_ASSERT_EQUAL_INT(39, out.satellites[0].snr);
}

void test_rejects_garbage() {
    SkyData out;
    TEST_ASSERT_FALSE(parseSky("not json", out));
}

void test_rejects_empty_string() {
    SkyData out;
    TEST_ASSERT_FALSE(parseSky("", out));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_parses_counts);
    RUN_TEST(test_parses_satellite_count);
    RUN_TEST(test_parses_first_satellite);
    RUN_TEST(test_parses_unused_satellite);
    RUN_TEST(test_parses_float_fields);
    RUN_TEST(test_rejects_garbage);
    RUN_TEST(test_rejects_empty_string);
    return UNITY_END();
}
